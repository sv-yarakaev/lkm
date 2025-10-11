#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/radix-tree.h>
#include <linux/spinlock_types.h>
#include <linux/debugfs.h>
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
#include <asm-generic/errno-base.h>
#include <linux/gfp_types.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>

/* Проверка версии ядра */
#if LINUX_VERSION_CODE < KERNEL_VERSION(6,1,0) || LINUX_VERSION_CODE > KERNEL_VERSION(6,1,255)
#warning "Этот модуль предназначен для ядра 6.1.x (например, 6.1.130)"
#endif

#define DEVICE_NAME "testblk"
#define DISK_SIZE_KiB 448
#define DISK_SIZE_BYTES (DISK_SIZE_KiB * 1024)  // 448 KiB
#define CAPACITY_SECTORS (DISK_SIZE_BYTES / SECTOR_SIZE)  // 896 секторов
#define BDISKMAJOR		1

static unsigned long rd_size = 4096;

//static int dev_major;
//static struct block_dev *block_device;
static struct dentry *bd_debugfs_dir;
static int max_part = 1;

struct block_dev {
    int bd_number;
    struct gendisk *gdisk;

    struct list_head bd_list;
    spinlock_t bd_lock;
    struct radix_tree_root bd_pages;
    u64 bd_nr_pages;
};
static void bd_submit_bio(struct bio* bio);  // todo убрать в заголовочный файл
static int bd_rw_page(struct block_device *bdev, sector_t sector, struct page *page, enum req_op op); // todo убрать в заголовочный файл

static const struct block_device_operations bd_fops = {
	.owner =		THIS_MODULE,
	.submit_bio =		bd_submit_bio,
	.rw_page =		bd_rw_page,
};

static int bd_rw_page(struct block_device *bdev, sector_t sector, struct page *page, enum req_op op) {
    return 0;
}

/*
 * bd_submit_bio -- точка входа I/O 
 * - вызывается блочным слоем при приходе BIO
 * - перебирает сегменты bio_for_each_segment и вызывает brd_do_bvec
 * - проверки: выравнивание (offset и len должны быть кратны SECTOR_SIZE)
 * - особое поведение: если copy_to_brd_setup вернул -ENOMEM и био с REQ_NOWAIT,
 *   то вызывается bio_wouldblock_error(bio) — правильная реакция для frontends
 *   которые не хотят ждать.
 */
static void bd_submit_bio(struct bio* bio) {

}

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
    int error = -ENOMEM;
    struct block_dev *bd;
    struct gendisk *disk; 
    char buffer[DISK_NAME_LEN]; //DISK_NAME_LEN

    bd = bd_alloc_dev(n);
    if (IS_ERR(bd)) {
        return PTR_ERR(bd);
    }

    spin_lock_init(&bd->bd_lock);
    INIT_RADIX_TREE(&bd->bd_pages, GFP_ATOMIC);

    snprintf(buffer, DISK_NAME_LEN, "bram%d", n);

    if (!IS_ERR_OR_NULL(bd_debugfs_dir)) {
		debugfs_create_u64(buffer, 0444, bd_debugfs_dir,
				&bd->bd_nr_pages);
    }

    disk = bd->gdisk = blk_alloc_disk(NUMA_NO_NODE);

    if (!disk) {
        goto out_free_dev;
    }
    disk->major = BDISKMAJOR;
    disk->first_minor = n * max_part;
    disk->minors = max_part;
    disk->fops = &bd_fops;      /* наши операции bio */
    disk->private_data = bd;  /* быстрый доступ из gendisk -> brd */
    strscpy(disk->disk_name, buffer, DISK_NAME_LEN);
    
    set_capacity(disk, rd_size * 2);  /* rd_size (KB) -> сектора (512B) */

    /*
        * Установка physical block size в PAGE_SIZE: это сделано, чтобы fdisk
        * выравнивал partition'ы по 4K (необходимо, если будет использован
        * direct_access API, возвращающий PFN).
  */
    blk_queue_physical_block_size(disk->queue, PAGE_SIZE);
    /*
        * Характеристики очереди: не ротационное, не добавлять entropy и NOWAIT.
    */
    blk_queue_flag_set(QUEUE_FLAG_NONROT, disk->queue);
    blk_queue_flag_clear(QUEUE_FLAG_ADD_RANDOM, disk->queue);
    blk_queue_flag_set(QUEUE_FLAG_NOWAIT, disk->queue);
    
    error = add_disk(disk);
    if (error)   goto cleanup_disk;

    return 0;



cleanup_disk:
    put_disk(disk);

out_free_dev:
    
    return error;
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