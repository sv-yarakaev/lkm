#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/proc_fs.h>


#define LOCAL_SOFTIRQ 29

static void cb_softirq_action(struct softirq_action *h)
{
	pr_info("cb_softirq_action() executing in softirq context (CPU %d)\n", smp_processor_id());
}

static int __init ex_softirq_init(void) {
    
    pr_info("Load module.\n");

    init_irq_work(&my_irq_work, my_irq_work_handler);
    pr_info("Init irq_work.\n");

    // create proc for raising
    proc_entry = proc_create("ex_softirq", 0666, NULL, &proc_fops);
    if (!proc_entry) {
        pr_err("Cannot create /proc/ex_softirq\n");
        return -ENOMEM;
    }

    pr_info("Run: echo raise | sudo tee /proc/ex_softirq\n");
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
