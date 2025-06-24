#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/uaccess.h>



#define MAX_LEN 100

static char string[MAX_LEN + 1] = "Default string!";



static int string_get(char *buffer, const struct kernel_param *kp)
{
    strncpy(buffer, string, MAX_LEN);
    return strlen(buffer);
}

static const struct kernel_param_ops string_ops = {
    .get = string_get,
    .set = NULL,  
};

module_param_cb(string, &string_ops, &string, 0444);
MODULE_PARM_DESC(string, "String (RO), modified only via index and char");

static int index = -1;
static char char_param = '\0';


static void modify_string(void)
{
    size_t len;

    if (index < 1 || index > MAX_LEN)
        return;

    if (char_param < 32 || char_param > 126)
        return;

    len = strlen(string);

    if (index <= len) {
        string[index - 1] = char_param;
        pr_info("Modified string[%d] = '%c' -> \"%s\"\n", index - 1, char_param, string);
    } else {
        pr_warn("Index %d out of bounds (length: %zu)\n", index, len);
    }
    
}


static int index_set(const char *val, const struct kernel_param *kp)
{
    int ret, temp;

    ret = kstrtoint(val, 10, &temp);
    if (ret < 0)
        return ret;

    if (temp < 1 || temp > MAX_LEN)
        return -EINVAL;

    index = temp;
    modify_string();  

    return 0;
}

static int index_get(char *buffer, const struct kernel_param *kp)
{
    return sprintf(buffer, "%d", index);
}

static const struct kernel_param_ops index_ops = {
    .set = index_set,
    .get = index_get,
};

module_param_cb(index, &index_ops, &index, 0660);
MODULE_PARM_DESC(index, "Index from 1 to 100");


static int char_set(const char *val, const struct kernel_param *kp)
{
    if (!val || val[0] == '\0' || val[1] != '\0')
        return -EINVAL;

    if (val[0] < 32 || val[0] > 126)
        return -EINVAL;

    char_param = val[0];
    modify_string();  

    return 0;
}

static int char_get(char *buffer, const struct kernel_param *kp)
{
    return sprintf(buffer, "%c", char_param);
}

static const struct kernel_param_ops char_ops = {
    .set = char_set,
    .get = char_get,
};

module_param_cb(char, &char_ops, &char_param, 0660);
MODULE_PARM_DESC(char, "Visible ASCII character");

static int __init mymodule_init(void)
{
    pr_info("Module loaded. Initial string: \"%s\"\n", string);
    return 0;
}

static void __exit mymodule_exit(void)
{
    pr_info("Module exiting. Final string: \"%s\"\n", string);
}

module_init(mymodule_init);
module_exit(mymodule_exit);



MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stv");
MODULE_DESCRIPTION("HW 2 ");
MODULE_VERSION("0.3");