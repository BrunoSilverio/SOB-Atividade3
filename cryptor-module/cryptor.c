#include <linux/module.h>  //Needed by all modules
#include <linux/kernel.h>  //Needed for KERN_INFO
#include <linux/init.h>    // Needed for the macros
#include <linux/device.h>  // Header to support the kernel Driver Model
#include <linux/fs.h>      // Header for the Linux file system support
#include <linux/uaccess.h> // Required for the copy to user function

static int __init cryptor_init(void)
{
  // Cryptor initial code
  return 0;
}

static void __exit cryptor_exit(void)
{
  // Cryptor exit code
}

module_init(cryptor_init);
module_exit(cryptor_exit);
