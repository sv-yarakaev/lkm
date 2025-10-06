#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/ktime.h>
#include <linux/mm.h>


static int __init ex_init(void) {
    size_t size = 1024;
    void *ptr = NULL;
    ktime_t start, end;
    s64 delta;
    while (size < (1UL << 35)) {  
        pr_info("vmalloc: %zu byte\n", size);
        start = ktime_get();
        ptr = vmalloc(size);
        end = ktime_get();
        delta = ktime_to_ns(ktime_sub(end, start)) / 1000000;
        if (ptr) {
            pr_info("vmalloc: SUCCESS\n");
            pr_info("vmalloc: %zu byte, %lld ms, type: virtual non-contiguous\n", size, delta);
            vfree(ptr);
            size *= 2;
        } else {
            pr_crit("vmalloc: FAIL, err_msg = out of memory\n");
            break;
        }
    }
    return 0;
}

static void __exit ex_exit(void) {
    pr_info("BB\n");
}

module_init(ex_init);
module_exit(ex_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stv");
MODULE_DESCRIPTION("HW_07 vmalloc allocation");

s

