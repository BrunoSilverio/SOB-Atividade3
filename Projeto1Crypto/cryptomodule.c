#include "cryptomodule.h"

/*
 * Tabela com as funções de manipulação de arquivos
 * Pode ser chamada de "jump table"
 * As funções declaradas aqui sobrescrevem as operações padrão.
 */
static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release};

static struct miscdevice mdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &fops,
};

/* ---------------------------------------------------------------- */

static int Major;
static struct class *cls;
static short size_of_message;
static char msg[BUF_LEN];
static char operacao;
static unsigned char dados[TAMMAX];
unsigned char dadosHex[TAMMAX / 2];
static char string_hash[SHA256_LENGTH * 2 + 1];
// static char resultado[BUF_LEN];
static char *readMSG;
static char *deciphertext;
static char *key;

/*Cria um parametro para o modulo, com a permicao 0
o parametro so pode ser atribuido na hora do insmod*/

/*Para que seja possivel ler uma string com espaco eh preiso
colocar a string com aspas duplas dentro de aspas simples
EX: '"Hello Word"', nesse caso o shell vai pegar as aspas simples
e mandar a string com aspas duplas para o modulo*/

module_param(key, charp, 0);
MODULE_PARM_DESC(key, "A character string");

static void shiftConcat(size_t const size, char *stringHex, char *stringNorm)
{
	unsigned char byte;
	int i, j;

	// Transformar em valores hexa normal
	for(i = 0; i < size; i++){
		pr_info("%i ", stringHex[i]);
	}
	for (i = 0; i < size; i++)
	{
		if ((int)stringHex[i] >= 48 && (int)stringHex[i] <= 57)
			stringHex[i] -= (char)48;
		else if ((int)stringHex[i] >= 97 && (int)stringHex[i] <= 102)
			stringHex[i] -= (char)87;
		else if ((int)stringHex[i] >= 65 && (int)stringHex[i] <= 70)
			stringHex[i] -= (char)55;
		else
			pr_info("ERROR: String em Hexa contem valor desconhecido -> %i", i);
	}

	// Concatenação em um unico byte
	j = 0;
	for (i = 0; i < size / 2; i++)
	{
		stringNorm[i] = (stringHex[j] << 4) + stringHex[j + 1];
		j += 2;
	}
	// Para Printar -- Revomer depois
	for (i = 0; i < size / 2; i++)
	{
		for (j = 7; j >= 0; j--)
		{
			byte = (stringNorm[i] >> j) & 1;
			pr_info("%u", byte);
		}
		pr_info(" ");
	}
	pr_info("\n");
}
/* ---------------------------------------------------------------- */

struct tcrypt_result
{
	struct completion completion;
	int err;
};

struct skcipher_def
{
	struct scatterlist sg;
	struct crypto_skcipher *tfm;
	struct skcipher_request *req;
	struct tcrypt_result result; //verifica se a transformacao ocorreu com sucesso.
	char *scratchpad;// ponteiro auxiliar
	char *ciphertext;// string para a ser cifrada
	char *ivdata;
};

static struct skcipher_def sk;
/*-------------------------------Encrypt---------------------------------------*/
static void test_skcipher_finish(struct skcipher_def *sk)
{
	if (sk->tfm)
		crypto_free_skcipher(sk->tfm);
	if (sk->req)
		skcipher_request_free(sk->req);
	if (sk->ivdata)
		kfree(sk->ivdata);
	if (sk->scratchpad)
		kfree(sk->scratchpad);
	if (sk->ciphertext)
		kfree(sk->ciphertext);
}
static int test_skcipher_result(struct skcipher_def *sk, int rc)
{
	switch (rc)
	{
	case 0:
		break;
	case -EINPROGRESS:

	case -EBUSY:
		rc = wait_for_completion_interruptible(
			&sk->result.completion);
		if (!rc && !sk->result.err)
		{
			reinit_completion(&sk->result.completion);
			break;
		}
	default:
		pr_info("skcipher encrypt returned with %d result %d\n",
				rc, sk->result.err);
		break;
	}

	init_completion(&sk->result.completion);
	return rc;
}
static void test_skcipher_callback(struct crypto_async_request *req, int error)
{
	struct tcrypt_result *result = req->data;
	int ret;
	if (error == -EINPROGRESS)
		return;
	result->err = error;
	complete(&result->completion);
	pr_info("Encryption finished successfully result = %s\n", result->completion);
}
static int test_skcipher_encrypt(char *plaintext, char *password,
								 struct skcipher_def *sk)
{
	int ret = -EFAULT;
	unsigned char key[SYMMETRIC_KEY_LENGTH];

