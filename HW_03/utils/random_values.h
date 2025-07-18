#include <linux/random.h>

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