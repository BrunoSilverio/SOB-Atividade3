#include "device.h" 

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
        .name = DEVICE,
        .fops = &fops,
};

/* ---------------------------------------------------------------- */

static ssize_t device_read(struct file *filp, char *buffer, size_t length,loff_t * offset)
{
	printk(KERN_INFO "arquivo lido");
	return 0;
}

static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	printk(KERN_INFO "arquivo escrito");
	return 0;
}

static int device_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "arquivo aberto");
	return 0; 
}

static int device_release(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "arquivo liberado");
	return 0;
}

/* ---------------------------------------------------------------- */

/* Função que será chamada quando o modulo é instalado */
static int __init hello_module(void)
{
	printk(KERN_INFO "Dispositivo registrado"); /* OK */
}

/* Função que será chamada quando o modulo é removido  */
static void __exit goodbye_module(void)
{
	printk(KERN_INFO "Dispositivo removido\n");

}

/* Registra quais funções devem ser chamadas para cada "evento"  */
module_init(hello_module);
module_exit(goodbye_module);

/* Informações sobre o módulo  */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("");
MODULE_DESCRIPTION("");
