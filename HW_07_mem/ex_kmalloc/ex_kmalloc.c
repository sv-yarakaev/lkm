#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/ktime.h>
#include <linux/mm.h>


static int __init ex_init(void) {
    size_t size = 1024;
    void *ptr = NULL;
    ktime_t start, end;
    s64 delta;
    while (size < (1UL << 32)) {  
        pr_info("kmalloc: %zu byte\n", size);
        start = ktime_get();
        ptr = kmalloc(size, GFP_KERNEL);
        end = ktime_get();
        delta = ktime_to_ns(ktime_sub(end, start)) / 1000000;  // ns to ms
        if (ptr) {
            pr_info("kmalloc: SUCCESS\n");
            pr_info("kmalloc: %zu byte, %lld ms, type: physical contiguous\n", size, delta);
            kfree(ptr);
            size *= 2;
        } else {
            pr_crit("kmalloc: FAIL, err_msg = out of memory\n");
            break;
        }
    }
    return 0;
}

static void __exit ex_exit(void) {
}

module_init(ex_init);
module_exit(ex_exit);  
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stv");
MODULE_DESCRIPTION("HW_07 kmalloc allocation");


