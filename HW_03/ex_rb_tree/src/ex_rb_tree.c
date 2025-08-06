
#include <linux/module.h>
#include <linux/kernel.h>

#include "linux/printk.h"
#include "random_values.h"



static int ex_rb_init(void) {
    char *value = create_random_id(24);
    pr_info("Start: %s\n", value);

    return 0;
}

static void ex_rb_exit(void) {

}


module_init(ex_rb_init);
module_exit(ex_rb_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stv");
MODULE_DESCRIPTION("HW_3 example rb tree");
