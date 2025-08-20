#include "linux/sched.h"
#include <linux/module.h>


struct queue_item {
    void *data;
    struct list_head list;
};

// Структура для самой очереди
struct queue {
    struct list_head head;  // Голова списка
    //spinlock_t lock;        // Спинлок для синхронизации
    unsigned int size;      // Текущий размер очереди
};

// Инициализация очереди
static inline void queue_init(struct queue *q)
{
    INIT_LIST_HEAD(&q->head);
    //spin_lock_init(&q->lock);
    q->size = 0;
}

// Создание новой очереди
struct queue *queue_create(void)
{
    struct queue *q = kmalloc(sizeof(*q), GFP_KERNEL);
    if (!q)
        return NULL;
    queue_init(q);
    return q;
}

// Добавление элемента в конец очереди (enqueue)
int queue_enqueue(struct queue *q, void* data)
{
    struct queue_item *item;
    unsigned long flags;

    if (!q)
        return -EINVAL;

    item = kmalloc(sizeof(*item), GFP_KERNEL);
    if (!item)
        return -ENOMEM;

    item->data = data;
    INIT_LIST_HEAD(&item->list);

    //spin_lock_irqsave(&q->lock, flags);
    list_add_tail(&item->list, &q->head);
    q->size++;
    //spin_unlock_irqrestore(&q->lock, flags);

    return 0;
}

// Удаление и возврат первого элемента из очереди (dequeue)
int queue_dequeue(struct queue *q, void *data)
{
    struct queue_item *item;
    unsigned long flags;

    if (!q || !data)
        return -EINVAL;

    //spin_lock_irqsave(&q->lock, flags);
    if (list_empty(&q->head)) {
        //spin_unlock_irqrestore(&q->lock, flags);
        return -ENOENT; // Очередь пуста
    }

    item = list_first_entry(&q->head, struct queue_item, list);
    data = item->data;
    list_del(&item->list);
    q->size--;
    //spin_unlock_irqrestore(&q->lock, flags);

    kfree(item);
    return 0;
}




static int main_body(void) {
    struct task_struct* cur_node;


    return 0;
}













static int __init init_queue(void) { return 0;}
static void __exit exit_queue(void) {}



MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("HW-03 example queue");
module_init(init_queue);
module_exit(exit_queue);