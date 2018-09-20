#include <linux/module.h> //Needed by all modules
#include <linux/kernel.h> //Needed for KERN_INFO
#include <linux/init.h>   // Needed for the macros

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
