#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include "linux/moduleparam.h"
#include "asm-generic/errno-base.h"
#include "linux/llist.h"
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/gfp_types.h>
#include <linux/printk.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/random.h>
#include <linux/types.h>
#include "linux/sched.h"

const char *find_pid = "Find PID= %d ";
const char *notfind_pid = "PID %d not found";
static char output_string[64] = "Enter PID to extern_pid to find";

static int extern_pid = -1;
static int pid_search(const char *val, const struct kernel_param *kp) {

    return 0;
}

static const struct kernel_param_ops search_pid = {
    .set = pid_search,
    .get = NULL,
};



static int *array = NULL;

struct pids_snap {
    int pid;
    struct llist_node list;
};
static LLIST_HEAD(my_list);


static int count_processes(void) {
    struct task_struct *task;
    int count = 0;

    rcu_read_lock();
    //NOLINTNEXTLINE(clang-analyzer-sizeof-pointer)
    for_each_process(task) {
        struct pids_snap *local = kmalloc(sizeof(struct pids_snap), GFP_KERNEL);
        if (!local) {
            continue; // заменить
        }
        local->pid = task->pid;
        llist_add(&local->list, &my_list);
        count++;
    }
    rcu_read_unlock();

    return count;
}

static void free_pid_list(void) {
    struct llist_node *node, *tmp;
    struct pids_snap *entry;

    llist_for_each_safe(node, tmp, llist_del_all(&my_list)) {
        entry = container_of(node, struct pids_snap, list);
        pr_info("Freeing PID %d\n", entry->pid);
        kfree(entry);
    }
}

static int create_array(void) {
    int i = 0;
    struct pids_snap *entry;
    int prepare_size = count_processes();
    array = kmalloc(prepare_size * sizeof(int), GFP_KERNEL);
    if (!array) {
        pr_err("kmalloc failed, cannot create array");
        return -ENOMEM;
    }
    llist_for_each_entry(entry, my_list.first, list) {
        printk(KERN_INFO "Add PID: %d\n", entry->pid);
        array[i] = entry->pid;
    }
    return 0;
    
}


static int __init exbin_search_init(void) {
  pr_info("Init. Example kernel binary search\n");
  pr_info("Find PID on start module\n");
  create_array();
    
  return 0;
}

static void __exit exbin_search_exit(void) {
  kfree(array);
  free_pid_list();
  pr_info("Exit.\n");
}
static int get_put(char *buffer, const struct kernel_param *kp) {
    strncpy(buffer, output_string, strlen(output_string));
    return strlen(output_string);
}

static const struct kernel_param_ops output_param = {
    .get = get_put,
    .set = NULL,
};

module_param_cb(output_string, &output_param, &output_string, 0444);
MODULE_PARM_DESC(get_put, "Find PID");

module_param_cb(extern_pid, &search_pid, &extern_pid, 0660);
MODULE_PARM_DESC(extern_pid, "PID for search");


module_init(exbin_search_init);
module_exit(exbin_search_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stv");
MODULE_DESCRIPTION("HW_3 ex_bin_search");
