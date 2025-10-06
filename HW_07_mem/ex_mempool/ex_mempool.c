#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/ktime.h>
#include <linux/mempool.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/slab.h>

static int __init ex_init(void) {
  size_t size = 1024;
  struct kmem_cache *cache = NULL;
  mempool_t *pool = NULL;
  void *ptr = NULL;
  ktime_t start, end;
  s64 delta;
  while (size < (1UL << 32)) {
    pr_info("mempool: %zu byte\n", size);
    cache = kmem_cache_create("ex_mempool_cache", size, 0, 0, NULL);
    if (!cache) {
      pr_crit("mempool: FAIL, err_msg = cache create failed\n");
      break;
    }
    pool = mempool_create(1, mempool_alloc_slab, mempool_free_slab, cache);
    if (!pool) {
      pr_crit("mempool: FAIL, err_msg = pool create failed\n");
      kmem_cache_destroy(cache);
      break;
    }
    start = ktime_get();
    ptr = mempool_alloc(pool, GFP_KERNEL);
    end = ktime_get();
    delta = ktime_to_ns(ktime_sub(end, start)) / 1000000;
    if (ptr) {
      pr_info("mempool: SUCCESS\n");
      pr_info("mempool: %zu byte, %lld ms, type: pooled object cached\n", size,
              delta);
      mempool_free(ptr, pool);
      mempool_destroy(pool);
      kmem_cache_destroy(cache);
      size *= 2;
    } else {
      pr_crit("mempool: FAIL, err_msg = out of memory\n");
      mempool_destroy(pool);
      kmem_cache_destroy(cache);
      break;
    }
  }
  return 0;
}

static void __exit ex_exit(void) {}

module_init(ex_init);
module_exit(ex_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stv");
MODULE_DESCRIPTION("HW_07 mempool allocation");
