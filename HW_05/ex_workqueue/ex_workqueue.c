#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/workqueue.h>

static void my_work_func(struct work_struct *work) {
  pr_info("Work function executed\n");
  msleep(100);
  pr_info("Work function completed after sleep\n");
}

static void my_delayed_work_func(struct work_struct *work) {
  pr_info("Delayed work function executed\n");
}

static struct workqueue_struct *my_wq;
static struct work_struct my_work;
static DECLARE_DELAYED_WORK(my_delayed_work, my_delayed_work_func);

static int __init ex_workqueue_init(void) {
  pr_info("Module init\n");

  // wq with WQ_MEM_RECLAIM
  my_wq = alloc_workqueue("ex_wq", WQ_UNBOUND | WQ_MEM_RECLAIM, 1);
  if (!my_wq) {
    pr_err("Failed to allocate workqueue\n");
    return -ENOMEM;
  }
  pr_info("Workqueue allocated with WQ_MEM_RECLAIM\n");

  INIT_WORK(&my_work, my_work_func);
  pr_info("Work initialized\n");

  queue_work(my_wq, &my_work);
  pr_info("Work queued on custom wq\n");

  schedule_work(&my_work);
  pr_info("Work scheduled on system wq\n");

  schedule_delayed_work(&my_delayed_work, msecs_to_jiffies(500));
  pr_info("Delayed work scheduled on system wq\n");

  mod_delayed_work(system_wq, &my_delayed_work, msecs_to_jiffies(1000));
  pr_info("Modified delayed work delay to 1000ms\n");

  queue_work_on(0, my_wq, &my_work);
  pr_info("Work queued on specific CPU 0\n");

  // flush_work - sync cur work
  flush_work(&my_work);
  pr_info(" Flushed specific work\n");

  struct workqueue_struct *single_wq =
      create_singlethread_workqueue("ex_single_wq");
  if (single_wq) {
    queue_work(single_wq, &my_work);
    pr_info("Work queued on single-threaded wq\n");
    destroy_workqueue(single_wq);
    pr_info("Single-threaded wq destroyed\n");
  }

  cancel_delayed_work(&my_delayed_work);
  pr_info("Attempted to cancel delayed work\n");

  pr_info("Simulating IRQ deferred work\n");
  queue_work(my_wq, &my_work);

  cancel_work_sync(&my_work);
  pr_info("Synchronous cancel of work\n");

  return 0;
}

static void __exit ex_workqueue_exit(void) {
  cancel_delayed_work_sync(&my_delayed_work);
  cancel_work_sync(&my_work);
  flush_workqueue(my_wq);
  destroy_workqueue(my_wq);
  pr_info("Workqueue flushed and destroyed\n");
  pr_info("Module exiting\n");
}

module_init(ex_workqueue_init);
module_exit(ex_workqueue_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stv");
MODULE_DESCRIPTION("HW-05 wq");
