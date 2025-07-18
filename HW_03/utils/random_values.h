#include <linux/slab.h>
#include <linux/random.h>

static char* create_random_id(const u8 length);


static char get_random_visible_char(void) {
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
    ID[count] = get_random_visible_char();
    count++;
  }
  ID[length] ='\0';
  return ID;
}
