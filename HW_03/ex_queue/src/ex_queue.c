#include "linux/gfp_types.h"
#include "linux/list.h"
#include "linux/slab.h"
#include <linux/module.h>
#include <linux/kernel.h>

#include "random_values.h"
/*
    Интерфейс очереди. Основные опрации.
Поддерживаемые функции:
    * enqueue  добавить элемент в конец очереди
    * dequeue удаляет элемент из начала очереди
    * peek  возвращает элемент из н ачала очереди без удаления
    * isEmpty  проверка на путоту
    * size  размер
*/

struct queue_entry {
    void *data;
    struct list_head list;
};

struct queue {
    struct list_head head;
};

static inline size_t queue_size(struct queue *q);

static inline void queue_init(struct queue *q) {
    INIT_LIST_HEAD(&q->head);
}


static inline bool queue_is_empty(struct queue *q) {
    return list_empty(&q->head);
}

static inline void queue_enqueue(struct queue *q, void *data) {
    struct queue_entry *entry;
    entry = kmalloc(sizeof(*entry), GFP_KERNEL);
    if (!entry) return;
    entry->data = data;
    
    INIT_LIST_HEAD(&entry->list);
    list_add_tail(&entry->list, &q->head);
    pr_info("enqueue: pointer data = %p, new size=%zu\n", data, queue_size(q));
    pr_info("enqueue: string data = %s, new size=%zu\n", (char *)data, queue_size(q));
}

static inline struct queue_entry *queue_dequeue(struct queue *q) {
    struct queue_entry* entry;

    if (list_empty(&q->head)) return NULL;

    entry = list_first_entry(&q->head, struct queue_entry, list);
    list_del(&entry->list);

    pr_info("dequeue: pointer data = %p, new size=%zu\n", entry->data, queue_size(q));
    pr_info("dequeue: string data = %s, new size=%zu\n", (char *)entry->data, queue_size(q));
    return entry;
}

static inline struct queue_entry *queue_peek(struct queue *q) {
    if (list_empty(&q->head))
        return NULL;

    return list_first_entry(&q->head, struct queue_entry, list);
}

static inline size_t queue_size(struct queue *q) {
    struct queue_entry *entry;
    size_t count = 0;

    list_for_each_entry(entry, &q->head, list)
        count++;

    return count;
}


static int main_body(void) {
    struct queue q;
    struct queue_entry *e;

    queue_init(&q);

    queue_enqueue(&q, create_random_id(16));
    queue_enqueue(&q, create_random_id(16));
    queue_enqueue(&q, create_random_id(16));
    queue_enqueue(&q, create_random_id(16));
    queue_enqueue(&q, create_random_id(16));

    printk(KERN_INFO "queue size: %zu\n", queue_size(&q));

    e = queue_peek(&q);
    if (e)
        printk(KERN_INFO "peek: %s\n", (char *)e->data);

    while (!queue_is_empty(&q)) {
        e = queue_dequeue(&q);
        printk(KERN_INFO "dequeue: %s\n", (char *)e->data);
        kfree(e); 
    }


    return 0;
}



static int __init init_queue(void) { 
    main_body();    
    return 0;
}
static void __exit exit_queue(void) {}



MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("HW-03 example queue");
module_init(init_queue);
module_exit(exit_queue);