#include <linux/module.h>
#include <linux/kernel.h>





static int __init ex_softirq_init(void) {
	return 0;
}

static void __exit ex_softirq_exit(void) { 
}



module_init(ex_softirq_init);
module_exit(ex_softirq_exit);





MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stv");
MODULE_DESCRIPTION("HW 05 TIMER_SOFTIRQ");
