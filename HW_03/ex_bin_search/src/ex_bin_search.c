#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
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
        printk(KERN_INFO "PID: %d\n", entry->pid);
        array[i] = entry->pid;
    }
    return 0;
    
}


static int __init exbin_search_init(void) {
  pr_info("Init. Example kernel list\n");
  create_array();
    
  return 0;
}

static void __exit exbin_search_exit(void) {
  kfree(array);
  free_pid_list();
  pr_info("Exit.\n");
}

module_init(exbin_search_init);
module_exit(exbin_search_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stv");
MODULE_DESCRIPTION("HW_3 ex_bin_search");
