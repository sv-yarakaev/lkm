#include <linux/module.h>
#include <linux/slab.h>
#include <linux/ktime.h>
#include <linux/mm.h>


static int __init ex_init(void) {
    size_t size = 1024;
    struct kmem_cache *cache = NULL;
    void *ptr = NULL;
    ktime_t start, end;
    s64 delta;
    while (size < (1UL << 32)) {
        pr_info("kmem_cache: %zu byte\n", size);
        cache = kmem_cache_create("ex_cache", size, 0, 0, NULL);
        if (!cache) {
            pr_info("kmem_cache: FAIL, err_msg = cache create failed\n");
            break;
        }
        start = ktime_get();
        ptr = kmem_cache_alloc(cache, GFP_KERNEL);
        end = ktime_get();
        delta = ktime_to_ns(ktime_sub(end, start)) / 1000000;
        if (ptr) {
            pr_info("kmem_cache: SUCCESS\n");
            pr_info("kmem_cache: %zu byte, %lld ms, type: object cached physical\n", size, delta);
            kmem_cache_free(cache, ptr);
            kmem_cache_destroy(cache);
            size *= 2;
        } else {
            pr_info("kmem_cache: FAIL, err_msg = out of memory\n");
            kmem_cache_destroy(cache);
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
MODULE_DESCRIPTION("HW_07 kmem_cache");


