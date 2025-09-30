#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/delay.h>


#define LOCAL_SOFTIRQ 29

static void cb_softirq_action(struct softirq_action *h)
{
	pr_info("cb_softirq_action() executing in softirq context (CPU %d)\n", smp_processor_id());
}

static int __init ex_softirq_init(void) {
    pr_info("Example softirq. Init, register softirq: %d\n", LOCAL_SOFTIRQ);
    open_softirq(LOCAL_SOFTIRQ, cb_softirq_action); 
    msleep(50);

    pr_info("raising softirq %d\n", LOCAL_SOFTIRQ);
	raise_softirq(LOCAL_SOFTIRQ);

    return 0; 
}

static void __exit ex_softirq_exit(void) {
    pr_info("Example ex_softirq: BB n exit.\n");

}

module_init(ex_softirq_init);
module_exit(ex_softirq_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stv");
MODULE_DESCRIPTION("HW 05 TIMER_SOFTIRQ");
