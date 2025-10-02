#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>

static void cb_tasket_func(unsigned long data) {
    pr_info("Tasklet function executed with data: %lu and on cpu: %d \n", data, smp_processor_id());
}

static struct tasklet_struct local_tasket;

static int __init ex_tasklets_init(void) {
    pr_info("Module init\n");

    tasklet_init(&local_tasket, cb_tasket_func, 42UL);  
    pr_info("Tasklet initialized\n");

    tasklet_schedule(&local_tasket);
    pr_info("Tasklet scheduled (normal priority)\n");

    tasklet_hi_schedule(&local_tasket);
    pr_info("Tasklet scheduled (high priority)\n");


    pr_info("Simulating IRQ bottom-half usage\n");
    tasklet_schedule(&local_tasket);

    return 0;
}

static void __exit ex_tasklets_exit(void) {
    tasklet_kill(&local_tasket);
    pr_info("Tasklet killed on exit\n");
    pr_info("Module exiting\n");
}

module_init(ex_tasklets_init);
module_exit(ex_tasklets_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stv");
MODULE_DESCRIPTION("HW-05 tasklets");
