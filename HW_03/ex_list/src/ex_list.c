#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>


static int __init ehlo_init(void) {
  pr_info("Init. ex_list\n");
  return 0;
}

static void __exit ehlo_exit(void) {
  pr_info("Exit.\n");
}

module_init(ehlo_init);
module_exit(ehlo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stv");
MODULE_DESCRIPTION("HW_3 ex_list");