	if (!sk->tfm)
	{
		sk->tfm = crypto_alloc_skcipher("ecb(aes)", 0, 0);
		if (IS_ERR(sk->tfm))
		{
			pr_info("could not allocate skcipher handle\n");
			return PTR_ERR(sk->tfm);
		}
	}
	if (!sk->req)
	{
		sk->req = skcipher_request_alloc(sk->tfm, GFP_KERNEL);
		if (!sk->req)
		{
			pr_info("could not allocate skcipher request\n");
			ret = -ENOMEM;
			goto out;
		}
	}

	skcipher_request_set_callback(sk->req, CRYPTO_TFM_REQ_MAY_BACKLOG,
								  test_skcipher_callback,
								  &sk->result);
	/* clear the key */

	memset((void *)key, '\0', SYMMETRIC_KEY_LENGTH);

	/* Use the world's favourite password */
	sprintf((char *)key, "%s", password);

	/* AES 256 with given symmetric key */
	if (crypto_skcipher_setkey(sk->tfm, key, SYMMETRIC_KEY_LENGTH))
	{
		pr_info("key could not be set\n");
		ret = -EAGAIN;
		goto out;
	}
	pr_info("Symmetric key: %s\n", key);
	pr_info("Plaintext: %s\n", plaintext);
	if (!sk->ivdata)
	{
		/* see https://en.wikipedia.org/wiki/Initialization_vector */
		sk->ivdata = kmalloc(CIPHER_BLOCK_SIZE, GFP_KERNEL);
		if (!sk->ivdata)
		{
			pr_info("could not allocate ivdata\n");
			goto out;
		}
		get_random_bytes(sk->ivdata, CIPHER_BLOCK_SIZE);
	}
	if (!sk->scratchpad)
	{
		/* The text to be encrypted */
		sk->scratchpad = kmalloc(CIPHER_BLOCK_SIZE, GFP_KERNEL);
		if (!sk->scratchpad)
		{
			pr_info("could not allocate scratchpad\n");
			goto out;
		}
	}
	pr_info("Plaintext: %s\n", plaintext);
	sprintf((char *)sk->scratchpad, "%s", plaintext);
	sg_init_one(&sk->sg, sk->scratchpad, CIPHER_BLOCK_SIZE);
	skcipher_request_set_crypt(sk->req, &sk->sg, &sk->sg,
							   CIPHER_BLOCK_SIZE, sk->ivdata);
	init_completion(&sk->result.completion);
	/* encrypt data */
	ret = crypto_skcipher_encrypt(sk->req);

	//ret = crypto_skcipher_decrypt(sk->req); //decripita o ciphertext dentro da scatterlist.

	ret = test_skcipher_result(sk, ret);

	if (ret)
		goto out;
	pr_info("Encryption request successful\n");

out:
	return ret;
}


static int test_skcipher_dencrypt(char *plaintext, char *password,
								 struct skcipher_def *sk)
{
	int ret = -EFAULT;
	unsigned char key[SYMMETRIC_KEY_LENGTH];

	if (!sk->tfm)
	{
		sk->tfm = crypto_alloc_skcipher("ecb(aes)", 0, 0);
		if (IS_ERR(sk->tfm))
		{
			pr_info("could not allocate skcipher handle\n");
			return PTR_ERR(sk->tfm);
		}
	}
	if (!sk->req)
	{
		sk->req = skcipher_request_alloc(sk->tfm, GFP_KERNEL);
		if (!sk->req)
		{
			pr_info("could not allocate skcipher request\n");
			ret = -ENOMEM;
			goto out;
		}
	}

	skcipher_request_set_callback(sk->req, CRYPTO_TFM_REQ_MAY_BACKLOG,
								  test_skcipher_callback,
								  &sk->result);
	/* clear the key */

	memset((void *)key, '\0', SYMMETRIC_KEY_LENGTH);

	/* Use the world's favourite password */
	sprintf((char *)key, "%s", password);

