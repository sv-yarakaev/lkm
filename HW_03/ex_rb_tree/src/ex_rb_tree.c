
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>     // task_struct
#include <linux/mm.h>        // vm_area_struct, mm_struct
#include <linux/mm_types.h>  // mmap_lock
#include <linux/kthread.h>   // Для примера с потоком
#include <linux/rbtree.h>


#include <linux/printk.h>
#include "asm-generic/errno-base.h"

/*
struct task_snap {
    pid_t pid;
    struct mm_struct *mm;
    struct list_head list;    
}; */

struct mm_pid_node {
    struct rb_node node;
    struct mm_struct *mm;
    pid_t pid;
};


struct mm_pid_tree {
    struct rb_root root;      
};
struct mm_pid_tree local_rb_tree; //



int mm_pid_tree_insert(struct mm_pid_tree *tree, pid_t pid, struct mm_struct *mm);
void mm_pid_tree_free(struct mm_pid_tree *tree);
//возможно удалить
static inline void mm_pid_tree_init(struct mm_pid_tree *tree)
{
    tree->root = RB_ROOT;
}

void mm_pid_tree_free(struct mm_pid_tree *tree)
{
    struct rb_node *node;

    if (RB_EMPTY_ROOT(&tree->root)) {
        pr_warn("free_rb_tree: tree is already empty\n");
        return;
    }


    for(node = rb_first(&tree->root); node;) {
        struct rb_node *next = rb_next(node);
        struct mm_pid_node *data = rb_entry(node, struct mm_pid_node, node);
        if (!data) {
            pr_warn("free_rb_tree: rb_entry returned NULL\n");
            node = next;
            continue;
        }
        pr_info("free_rb_tree: freeing node pid=%d mm=%p\n",
                data->pid, data->mm);
        rb_erase(node, &tree->root);
        kfree(data);
        node = next;
    }
    if (RB_EMPTY_ROOT(&tree->root))
        pr_info("free_rb_tree: tree cleanup complete, all nodes freed\n");
    else
        pr_warn("free_rb_tree: tree not empty after cleanup!\n");

}



int mm_pid_tree_insert(struct mm_pid_tree *tree, pid_t pid, struct mm_struct *mm)
{
    struct mm_pid_node *new_node;
    struct rb_node **link = &tree->root.rb_node, *parent = NULL;
    struct mm_pid_node *current_node;

    new_node = kmalloc(sizeof(struct mm_pid_node), GFP_KERNEL);
    if (!new_node)
        return -ENOMEM;

    new_node->pid = pid;
    new_node->mm = mm;

    while (*link) {
        parent = *link;
        current_node = rb_entry(parent, struct mm_pid_node, node);

        if (pid < current->pid)
            link = &parent->rb_left;
        else if (pid > current->pid)
            link = &parent->rb_right;
        else
            goto error_duplicate; 
    }

    rb_link_node(&new_node->node, parent, link);
    rb_insert_color(&new_node->node, &tree->root);

    return 0;

error_duplicate:
    kfree(new_node);
    return -EEXIST;
}



static LIST_HEAD(task_snap_head);

static int take_task_snap(void)
{
    struct task_struct *task;
    //struct task_snap *snap;
    int ret;
     
    rcu_read_lock();
    for_each_process(task) {
        if (task->mm) {
            pr_info("Insert task: %s: %d to rb tree\n", task->comm, task->pid);
            ret = mm_pid_tree_insert(&local_rb_tree, task->pid, task->mm);
        }
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
    mm_pid_tree_free(&local_rb_tree);
    pr_info("Bb!");
}


module_init(ex_rb_init);
module_exit(ex_rb_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stv");
MODULE_DESCRIPTION("HW_3 example rb tree");
