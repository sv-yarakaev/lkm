#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/version.h>

static struct timer_list my_timer;
static unsigned int counter;
// static unsigned int timeout_ms;
static unsigned long interval_jiff;

#define TIMER_INTERVAL_SEC 30U /* период таймера, секунды  */
#define TIMER_STOP_SEC 300U    /* сколько секунд печатаем */

static void my_timer_callback(struct timer_list *timer) {
    static unsigned long start_jiffies;
    unsigned long current_jiffies = jiffies;

    if (start_jiffies == 0) {
        start_jiffies = current_jiffies;
    }

    unsigned long end_jiffies = start_jiffies + secs_to_jiffies(TIMER_STOP_SEC);

    // Проверяем, не достигли ли времени остановки
    if (time_before(current_jiffies, end_jiffies)) {
        counter++;
        unsigned int elapsed_sec = jiffies_to_msecs(current_jiffies - start_jiffies) / 1000;
        pr_info("min=%u: Hello, timer!\n", elapsed_sec / 60);
        mod_timer(timer, jiffies + interval_jiff);
    } else {
        pr_info("Timer stopped after %u seconds\n", TIMER_STOP_SEC);
    }
}

static int __init ex_timer_init(void) {
  interval_jiff = secs_to_jiffies(TIMER_INTERVAL_SEC);

  pr_info("Module loaded.\n");

  timer_setup(&my_timer, my_timer_callback, 0);
  mod_timer(&my_timer, jiffies + interval_jiff);
  return 0;
}

static void __exit ex_timer_exit(void) {
  /*Внезапно с версии 6.15 del_timer_sync отсутвует. Так как по умолчанию я
   * использую 6.15.8-200 то это прям вынужденно   */
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 15, 0)
  del_timer_sync(&my_timer);
#else
  timer_shutdown_sync(&my_timer);
#endif
  pr_info("Module is unloaded\n");
}

module_init(ex_timer_init);
module_exit(ex_timer_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("HW 06 kernel timer example");
