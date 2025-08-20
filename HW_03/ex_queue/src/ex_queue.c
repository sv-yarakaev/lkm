#include <linux/module.h>





















static int init_queue(void) { return 0;}
static void __exit exit_queue(void) {}



MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("HW-03 example queue");
module_init(init_queue);
module_exit(exit_queue);