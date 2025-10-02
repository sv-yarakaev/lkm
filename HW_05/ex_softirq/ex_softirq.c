#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>

static int __init ex_softirq_init(void) {

  pr_info("Load module.\n");

  return 0;
}

static void __exit ex_softirq_exit(void) { pr_info("BB n exit.\n"); }

module_init(ex_softirq_init);
module_exit(ex_softirq_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stv");
MODULE_DESCRIPTION("HW 05 Empty SOFTIRQ");
