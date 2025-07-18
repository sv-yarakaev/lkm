#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include "linux/list.h"
#include "linux/slab.h"
#include "linux/gfp_types.h"
#include "linux/printk.h"
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/random.h>
#include "linux/types.h"

#define LENGTH_ID 8
#define LENGTH_LIST 124

struct rnd_list {
  char *ID;
  struct list_head iter;
};
static LIST_HEAD(rnd_list);

static char get_random_alnum(void) {
    u8 random_byte;
    char c;
    
    do {
        random_byte = get_random_u8();
        c = random_byte % 62;
    } while (random_byte >= 62 * 4); 
        
    if (c < 10) {
        return '0' + c;      // 0-9
    } else if (c < 36) {
        return 'A' + (c - 10); // A-Z
    } else {
        return 'a' + (c - 36); // a-z
    }
}

static char* create_random_id(const u8 length) {
  
  if (length == 0 || length > 255) {  
    pr_warn("Invalid length: %u\n", length);
    return NULL;
  }
  //const u8 length_string = 8;
  u8 count = 0;
  char *ID = (char *)kmalloc((length + 1) * sizeof(char), GFP_KERNEL);  // +1 для '\0'
  if (ID == NULL) {
    pr_alert("Cannot alloc memory");
    return NULL;
  }

  while( count < length) {
    ID[count] = get_random_alnum();
    count++;
  }
  ID[length] ='\0';
  return ID;
}

static int init_local_list(void) {

  struct rnd_list *rnd_one;
  int i;

  for(i = 0; i < LENGTH_LIST; i++) {
    rnd_one = kmalloc(sizeof(*rnd_one), GFP_KERNEL);
    if (!rnd_one) {
            pr_alert("Cannot allocate memory for rnd_list\n");
            goto error;
    }
    rnd_one->ID = create_random_id(LENGTH_ID);
    if (!rnd_one->ID) {
            pr_alert("Cannot create random ID\n");
            kfree(rnd_one);
            goto error;
    }
    INIT_LIST_HEAD(&rnd_one->iter);
    list_add_tail(&rnd_one->iter, &rnd_list);
    pr_info("\tID = %s\n", rnd_one->ID);
  }
  return 0;
  error:
    return -1;
}

static void cleanup_list(void) {
    struct rnd_list *rnd_current, *rnd_tmp;
    
    list_for_each_entry_safe(rnd_current, rnd_tmp, &rnd_list, iter) {
        list_del(&rnd_current->iter);
        kfree(rnd_current->ID);
        kfree(rnd_current);
    }
}

static int __init exlist_init(void) {
  pr_info("Init. Example kernel list\n");
  pr_info("Length of my list %d\n", LENGTH_LIST);
  const char *output_random = create_random_id(LENGTH_ID);
  pr_info("%s", output_random);
  kfree(output_random);
  if (init_local_list() != 0) {
    return -1;
  }
  return 0;
}

static void __exit exlist_exit(void) {
  cleanup_list();
  pr_info("Exit.\n");
}

module_init(exlist_init);
module_exit(exlist_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stv");
MODULE_DESCRIPTION("HW_3 ex_list");
