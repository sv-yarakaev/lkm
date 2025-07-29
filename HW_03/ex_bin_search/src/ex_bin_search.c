/*
    Использование bsearch.
    Поиск Pid процесса. Массив pid формируется в момент старта модуля.
    Есть два параметра для поиска pid: extern_pid и output_string

    Замечание. Массив не сортируется, так как  заполняется уже отсортированым

*/
#include "linux/kern_levels.h"
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/moduleparam.h>
#include <asm-generic/errno-base.h>
#include <linux/llist.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/gfp_types.h>
#include <linux/printk.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/random.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/bsearch.h>
#include <linux/kstrtox.h>


char *find_pid = "Find PID= %d ";
const char *notfind_pid = "PID %d not found";
static char output_string[64] = "Enter PID to extern_pid to find";

static int *array = NULL;
static int common_size;

static int extern_pid = -1;

static int compare_pids(const void *a, const void *b);
static int get_put(char *buffer, const struct kernel_param *kp);


static int compare_pids(const void *a, const void *b) {
    int arg1 = *(const int*)a;
    int arg2 = *(const int*)b;
    if (arg1 > arg2) 
        return -1;  
    if (arg1 < arg2) 
        return 1;
    return 0;
}




static int pid_search(const char *val, const struct kernel_param *kp) {

    int find_pid;
    int ret = kstrtoint(val, 10, &find_pid);
    int *result = (int *) bsearch(&ret, array, common_size, sizeof(int), compare_pids); 

    if (result != NULL) {
        //char buf[100];
        snprintf(output_string,sizeof(output_string), "Find PID = %d ", ret);
        get_put(output_string, kp);
        printk(KERN_INFO "Find PID = %d \n", ret);


    } else {
        snprintf(output_string,sizeof(output_string), "Not found PID = %d ", ret);
        get_put(output_string, kp);
        printk(KERN_INFO "PID = %d not found \n", ret);
    }

    return 0;
}


static const struct kernel_param_ops search_pid = {
    .set = pid_search,
    .get = NULL,
};


struct pids_snap {
    int pid;
    struct llist_node list;
};
static LLIST_HEAD(pids_list);





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
        llist_add(&local->list, &pids_list);
        count++;
    }
    rcu_read_unlock();

    common_size = count;
       
    return count;
}

static void free_pid_list(void) {
    struct llist_node *node, *tmp;
    struct pids_snap *entry;

    llist_for_each_safe(node, tmp, llist_del_all(&pids_list)) {
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
    llist_for_each_entry(entry, pids_list.first, list) {
        printk(KERN_INFO "%d -> Add PID: %d\n", i, entry->pid);
        array[i] = entry->pid;
        i++;
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