	/* AES 256 with given symmetric key */
	if (crypto_skcipher_setkey(sk->tfm, key, SYMMETRIC_KEY_LENGTH))
	{
		pr_info("key could not be set\n");
		ret = -EAGAIN;
		goto out;
	}
	pr_info("Symmetric key: %s\n", key);
	pr_info("Plaintext: %s\n", plaintext);
	if (!sk->ivdata)
	{
		/* see https://en.wikipedia.org/wiki/Initialization_vector */
		sk->ivdata = kmalloc(CIPHER_BLOCK_SIZE, GFP_KERNEL);
		if (!sk->ivdata)
		{
			pr_info("could not allocate ivdata\n");
			goto out;
		}
		get_random_bytes(sk->ivdata, CIPHER_BLOCK_SIZE);
	}
	if (!sk->scratchpad)
	{
		/* The text to be encrypted */
		sk->scratchpad = kmalloc(CIPHER_BLOCK_SIZE, GFP_KERNEL);
		if (!sk->scratchpad)
		{
			pr_info("could not allocate scratchpad\n");
			goto out;
		}
	}
	pr_info("Plaintext: %s\n", plaintext);
	sprintf((char *)sk->scratchpad, "%s", plaintext);
	sg_init_one(&sk->sg, sk->scratchpad, CIPHER_BLOCK_SIZE);
	skcipher_request_set_crypt(sk->req, &sk->sg, &sk->sg,
							   CIPHER_BLOCK_SIZE, sk->ivdata);
	init_completion(&sk->result.completion);
	/* encrypt data */

	ret = crypto_skcipher_decrypt(sk->req); //decripita o ciphertext dentro da scatterlist.

	ret = test_skcipher_result(sk, ret);

	if (ret)
		goto out;
	pr_info("Encryption request successful\n");

out:
	return ret;
}

int cryptoapi_init(char *msgUser)
{

	sk.tfm = NULL;
	sk.req = NULL;
	sk.scratchpad = NULL;
	sk.ciphertext = NULL;
	sk.ivdata = NULL;

	char *ciphertext;
	static char resultado[BUF_LEN];
	int i;
	test_skcipher_encrypt(msgUser, key, &sk);

	//char *strAux;
	//faz o calculo do endereco virtual utilizando o end de pagina e offset
	ciphertext = sg_virt(&sk.sg);

	/*printa o conteudo do endereço ao utilizar a funcao 
	* decrypt retorna o chipertext decripitado.*/

	for (i = 0; i < strlen(ciphertext); i++)
	{

		sprintf(&resultado[i * 2], "%02hhx", ciphertext[i]);
		// strcat(resultado,strAux)
	}
	resultado[i * 2] = '\0';

	pr_info("Resultado Cifrado: %s", resultado);

	readMSG = resultado;

	return 0;
}

int decryptoapi_init(char *msgUser)
{

	sk.tfm = NULL;
	sk.req = NULL;
	sk.scratchpad = NULL;
	sk.ciphertext = NULL;
	sk.ivdata = NULL;

	char resultadoCerto[BUF_LEN];
	int i;

	shiftConcat(strlen(msgUser),msgUser,resultadoCerto);

	test_skcipher_dencrypt(resultadoCerto, key, &sk);

	deciphertext = sg_virt(&sk.sg);

	pr_info("Resultado decifrado: %s", deciphertext);

	readMSG = deciphertext;

	return 0;
}

void cryptoapi_exit(void)
{
	test_skcipher_finish(&sk);
}
/*-------------------------------Decrypt---------------------------------------*/

/*-------------------------------HASH------------------------------------------*/
static void show_hash_result(char *plaintext, char *hash_sha256)
{
	int i;

	pr_info("sha256 test for string: '%s'\n", plaintext);

	for (i = 0; i < SHA256_LENGTH; i++)
		sprintf(&string_hash[i * 2], "%02x", (unsigned char)hash_sha256[i]);

	string_hash[i * 2] = 0;

	pr_info("%s\n", string_hash);
	readMSG = string_hash;
}
int cryptosha256_init(char *plaintext)
{

	char hash_sha256[SHA256_LENGTH];

	struct crypto_shash *sha256;

	struct shash_desc *shash; //Make and install the module: And you should see that the hash was calculated for the test string. Finally, remove the test module: Symmetric key encryption Here is an example of symmetrically encrypting a string using the AES algorithm and a password. sha256 = crypto_alloc_shash( "sha256" , 0, 0);

	sha256 = crypto_alloc_shash("sha256", 0, 0);

	if (IS_ERR(sha256))
		return -1;

	shash = kmalloc(sizeof(struct shash_desc) + crypto_shash_descsize(sha256), GFP_KERNEL);

	if (!shash)
		return -ENOMEM;

	shash->tfm = sha256;
	shash->flags = 0;

	if (crypto_shash_init(shash))
		return -1;
	if (crypto_shash_update(shash, plaintext, strlen(plaintext)))
		return -1;
	if (crypto_shash_final(shash, hash_sha256))
		return -1;

	kfree(shash);

	crypto_free_shash(sha256);

	show_hash_result(plaintext, hash_sha256);

	return 0;
}

