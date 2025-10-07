#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/random.h>
#include <linux/semaphore.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/jiffies.h>


/* Module parameters */
static int readers = 2;
static int writers = 2;
static int initial_count = 42;


#define NUM_NAMES 24
#define NAME_LENGTH 32

static const char *random_names[NUM_NAMES] = {
	"Alice","Bob","Charlie","David","Eva","Frank",
	"Grace","Henry","Ivy","Jack","Kate","Liam",
	"Mia","Noah","Olivia","Peter","Quinn","Rachel",
	"Sam","Tina","Ulysses","Victoria","William","Zoe"
};

/* === Simple linked-list database === */
typedef struct record_db {
	char *name;
	long id;
	struct list_head node;
} record_db_t;

static LIST_HEAD(g_records);

/* Synchronization */
static struct semaphore rmutex;    /* protects readers_count */
static struct semaphore rw_mutex;  /* writer exclusive lock */
static int readers_count = 0;

/* Wait queue for sleeping readers */
static wait_queue_head_t wq;

/* Thread handles */
static struct task_struct **reader_tasks;
static struct task_struct **writer_tasks;

static bool reader_stop = false;

/* --- Helpers --- */
static record_db_t *record_db_new(const char *name, long id)
{
	record_db_t *rec;
	rec = kmalloc(sizeof(*rec), GFP_KERNEL);
	if (!rec)
		return NULL;
	rec->name = kstrdup(name ? name : "", GFP_KERNEL);
	if (!rec->name) {
		kfree(rec);
		return NULL;
	}
	rec->id = id;
	INIT_LIST_HEAD(&rec->node);
	return rec;
}

static void record_db_free(record_db_t *rec)
{
	if (!rec)
		return;
	kfree(rec->name);
	kfree(rec);
}

static int record_db_add(const char *name, long id)
{
	record_db_t *rec = record_db_new(name, id);
	struct list_head *pos;
	if (!rec)
		return -ENOMEM;

	list_for_each(pos, &g_records) {
		record_db_t *existing = list_entry(pos, record_db_t, node);
		if (rec->id < existing->id) {
			list_add_tail(&rec->node, pos->prev);
			return 0;
		}
	}
	list_add_tail(&rec->node, &g_records);
	return 0;
}

static record_db_t *record_db_find_by_id(long id)
{
	record_db_t *pos;
	list_for_each_entry(pos, &g_records, node) {
		if (pos->id == id)
			return pos;
		if (pos->id > id)
			return NULL;
	}
	return NULL;
}

static void init_db(void)
{
	int i;
	u32 r;
	for (i = 0; i < initial_count; i++) {
		get_random_bytes(&r, sizeof(r));
		long id = 100 + (r % 900);
		get_random_bytes(&r, sizeof(r));
		const char *nm = random_names[r % NUM_NAMES];
		record_db_add(nm, id);
	}
	pr_info("rw_db: initialized %d records\n", initial_count);
}

static void free_db(void)
{
	record_db_t *pos, *n;
	list_for_each_entry_safe(pos, n, &g_records, node) {
		list_del(&pos->node);
		record_db_free(pos);
	}
	pr_info("rw_db: freed database\n");
}

/* === Reader thread === */
static int reader_fn(void *data)
{
	int id_local = (int)(long)data;
	unsigned int seed;
	get_random_bytes(&seed, sizeof(seed));

	pr_info("rw_db: reader %d started\n", id_local);

	while (!kthread_should_stop() && !reader_stop) {
		long idx;
		u32 r;
		get_random_bytes(&r, sizeof(r));
		idx = 100 + (r % 900);

		/* reader entry section */
		down(&rmutex);
		readers_count++;
		if (readers_count == 1)
			down(&rw_mutex); /* first reader blocks writers */
		up(&rmutex);

		record_db_t *found = record_db_find_by_id(idx);

		/* reader exit section */
		down(&rmutex);
		readers_count--;
		if (readers_count == 0)
			up(&rw_mutex);
		up(&rmutex);

		if (found) {
			pr_info("[R %d] found id=%ld name=%s — exiting\n",
				id_local, found->id, found->name);
			break; /* === exit after first success === */
		} else {
			pr_info("[R %d] id=%ld not found — waiting\n", id_local, idx);
			wait_event_interruptible_timeout(
				wq,
				kthread_should_stop() || reader_stop ||
					(record_db_find_by_id(idx) != NULL),
				msecs_to_jiffies(200));
			msleep(100);
		}

		cond_resched();
	}

	pr_info("rw_db: reader %d finished\n", id_local);
	return 0;
}

