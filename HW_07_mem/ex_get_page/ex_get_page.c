#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/gfp.h>
#include <linux/ktime.h>
#include <linux/mm.h>
#include <linux/module.h>

static int __init ex_init(void) {
  unsigned int order = 0; // Start with 1 page (4KB typically)
  size_t size;
  unsigned long ptr = 0;
  ktime_t start, end;
  s64 delta;
  while (order < 20) { // Limit to reasonable max order
    size = PAGE_SIZE << order;
    pr_info("get_page: %zu byte\n", size);
    start = ktime_get();
    ptr = __get_free_pages(GFP_KERNEL, order);
    end = ktime_get();
    delta = ktime_to_ns(ktime_sub(end, start)) / 1000000;
    if (ptr) {
      pr_info("get_page: SUCCESS\n");
      pr_info("get_page: %zu byte, %lld ms, type: physical contiguous pages\n",
             size, delta);
      free_pages(ptr, order);
      order++;
    } else {
      printk(KERN_INFO "get_page: FAIL, err_msg = out of memory\n");
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
MODULE_DESCRIPTION("HW_07_mem example  get_page allocation");
