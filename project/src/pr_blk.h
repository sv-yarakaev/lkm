#ifndef _BD_H
#define _BD_H
#include <linux/backing-dev.h>
#include <linux/bio.h>
#include <linux/blkdev.h>
#include <linux/debugfs.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/highmem.h>
#include <linux/init.h>
#include <linux/initrd.h>
#include <linux/ioctl.h>
#include <linux/list.h>
#include <linux/major.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/mutex.h>
#include <linux/pagemap.h>
#include <linux/proc_fs.h>
#include <linux/radix-tree.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/version.h>

struct ram_stat {
  /* Счётчики операций */
  atomic64_t reads;           // количество операций чтения
  atomic64_t writes;          // количество операций записи
  atomic64_t read_sectors;    // секторов прочитано (1 сектор = 512 байт)
  atomic64_t written_sectors; // секторов записано

  /* Ошибки */
  atomic64_t errors; // ошибки (например, out-of-range)

  /* Опционально: задержки */
#ifdef BRD_COLLECT_LATENCY
  atomic64_t total_read_time_ns;  // суммарное время чтения
  atomic64_t total_write_time_ns; // суммарное время записи
  atomic64_t max_read_time_ns;
  atomic64_t max_write_time_ns;
#endif
  /* Дополнительная аналитика */
  atomic64_t multi_segment_bios; // bio с >1 сегментом
  atomic64_t small_requests;     // запросы < 4 КБ (т.е. < 8 секторов)

  /* Для расчёта среднего размера запроса */
  atomic64_t total_requests; // reads + writes
  // Средний размер = (read_sectors + written_sectors) / total_requests

  /* Время жизни диска */
  ktime_t creation_time; // не атомарное — устанавливается один раз
  ktime_t deletion_time; // можно не хранить, но полезно для лога

  /* Статика (можно без атомарности) */
  unsigned long size_pages; // сколько страниц выделено
  unsigned long size_bytes; // общий размер в байтах
};

struct block_dev {
  int bd_number;
  struct gendisk *gdisk;

  struct list_head bd_list;
  spinlock_t bd_lock;
  struct radix_tree_root bd_pages;
  u64 bd_nr_pages;
  struct ram_stat stat;
};

#endif