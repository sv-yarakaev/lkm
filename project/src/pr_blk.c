#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/blkdev.h>
#include <linux/blk-mq.h>
#include <linux/hdreg.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/version.h>

/* Проверка версии ядра */
#if LINUX_VERSION_CODE < KERNEL_VERSION(6,1,0) || LINUX_VERSION_CODE > KERNEL_VERSION(6,1,255)
#warning "Этот модуль предназначен для ядра 6.1.x (например, 6.1.130)"
#endif

#define DEVICE_NAME "testblk"
#define DISK_SIZE_KiB 448
#define DISK_SIZE_BYTES (DISK_SIZE_KiB * 1024)  // 448 KiB
#define CAPACITY_SECTORS (DISK_SIZE_BYTES / SECTOR_SIZE)  // 896 секторов

static int dev_major;
static struct block_dev *block_device;

struct block_dev {
    sector_t capacity;
    u8 *data;
    struct blk_mq_tag_set tag_set;
    struct gendisk *gdisk;
};





static int __init mblock_driver_init(void)
{
    
    return 0;


}

static void __exit mblock_driver_exit(void)
{
    
}

module_init(mblock_driver_init);
module_exit(mblock_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stv");
MODULE_DESCRIPTION("RAM-backed block device for kernel 6.1.x");
MODULE_VERSION("1.1");