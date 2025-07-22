#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/gfp_types.h>
#include <linux/printk.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/random.h>
#include <linux/types.h>



static int __init exbin_search_init(void) {
  pr_info("Init. Example kernel list\n");
    
  return 0;
}

static void __exit exbin_search_exit(void) {
  
  pr_info("Exit.\n");
}

module_init(exbin_search_init);
module_exit(exbin_search_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stv");
MODULE_DESCRIPTION("HW_3 ex_bin_search");
