
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>     // task_struct
#include <linux/mm.h>        // vm_area_struct, mm_struct
#include <linux/mm_types.h>  // mmap_lock
#include <linux/kthread.h>   // Для примера с потоком
#include <linux/rbtree.h>


#include <linux/printk.h>
#include "asm-generic/errno-base.h"
#include "random_values.h"

struct task_snap {
    pid_t pid;
    struct mm_struct *mm;
    struct list_head list;    
};

struct mm_pid_node {
    struct rb_node node;
    struct mm_struct *mm;
    pid_t pid;
};

struct mm_pid_tree {
    struct rb_root root;      
};

static inline void mm_pid_tree_init(struct mm_pid_tree *tree)
{
    tree->root = RB_ROOT;
}




static LIST_HEAD(task_snap_head);

static int take_task_snap(void)
{
    struct task_struct *task;
    struct task_snap *snap;
    
    rcu_read_lock();
    for_each_process(task) {
        snap = kmalloc(sizeof(*snap), GFP_ATOMIC);
        if (!snap) {
            rcu_read_unlock();
            return -ENOMEM;
        }
        
        snap->pid = task->pid;
        snap->mm = task->mm;
        
        list_add(&snap->list, &task_snap_head);
    }
    rcu_read_unlock();
    
    return 0;
}



static int ex_rb_init(void) {
    if (!take_task_snap()) {
        pr_info("Take partial snap is succesfull\n");
    } else {
        pr_warn("Cannot take snapshots\n");
        return -ENOMEM; // надо поискать что-то более подходящее
    }
    
    return 0;
}

static void ex_rb_exit(void) {
//    kfree(task_snap);
    pr_info("Bb!");
}


module_init(ex_rb_init);
module_exit(ex_rb_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stv");
MODULE_DESCRIPTION("HW_3 example rb tree");
