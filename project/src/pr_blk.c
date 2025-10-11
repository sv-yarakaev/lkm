#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include "asm-generic/int-ll64.h"
#include "linux/radix-tree.h"
#include "linux/spinlock_types.h"


#include <linux/module.h>
#include <linux/blkdev.h>
#include <linux/blk-mq.h>
#include <linux/hdreg.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/version.h>
#include <linux/blk_types.h>
#include <linux/list.h>
#include "asm-generic/errno-base.h"
#include "linux/err.h"
#include "linux/gfp_types.h"
#include "linux/mutex.h"
#include "linux/spinlock.h"

/* Проверка версии ядра */
#if LINUX_VERSION_CODE < KERNEL_VERSION(6,1,0) || LINUX_VERSION_CODE > KERNEL_VERSION(6,1,255)
#warning "Этот модуль предназначен для ядра 6.1.x (например, 6.1.130)"
#endif

#define DEVICE_NAME "testblk"
#define DISK_SIZE_KiB 448
#define DISK_SIZE_BYTES (DISK_SIZE_KiB * 1024)  // 448 KiB
#define CAPACITY_SECTORS (DISK_SIZE_BYTES / SECTOR_SIZE)  // 896 секторов
#define BDISKMAJOR		1

static int dev_major;
static struct block_dev *block_device;
static struct dentry *bd_debugfs_dir;

struct block_dev {
    int bd_number;
    struct gendisk *gdisk;

    struct list_head bd_list;
    spinlock_t bd_lock;
    struct radix_tree_root bd_pages;
    u64 bd_nr_pages;
};

/*
 * Найти или выделить block_dev (вставить в глобальный список)
 * Возвращает указатель или ERR_PTR(-errno).  если
 * устройство уже существует, функция возвращает ERR_PTR(-EEXIST), а не
 * сам указатель. Это поведение используется вызывающими функциями.
 */
static LIST_HEAD(bd_devices);
static DEFINE_MUTEX(bd_devices_mutex);


static struct block_dev* bd_alloc_dev(int n) {
    struct block_dev *bd;

    mutex_lock(&bd_devices_mutex);
    list_for_each_entry(bd, &bd_devices, bd_list) {
        if (bd->bd_number == n) {
            mutex_unlock(&bd_devices_mutex);
            /*  Возвращаем ERR_PTR(-EEXIST). Это идиома ядра: вместо NULL возвращается указатель, 
            в котором закодирован отрицательный код ошибки.
             -EEXIST означает «устройство уже существует». 
             Использование ERR_PTR позволяет вызывающей функции легко отличить ошибку от валидного указателя 
             (через IS_ERR() и получить код через PTR_ERR()). */
            return ERR_PTR(-EEXIST);
        }
    }

    bd = kzalloc(sizeof(*bd), GFP_KERNEL); // kzalloc — выделение + обнуление.
    if (!bd) {
        mutex_unlock(&bd_devices_mutex);
        return ERR_PTR(-ENOMEM);
    }
    bd->bd_number = n;
    
    list_add_tail(&bd->bd_list, &bd_devices);
    mutex_unlock(&bd_devices_mutex);
    /* Возвращаем указатель на выделенный brd_device. 
    Вызывающая сторона должна проверить, что возвращён не ERR_PTR (через IS_ERR()), 
    и затем продолжить полную инициализацию устройства.*/
    return bd;
}

/*
 *   bd_alloc -- инициализация gendisk и регистрация устройства
 *   создаёт gendisk, настраивает имя, размеры, флаги очереди и регистрирует
 *   диск через add_disk().
 */
static int bd_alloc(int n) {
    struct block_dev *bd;
    struct gendisk *disk; 
    char buffer[32]; //DISK_NAME_LEN

    bd = bd_alloc_dev(n);
    if (IS_ERR(bd)) {
        return PTR_ERR(bd);
    }

    spin_lock_init(&bd->bd_lock);
    INIT_RADIX_TREE(&bd->bd_pages, GFP_ATOMIC);

    snprintf(buffer, 32, "bram%d", n);




    return 0;
} 
static void bd_probe(dev_t dev){

}


static int __init mblock_driver_init(void)
{
    int error = 0;
    int major = register_blkdev(BDISKMAJOR, DEVICE_NAME);
    if (major < 0) {
        error = -EIO;
        return major;
    }
    bd_alloc(0);

    return 0;


}

static void __exit mblock_driver_exit(void)
{
    unregister_blkdev(BDISKMAJOR, DEVICE_NAME);
}

module_init(mblock_driver_init);
module_exit(mblock_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stv");
MODULE_DESCRIPTION("RAM-backed block device for kernel 6.1.x");
MODULE_VERSION("1.1");