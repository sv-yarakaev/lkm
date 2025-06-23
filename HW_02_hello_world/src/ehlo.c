#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/stat.h>
#include <linux/string.h>

#define MAX_STR_LEN 100

static int idx = 0;
static char value = '\0';
static char str[MAX_STR_LEN] = "default";

static struct {
  int idx;
  int val;
} check;

static void modify_string(void)
{
    size_t len;

    if (idx < 1 || idx > MAX_STR_LEN)
        return;

    if (value < 32 || value > 126)
        return;


    len = strlen(str);

    if (idx <= len) {
        str[idx - 1] = value;
        pr_info("Modified string[%d] = '%c' -> \"%s\"\n", idx - 1, value, str);
    } else {
        pr_warn("Index %d out of bounds (length: %zu)\n", idx, len);
    }

   
}




// Проверка и применение idx (должен быть в пределах длины str)
static int param_set_idx(const char *val, const struct kernel_param *kp) {
  int new_idx, ret;
  ret = kstrtoint(val, 10, &new_idx); // Преобразуем строку в int
  if (ret < 0) {
    pr_err("Invalid idx value\n");
    return ret;
  }

  if (new_idx < 0 || new_idx >= strlen(str)) {
    pr_err("idx %d is out of bounds (str len: %zu)\n", new_idx, strlen(str));
    return -EINVAL;
  }
  check.idx = 1;

  *(int *)kp->arg = new_idx; // Устанавливаем новое значение
  return 0;
}

// Получение текущего idx
static int param_get_idx(char *buffer, const struct kernel_param *kp) {
  return sprintf(buffer, "%d", *(int *)kp->arg);
}

// Проверка и применение value (должен быть печатным ASCII-символом)
static int param_set_value(const char *val, const struct kernel_param *kp) {
  
  if (!val ) {
    pr_err("value must be a single character\n");
    return -EINVAL;
  }

  //new_value = val;
  if (*val < 32 || *val > 126) {
    pr_err("value must be a printable ASCII character\n");
    return -EINVAL;
  }
  check.val = 1;
  *(char *)kp->arg = *val; // Устанавливаем новое значение
  return 0;
}

// Получение текущего value
static int param_get_value(char *buffer, const struct kernel_param *kp) {
  return sprintf(buffer, "%c", *(char *)kp->arg);
}

// Проверка и применение str (длина не должна превышать MAX_STR_LEN)
// static int param_set_str(const char *val, const struct kernel_param *kp) {
//   if (strlen(val) >= MAX_STR_LEN) {
//     pr_err("str too long (max %d chars)\n", MAX_STR_LEN - 1);
//     return -EINVAL;
//   }

//   strncpy(str, val, MAX_STR_LEN);
//   return 0;
// }

// Получение текущего str
static int param_get_str(char *buffer, const struct kernel_param *kp) {
  strncpy(buffer, str, MAX_STR_LEN);
  return strlen(buffer);
}

//--- Объявление параметров с callback ---//
static const struct kernel_param_ops param_ops_idx = {
    .set = param_set_idx,
    .get = param_get_idx,
};

static const struct kernel_param_ops param_ops_value = {
    .set = param_set_value,
    .get = param_get_value,
};

static const struct kernel_param_ops param_str = {
    .set = NULL,
    .get = param_get_str,
};

module_param_cb(idx, &param_ops_idx, &idx, 0644);
MODULE_PARM_DESC(idx, "Index in str (0 <= idx < MAX_STR_LEN");

module_param_cb(value, &param_ops_value, &value, 0644);
MODULE_PARM_DESC(value, "Character to insert into str (must be printable ASCII)");

module_param_cb(str, &param_str, &str, 0444);
MODULE_PARM_DESC(str, "String(RO) to modify only idx and char (max 100 chars)");

//---------------------------------------
// Инициализация и выход
//---------------------------------------
static int __init ehlo_init(void) {
  printk(KERN_INFO "(default values) Module loaded: idx=%d, value='%c', str=\"%s\"\n", idx,
         value, str);
  return 0;
}

static void __exit ehlo_exit(void) { pr_info("module is free\n"); }

module_init(ehlo_init);
module_exit(ehlo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stanislav");
MODULE_DESCRIPTION("A homework hello(ehlo)  module for the Linux kernel");
MODULE_VERSION("0.2.1");
