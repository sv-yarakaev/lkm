#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/uaccess.h>

#define MAX_LEN 100

static char my_str[MAX_LEN + 1] = "Default string!";
static int idx = -1;
static char ch_val = '\0';

static int my_str_get(char *buffer, const struct kernel_param *kp) {
  strncpy(buffer, my_str, MAX_LEN);
  return strlen(buffer);
}

static const struct kernel_param_ops my_str_ops = {
    .get = my_str_get,
    .set = NULL,
};

module_param_cb(my_str, &my_str_ops, &my_str, 0444);
MODULE_PARM_DESC(my_str, "String (RO), modified only via idx and char");

static void modify_my_str(void) {
  size_t len;

  if (idx < 1 || idx > MAX_LEN)
    return;

  if (ch_val < 32 || ch_val > 126)
    return;

  len = strlen(my_str);

  if (idx <= len) {
    my_str[idx - 1] = ch_val;
    pr_info("Modified string[%d] = '%c' -> \"%s\"\n", idx - 1, ch_val, my_str);
  } else {
    pr_warn("idx %d out of bounds (length: %zu)\n", idx, len);
  }
}

static int idx_set(const char *val, const struct kernel_param *kp) {
  int ret, temp;

  ret = kstrtoint(val, 10, &temp);
  if (ret < 0)
    return ret;

  if (temp < 1 || temp > MAX_LEN)
    return -EINVAL;

  idx = temp;
  modify_my_str();

  return 0;
}

static int idx_get(char *buffer, const struct kernel_param *kp) {
  return sprintf(buffer, "%d", idx);
}

static const struct kernel_param_ops idx_ops = {
    .set = idx_set,
    .get = idx_get,
};

module_param_cb(idx, &idx_ops, &idx, 0660);
MODULE_PARM_DESC(idx, "idx from 1 to 100");

static int char_set(const char *val, const struct kernel_param *kp) {
  if (!val || val[0] == '\0' || val[1] != '\0')
    return -EINVAL;

  if (val[0] < 32 || val[0] > 126)
    return -EINVAL;

  ch_val = val[0];
  modify_my_str();

  return 0;
}

static int char_get(char *buffer, const struct kernel_param *kp) {
  return sprintf(buffer, "%c", ch_val);
}

static const struct kernel_param_ops char_ops = {
    .set = char_set,
    .get = char_get,
};

module_param_cb(char, &char_ops, &ch_val, 0660);
MODULE_PARM_DESC(char, "Visible ASCII character");

static int __init ehlo_init(void) {
  pr_info(":init. Initial string: \"%s\"\n", my_str);
  return 0;
}

static void __exit ehlo_exit(void) {
  pr_info(":exit. Final string: \"%s\"\n", my_str);
}

module_init(ehlo_init);
module_exit(ehlo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stv");
MODULE_DESCRIPTION("HW 2 ");
MODULE_VERSION("0.3");