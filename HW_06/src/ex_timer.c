#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/param.h> //for HZ
                     
static struct timer_list my_timer;
static unsigned int counter = 0;
static unsigned int timeout_ms; // automatic define interval


static void my_timer_callback(struct timer_list *timer) {
    counter++;
    pr_info("Tick %u, HZ = %d, timeout = %u, jiffies = %lu\n", counter, HZ, timeout_ms, jiffies);

    mod_timer(timer, jiffies + msecs_to_jiffies(timeout_ms));
}

static __init int ex_timer_init(void) {
    timeout_ms = 1000 / HZ; 
    pr_info("Module loaded. HZ = %d (tick is every %u ms)\n", HZ, timeout_ms);

    timer_setup(&my_timer, my_timer_callback, 0);

    mod_timer(&my_timer, my_timer_callback, 0);

    return 0;
}

static __exit void ex_timer_exit(void) {
    del_timer_sync(&my_timer);
    pr_info("Module is unloaded\n");
}


module_init(ex_timer_init);
module_exit(ex_timer_exit);


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("HW 06 kernel timer examle");

