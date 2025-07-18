#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include "linux/gfp_types.h"
#include "linux/printk.h"
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/random.h>



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

static char* create_random_id(void) {
  const u8 length_string = 8;
  u8 count = 0;
  char *ID = (char *)kmalloc((length_string + 1) * sizeof(char), GFP_KERNEL);  // +1 для '\0'
  if (ID == NULL) {
    pr_alert("Cannot alloc memory");
  }

  while( count < length_string) {
    ID[count] = get_random_alnum();
    count++;
  }
  ID[length_string] ='\0';
  return ID;
}


static int __init exlist_init(void) {
  pr_info("Init. Example kernel list\n");
  pr_info("%s", create_random_id());
  return 0;
}

static void __exit exlist_exit(void) {
  pr_info("Exit.\n");
}

module_init(exlist_init);
module_exit(exlist_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stv");
MODULE_DESCRIPTION("HW_3 ex_list");
