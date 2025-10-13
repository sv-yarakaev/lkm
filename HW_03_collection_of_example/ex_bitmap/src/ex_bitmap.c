#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/gfp_types.h>
#include <linux/printk.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/bitmap.h>
#include <linux/random.h>
#include <linux/slab.h>
#define NR_ARRAY 2


unsigned long bitmap_array[NR_ARRAY];

static char* bin_view(unsigned long num) {
    char* buffer = kmalloc(sizeof(unsigned long) * 8 + 1, GFP_KERNEL); 
    if (!buffer) return NULL; 
    
    int bits = sizeof(num) * 8;
    for (int i = 0; i < bits; i++) {
        buffer[i] = (num & (1UL << (bits - 1 - i))) ? '1' : '0';
    }
    buffer[bits] = '\0';
    return buffer;
}


static inline unsigned long get_random_ulong(void)
{
    unsigned long val;
    get_random_bytes(&val, sizeof(val));
    return val;
}

static void set_first_bit(void) {

    for(int i = 0; i < NR_ARRAY; i++) {
        bitmap_set(&bitmap_array[i], 0, 1);
    }

}

static void check_first_bit(void) {
    for(int i = 0; i < NR_ARRAY; i++) {
        if (test_bit(0, &bitmap_array[i])) {
            pr_info("First bit is setup. Value 0x%lx\n", bitmap_array[i]);
            char *str = bin_view(bitmap_array[i]); 
            if (str) {
                pr_info("Value is %s\n", str); 
                kfree(str); 
            }
        } else {
            pr_warn("First bit isn't setup. Value 0x%lx\n", bitmap_array[i]);
        }
    }
}



static __init int init_ex_bitmap(void) {
    pr_info("Create bitmap array\n");
    for (int i = 0; i < NR_ARRAY; i++) {
        bitmap_array[i] = get_random_ulong();
        char *str = bin_view(bitmap_array[i]); 
        if (str) {
            pr_info("Value is %s\n", str);
            kfree(str); 
        }
    }
    set_first_bit();
    check_first_bit();

    return 0;
}

static __exit void exit_ex_bitmap(void) {
    pr_info("Bb\n");
}



module_init(init_ex_bitmap);
module_exit(exit_ex_bitmap);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("HW-03 example bitmaps in linux module");
