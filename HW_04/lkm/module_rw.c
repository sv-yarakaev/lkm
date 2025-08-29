#include <linux/kernel.h>
#include <linux/module.h>

static __init int rw_init(void) {
  printk(KERN_INFO "EHLO\n");
  return 0;
}
static __exit void rw_exit(void) { printk(KERN_INFO "BB\n"); }

module_init(rw_init);
module_exit(rw_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Homework 04 ");