/* === Writer thread === */
static int writer_fn(void *data)
{
	int id_local = (int)(long)data;
	unsigned int seed;
	get_random_bytes(&seed, sizeof(seed));

	pr_info("rw_db: writer %d started\n", id_local);

	while (!kthread_should_stop()) {
		u32 r;
		long idx;
		get_random_bytes(&r, sizeof(r));
		idx = 100 + (r % 900);

		down(&rw_mutex);

		record_db_t *found = record_db_find_by_id(idx);
		if (found) {
			get_random_bytes(&r, sizeof(r));
			const char *newname = random_names[r % NUM_NAMES];
			kfree(found->name);
			found->name = kstrdup(newname, GFP_KERNEL);
			if (!found->name)
				found->name = kstrdup("", GFP_KERNEL);

			pr_info("[W %d] modified id=%ld newname=%s — exiting\n",
				id_local, found->id, found->name);

			up(&rw_mutex);
			wake_up_all(&wq); /* notify readers */
			break; /* === exit after first success === */
		}

		up(&rw_mutex);

		pr_info("[W %d] id=%ld not found — retrying\n", id_local, idx);
		wait_event_interruptible_timeout(
			wq, kthread_should_stop(), msecs_to_jiffies(200));

		msleep(150);
		cond_resched();
	}

	pr_info("rw_db: writer %d finished\n", id_local);
	return 0;
}

/* === Module init/exit === */
static int __init rw_db_init(void)
{
	int i;
	pr_info("rw_db: init (readers=%d writers=%d count=%d)\n",
		readers, writers, initial_count);

	INIT_LIST_HEAD(&g_records);
	sema_init(&rmutex, 1);
	sema_init(&rw_mutex, 1);
	init_waitqueue_head(&wq);

	init_db();

	reader_tasks = kcalloc(readers, sizeof(struct task_struct *), GFP_KERNEL);
	writer_tasks = kcalloc(writers, sizeof(struct task_struct *), GFP_KERNEL);
	if (!reader_tasks || !writer_tasks)
		goto err_alloc;

	for (i = 0; i < readers; i++) {
		char name[32];
		snprintf(name, sizeof(name), "rw_reader_%d", i + 1);
		reader_tasks[i] = kthread_run(reader_fn, (void *)(long)(i + 1), "%s", name);
		if (IS_ERR(reader_tasks[i]))
			reader_tasks[i] = NULL;
	}

	for (i = 0; i < writers; i++) {
		char name[32];
		snprintf(name, sizeof(name), "rw_writer_%d", i + 1);
		writer_tasks[i] = kthread_run(writer_fn, (void *)(long)(i + 1), "%s", name);
		if (IS_ERR(writer_tasks[i]))
			writer_tasks[i] = NULL;
	}

	return 0;

err_alloc:
	kfree(reader_tasks);
	kfree(writer_tasks);
	free_db();
	return -ENOMEM;
}

static void __exit rw_db_exit(void)
{
	int i;
	pr_info("rw_db: exit — stopping threads\n");
	reader_stop = true;

	wake_up_all(&wq);
	msleep(100);
	
	/* if (reader_tasks) {
		for (i = 0; i < readers; i++)
			if (reader_tasks[i])
				kthread_stop(reader_tasks[i]);
		kfree(reader_tasks);
	}

	if (writer_tasks) {
		for (i = 0; i < writers; i++)
			if (writer_tasks[i])
				kthread_stop(writer_tasks[i]);
		kfree(writer_tasks);
	}

	wake_up_all(&wq);
	free_db();
	pr_info("rw_db: module unloaded\n");
	*/
	if (reader_tasks) {
		for (i = 0; i < readers; i++) {
			if (reader_tasks[i]) {
				pr_info("rw_db: stopping reader %d\n", i + 1);
				kthread_stop(reader_tasks[i]);
				reader_tasks[i] = NULL;
			}
		}
		kfree(reader_tasks);
		reader_tasks = NULL;
	}

	if (writer_tasks) {
		for (i = 0; i < writers; i++) {
			if (writer_tasks[i]) {
				pr_info("rw_db: stopping writer %d\n", i + 1);
				kthread_stop(writer_tasks[i]);
				writer_tasks[i] = NULL;
			}
		}
		kfree(writer_tasks);
		writer_tasks = NULL;
	}

	free_db();

	pr_info("rw_db: module successfully unloaded\n");
}

module_init(rw_db_init);
module_exit(rw_db_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stv");
MODULE_DESCRIPTION("Reader-Writer DB kernel module example");

