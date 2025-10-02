#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h> 
#include <linux/jiffies.h>


static void cb_tasklet_local(unsigned long data);
DECLARE_TASKLET(my_tasklet, cb_tasklet_local, 0);

static void cb_tasklet_local(unsigned long data)
{
	pr_info("cb_tasklet_local running in softirq context on CPU %d, data=%lu\n",
	        smp_processor_id(), data);
}

static int __init ex_tasklets_init(void)
{
	pr_info("Init - scheduling tasklet\n");
	tasklet_schedule(&my_tasklet);

	pr_info("Scheduling another tasklet run in 1 second\n");
	mdelay(10);
	tasklet_schedule(&my_tasklet);

	return 0;
}

static void __exit ex_tasklets_exit(void)
{
	pr_info("exit - killing tasklet\n");
	tasklet_kill(&my_tasklet);
	pr_info("tasklet killed\n");
}

module_init(ex_tasklets_init);
module_exit(ex_tasklets_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stv");
MODULE_DESCRIPTION("HW-05 tasklets");
