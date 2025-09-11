#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/slab.h>    // kmalloc/kfree
#include <linux/mutex.h>   // struct mutex
#include <linux/kthread.h> // kthreads
#include <linux/delay.h>   // msleep
#include <linux/random.h>  // get_random_u32

#define COUNT 42
#define NUM_NAMES 24
#define NAME_LENGTH 20
#define READERS 5
#define WRITERS 2
#define MAX_ATTEMPTS 100  // Ограничение попыток, чтобы избежать infinite loop

static const char *random_names[] = {
    "Alice", "Bob",   "Charlie", "David",    "Eva",     "Frank",
    "Grace", "Henry", "Ivy",     "Jack",     "Kate",    "Liam",
    "Mia",   "Noah",  "Olivia",  "Peter",    "Quinn",   "Rachel",
    "Sam",   "Tina",  "Ulysses", "Victoria", "William", "Zoe"};

typedef struct simple_ll_db {
    char *name;
    long id;
    struct list_head node;
} record_db_t;

static LIST_HEAD(g_records);

static struct mutex rmutex;
static struct mutex rw_mutex;
static int readers_count = 0;

static record_db_t *record_db_new(const char *name, long id) {
    record_db_t *rec = kmalloc(sizeof(*rec), GFP_KERNEL);
    if (!rec)
        return NULL;

    rec->name = kstrdup(name ? name : "", GFP_KERNEL);
    if (!rec->name) {
        kfree(rec);
        return NULL;
    }
    rec->id = id;
    return rec;
}

static int record_db_add(const char *name, long id) {
    record_db_t *rec = record_db_new(name, id);
    if (!rec)
        return -1;

    list_add_tail(&rec->node, &g_records);
    return 0;
}

static void record_db_free(record_db_t *rec) {
    if (!rec)
        return;
    kfree(rec->name);
    kfree(rec);
}

static record_db_t *record_db_find_by_id(long id) {
    record_db_t *pos;

    list_for_each_entry(pos, &g_records, node) {
        if (pos->id == id) {
            return pos;
        }
    }
    return NULL;
}

static void print_db(void) {
    record_db_t *pos;

    pr_info("=== Local Simple DataBase ===\n");
    list_for_each_entry(pos, &g_records, node) {
        pr_info("record: id=%ld name=%s\n", pos->id, pos->name);
    }
    pr_info("===============================\n");
}

__attribute__((unused))
static int record_db_update_by_id(long old_id, long new_id, const char *new_name) {
    record_db_t *pos;
    char *dup;
    const char *nm = new_name ? new_name : "";

    dup = kstrdup(nm, GFP_KERNEL);
    if (!dup)
        return -1;

    list_for_each_entry(pos, &g_records, node) {
        if (pos->id == old_id) {
            char *old = pos->name;
            pos->id = new_id;
            pos->name = dup;
            kfree(old);
            return 0;
        }
    }

    kfree(dup);
    return -1;
}

static int init_db(void) {
    int i;
    for (i = 0; i < COUNT; i++) {
        u32 name_index = get_random_u32() % NUM_NAMES;
        char name[NAME_LENGTH];
        snprintf(name, NAME_LENGTH, "%s", random_names[name_index]);
        long id = 1000 + (get_random_u32() % 9000);
        if (record_db_add(name, id) != 0) {
            pr_err("Failed to add record %d\n", i);
            return -ENOMEM;
        }
    }
    return 0;
}

static void free_db(void) {
    record_db_t *pos, *n;
    list_for_each_entry_safe(pos, n, &g_records, node) {
        list_del(&pos->node);
        record_db_free(pos);
    }
}

static int reader_thread(void *arg) {
    int id_local = *(int *)arg;
    int attempts = 0;
    kfree(arg);

    while (!kthread_should_stop() && attempts < MAX_ATTEMPTS) {
        long idx = 1000 + (get_random_u32() % 9000);  

        mutex_lock(&rmutex);
        readers_count++;
        if (readers_count == 1) {
            mutex_lock(&rw_mutex);
        }
        mutex_unlock(&rmutex);

        record_db_t *rec = record_db_find_by_id(idx);

        mutex_lock(&rmutex);
        readers_count--;
        if (readers_count == 0) {
            mutex_unlock(&rw_mutex);
        }
        mutex_unlock(&rmutex);

        if (rec == NULL) {
            pr_info("\tReader %d: id = %ld not found\n", id_local, idx);
            msleep(100);  
            attempts++;
            continue;
        } else {
            pr_info("[R] Find record %d: read idx=%ld name=%s id=%ld\n",
                   id_local, idx, rec->name, rec->id);
            break;  
        }
    }

    if (attempts >= MAX_ATTEMPTS) {
        pr_warn("Reader %d: max attempts reached\n", id_local);
    }

    return 0;  
}

