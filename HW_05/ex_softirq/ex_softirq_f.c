/*
 * ex_softirq.c — Пример модуля, использующего механизм, основанный на SOFTIRQ,
 * через irq_work (который использует HI_SOFTIRQ внутри).
 *
 * В ядре >= 6.1 нельзя зарегистрировать свой softirq.
 * Поэтому используем irq_work — легальный API, основанный на softirq.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/irq_work.h>
#include <linux/smp.h>
#include <linux/uaccess.h>

static struct irq_work my_irq_work;

// Обработчик irq_work — выполняется в контексте HI_SOFTIRQ!
static void my_irq_work_handler(struct irq_work *work)
{
    pr_info("ex_softirq: [SOFTIRQ CONTEXT] irq_work обработчик вызван на CPU %d\n",
            smp_processor_id());
}

// Функция записи в /proc — активирует irq_work
static ssize_t proc_write(struct file *file, const char __user *buffer,
                          size_t count, loff_t *pos)
{
    char kbuf[16];
    if (count > sizeof(kbuf) - 1)
        return -EINVAL;

    if (copy_from_user(kbuf, buffer, count))
        return -EFAULT;

    kbuf[count] = '\0';

    if (strncmp(kbuf, "raise", 5) == 0) {
        pr_info("ex_softirq: [PROC] Активируем irq_work (триггерим HI_SOFTIRQ)...\n");
        irq_work_queue(&my_irq_work);
        return count;
    }

    pr_warn("ex_softirq: Используйте: echo raise > /proc/ex_softirq\n");
    return -EINVAL;
}

static int proc_show(struct seq_file *m, void *v)
{
    seq_puts(m, "Write 'raise' to trigger softirq-based work.\n");
    return 0;
}

static int proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_show, NULL);
}

static const struct proc_ops proc_fops = {
    .proc_open = proc_open,
    .proc_read = seq_read,
    .proc_write = proc_write,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

static struct proc_dir_entry *proc_entry;

static int __init ex_softirq_init(void)
{
    pr_info("ex_softirq: Загрузка модуля (ядро >= 6.1.130).\n");

    // Инициализация irq_work
    init_irq_work(&my_irq_work, my_irq_work_handler);
    pr_info("ex_softirq: irq_work инициализирован.\n");

    // Создаём /proc
    proc_entry = proc_create("ex_softirq", 0666, NULL, &proc_fops);
    if (!proc_entry) {
        pr_err("ex_softirq: Не удалось создать /proc/ex_softirq\n");
        return -ENOMEM;
    }

    pr_info("ex_softirq: Модуль готов. Выполните: echo raise > /proc/ex_softirq\n");
    return 0;
}

static void __exit ex_softirq_exit(void)
{
    // Отмена pending irq_work (если есть)
    irq_work_sync(&my_irq_work);

    if (proc_entry) {
        proc_remove(proc_entry);
        pr_info("ex_softirq: /proc/ex_softirq удалён.\n");
    }

    pr_info("ex_softirq: Модуль выгружен.\n");
}
module_init(ex_softirq_init);
module_exit(ex_softirq_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Пример softirq через irq_work (без tasklets)");
MODULE_VERSION("1.1");
