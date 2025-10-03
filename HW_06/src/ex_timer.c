#include <linux/module.h>
#include <linux/kernel.h>


static __init int ex_timer_init(void) {
    return 0;
}

static __exit void ex_timer_exit(void) {}


module_init(ex_timer_init);
module_exit(ex_timer_exit);


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("HW 06 kernel timer examle");

