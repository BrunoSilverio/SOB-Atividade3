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
	.release = device_release
};

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
static char operacao
static char dados[128];

static char *key;

/*Cria um parametro para o modulo, com a permicao 0
o parametro so pode ser atribuido na hora do insmod*/

/*Para que seja possivel ler uma string com espaco eh preiso
colocar a string com aspas duplas dentro de aspas simples
EX: '"Hello Word"', nesse caso o shell vai pegar as aspas simples
e mandar a string com aspas duplas para o modulo*/
module_param(key, charp, 0);
MODULE_PARM_DESC(key,"A character string");

/* ---------------------------------------------------------------- */

static ssize_t device_read(struct file *filp, char *buffer, size_t length,loff_t * offset)
{
	if(operacao == 'c'){
		/*Retorna a o dado crifrado*/
	}
	else if(operacao == 'd'){
		/*Retorna a o dado dedos hexadecimal decifrada*/
	}
	else if(operacao == 'h'){
		/*Retorna a o resumo criptografico*/
	}
	else{
			printk(KERN_INFO "Operacao invalida");
	}
	printk(KERN_INFO "arquivo lido");
	return SUCCESS;
}

static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	int i;

	sprintf(msg, "%s(%zu letters)", buff, len);
	size_of_message = strlen(msg);
	operacao = msg[0];
	for(i=0;i<size_of_message - 2;i++){
		dados[i] = msg[i+2];
	}

	if(operacao == 'c' || operacao == 'C'){
		/*Cifrar dados*/
	}
	else if(operacao == 'd' || operacao == 'D'){
		/*Decifrar dados*/
	}
	else if(operacao == 'h'|| operacao == 'H'){
		/*Resumo criptografico key*/
	}
	else{
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
	if (Major < 0) {
			pr_alert("Registering char device failed with %d\n", Major);
			return Major;
	}

	pr_info("I was assigned major number %d.\n", Major);

	/*Cria um Ponteiro da struct de classe que sera usado para a criacao do device */
	cls = class_create(THIS_MODULE, DEVICE_NAME);
	/*Cria o Device na /dev*/
	device_create(cls, NULL, MKDEV(Major, 0), NULL, DEVICE_NAME);

	pr_info("%s\n", key); /*Print para ver se o programa recebe a key*/
	pr_info("Dispositivo criado em /dev/%s\n", DEVICE_NAME); /* OK */

	  return SUCCESS;
}

/* Função que será chamada quando o modulo é removido  */
static void __exit cryptomodule_exit(void)
{
	/*Retira o device e a classe e
	por fim retira o registro do major number*/
	device_destroy(cls, MKDEV(Major, 0));
	class_destroy(cls);
	unregister_chrdev(Major, DEVICE_NAME);
	printk(KERN_INFO "Dispositivo %s removido\n",DEVICE_NAME);

}

/* Registra quais funções devem ser chamadas para cada "evento"  */
module_init(cryptomodule_init);
module_exit(cryptomodule_exit);

/* Informações sobre o módulo  */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("");
MODULE_DESCRIPTION("");