static int writer_thread(void *arg) {
    int id_local = *(int *)arg;
    int attempts = 0;
    kfree(arg);

    while (!kthread_should_stop() && attempts < MAX_ATTEMPTS) {
        long idx = 1000 + (get_random_u32() % 9000);

        mutex_lock(&rw_mutex);

        record_db_t *rec = record_db_find_by_id(idx);

        if (rec != NULL) {
            u32 name_index = get_random_u32() % NUM_NAMES;
            char *new_name = kstrdup(random_names[name_index], GFP_KERNEL);
            if (!new_name) {
                pr_err("[W] Memory allocation failed\n");
                mutex_unlock(&rw_mutex);
                break;
            }
            pr_info("[W] Find record %d: id = %ld name = %s\n",
                   id_local, rec->id, rec->name);
            kfree(rec->name);
            rec->name = new_name;
            pr_info("\tChange name in record: %s\n", rec->name);
            mutex_unlock(&rw_mutex);
            break; 
        } else {
            mutex_unlock(&rw_mutex);
            pr_info("\tWriter %d: id = %ld not found\n", id_local, idx);
            msleep(100);
            attempts++;
        }
    }

    if (attempts >= MAX_ATTEMPTS) {
        pr_warn("Writer %d: max attempts reached\n", id_local);
    }

    return 0;  
}

static struct task_struct *readers[READERS];
static struct task_struct *writers[WRITERS];

static int __init rw_module_init(void) {
    int i;

    mutex_init(&rmutex);
    mutex_init(&rw_mutex);

    if (init_db() != 0) {
        pr_err("Failed to init DB\n");
        return -ENOMEM;
    }

    pr_info("Creating %d readers and %d writers\n", READERS, WRITERS);

    for (i = 0; i < READERS; i++) {
        int *id = kmalloc(sizeof(int), GFP_KERNEL);
        if (!id) {
            pr_err("Failed to alloc id for reader %d\n", i);
            goto cleanup;
        }
        *id = i + 1;
        readers[i] = kthread_run(reader_thread, id, "reader_%d", i);
        if (IS_ERR(readers[i])) {
            pr_err("Failed to create reader %d\n", i);
            kfree(id);
            goto cleanup;
        }
    }

    for (i = 0; i < WRITERS; i++) {
        int *id = kmalloc(sizeof(int), GFP_KERNEL);
        if (!id) {
            pr_err("Failed to alloc id for writer %d\n", i);
            goto cleanup;
        }
        *id = i + 1;
        writers[i] = kthread_run(writer_thread, id, "writer_%d", i);
        if (IS_ERR(writers[i])) {
            pr_err("Failed to create writer %d\n", i);
            kfree(id);
            goto cleanup;
        }
    }

    return 0;

cleanup:
    for (i = 0; i < READERS; i++) {
        if (readers[i] && !IS_ERR(readers[i]))
            kthread_stop(readers[i]);
    }
    for (i = 0; i < WRITERS; i++) {
        if (writers[i] && !IS_ERR(writers[i]))
            kthread_stop(writers[i]);
    }
    free_db();
    return -ENOMEM;
}

static void __exit rw_module_exit(void) {
    int i;

    for (i = 0; i < READERS; i++) {
        if (readers[i] && !IS_ERR(readers[i]))
            kthread_stop(readers[i]);
    }
    for (i = 0; i < WRITERS; i++) {
        if (writers[i] && !IS_ERR(writers[i]))
            kthread_stop(writers[i]);
    }

    print_db();
    free_db();

    pr_info("RW module unloaded\n");
}

module_init(rw_module_init);
module_exit(rw_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stv");
MODULE_DESCRIPTION("HW-04");