/*-------------------------------Funcoes de W/R------------------------------------------*/
static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{

	int bytes_read = 0;

		pr_info("readMSG: %s", readMSG);

		if (*readMSG == 0)
			return 0;

		/*
     	* Actually put the data into the buffer
     	*/

		while (length && *readMSG)
		{

			/*
			* The buffer is in the user data segment, not the kernel
			* segment so "*" assignment won't work.  We have to use
			* put_user which copies data from the kernel data segment to
			* the user data segment.
			*/

			put_user(*(readMSG++), buffer++);

			length--;
			bytes_read++;
		}
	
	pr_info("arquivo lido");
	return bytes_read;
}
static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t *off)
{
	int i, j;
	char tipo, msgPassada[BUF_LEN];
	sprintf(msg, "%s", buff, len);
	size_of_message = strlen(msg);
	operacao = msg[0];
	tipo = msg[size_of_message - 1];
	pr_info("msg  = %s", msg);
	pr_info("Operacao = %c", msg[0]);
	pr_info("size_of_message = %i", size_of_message);
	pr_info("Tipo dos dados = %c", tipo);

	for(i = 0;i < size_of_message - 3;i++)
	{
		msgPassada[i] = msg[i + 2];
	}
	msgPassada[i] = '\0';
	pr_info("Msg recebida: %s", msgPassada);

	if (operacao == 'c' || operacao == 'C')
	{
		/*Cifrar dados*/
		cryptoapi_init(msgPassada);
	}
	else if (operacao == 'd' || operacao == 'D')
	{
		/*Decifrar dados*/
		decryptoapi_init(msgPassada);
	}
	else if (operacao == 'h' || operacao == 'H')
	{
		/*Resumo criptografico key*/
		char *plaintext = msgPassada;

		cryptosha256_init(plaintext);
	}
	else
	{
		printk(KERN_INFO "Operacao invalida");
	}

	printk(KERN_INFO "Operacao realizada");
	return SUCCESS;
}

static int device_open(struct inode *inode, struct file *file)
{

	printk(KERN_INFO "arquivo aberto");
	return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "arquivo liberado");
	return SUCCESS;
}

/* ---------------------------------------------------------------- */

/* Função que será chamada quando o modulo é instalado */
static int __init cryptomodule_init(void)
{
	/*Faz uma requesicao para saber se o numero 0 pode ser */
	/*usado como  Major number para o modulo*/
	Major = register_chrdev(0, DEVICE_NAME, &fops);
	if (Major < 0)
	{
		pr_alert("Registering char device failed with %d\n", Major);
		return Major;
	}

	pr_info("I was assigned major number %d.\n", Major);

	/*Cria um Ponteiro da struct de classe que sera usado para a criacao do device */
	cls = class_create(THIS_MODULE, DEVICE_NAME);
	/*Cria o Device na /dev*/
	device_create(cls, NULL, MKDEV(Major, 0), NULL, DEVICE_NAME);

	pr_info("%s\n", key);									 /*Print para ver se o programa recebe a key*/
	pr_info("Dispositivo criado em /dev/%s\n", DEVICE_NAME); /* OK */

	return SUCCESS;
}

/* Função que será chamada quando o modulo é removido  */
static void __exit cryptomodule_exit(void)
{
	/*Retira o device e a classe e
	por fim retira o registro do major number*/
	cryptoapi_exit();
	device_destroy(cls, MKDEV(Major, 0));
	class_destroy(cls);
	unregister_chrdev(Major, DEVICE_NAME);
	printk(KERN_INFO "Dispositivo %s removido\n", DEVICE_NAME);
}

/* Registra quais funções devem ser chamadas para cada "evento"  */
module_init(cryptomodule_init);
module_exit(cryptomodule_exit);

/* Informações sobre o módulo  */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("");
MODULE_DESCRIPTION("");