/*
 *   Definições das operações de manipulação de arquivos que serão
 * usadas para acessar o nó criado em /dev
 *
 *   Definitions for the file hander operations whose will be used
 * to access the node created inside /dev
 *
 */
/*Ver o que cada biblioteca faz ou representa e comentar*/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/crypto.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/poll.h>

#define DEVICE_NAME "cryptomodule"
#define DISK_SIZE 4096
#define BUF_LEN 130
#define SUCCESS 0
#define TAMMAX 100

static ssize_t device_read(struct file *, char *, size_t, loff_t *);

static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

static int device_open(struct inode *, struct file *);

static int device_release(struct inode *, struct file *);
