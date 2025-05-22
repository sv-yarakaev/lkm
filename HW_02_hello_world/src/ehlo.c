#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>


static int __init ehlo_init(void)
{
    pr_info("EHLO \n\n");


    return 0;
}

static void __exit ehlo_exit(void)
{
    pr_info("Goodbye, World from the kernel!\n");
}

module_init(ehlo_init);
module_exit(ehlo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stv");
MODULE_DESCRIPTION("A simple hello(ehlo)  module for the Linux kernel");
