#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/delay.h> // msleep
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h> // kthreads
#include <linux/list.h>
#include <linux/module.h>
#include <linux/mutex.h>  // struct mutex
#include <linux/random.h> // get_random_u32
#include <linux/slab.h>   // kmalloc/kfree

#define COUNT 42
#define NUM_NAMES 24
#define NAME_LENGTH 20
#define READERS 5
#define WRITERS 2
#define MAX_ATTEMPTS 100 // Ограничение попыток, чтобы избежать infinite loop

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
static struct task_struct *reader_threads[READERS];
static struct task_struct *writer_threads[WRITERS];


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

__attribute__((unused)) static int
record_db_update_by_id(long old_id, long new_id, const char *new_name) {
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



static int __init rw_module_init(void) {


  if (init_db() != 0) {
    pr_err("Failed to init DB\n");
    return -ENOMEM;
  }

 return 0;
}

static void __exit rw_module_exit(void) {
  print_db();
  free_db();

  pr_info("RW module unloaded\n");
}

module_init(rw_module_init);
module_exit(rw_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stv");
MODULE_DESCRIPTION("HW-04");
