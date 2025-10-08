#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/init.h>
#include <linux/blkdev.h> // сюда перенесно gendisk с версии 6
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/blk_types.h>
#include <linux/list.h>
#include <linux/blk-mq.h>
#include <linux/hdreg.h>
#include <linux/buffer_head.h>
#include <linux/vmalloc.h>
/*
1.Регистрацию major номера.
2. Инициализацию очереди запросов с blk-mq (глубина 128, с поддержкой слияния запросов).
3. Создание gendisk с ёмкостью 448 KiB (896 секторов по 512 байт).
4. Обработку запросов чтения/записи через memcpy (эмуляция в памяти).
5. Базовые операции (open, release, ioctl).
6. Модуль init/exit с очисткой ресурсов.
*/




#define DEVICE_NAME "testblk"
#define CAPACITY_SECTORS (112 * PAGE_SIZE / SECTOR_SIZE)  // Примерно 448 KiB

static int dev_major = 0;

struct block_dev {
    sector_t capacity;
    u8 *data;  // Буфер данных для эмуляции диска
    struct blk_mq_tag_set tag_set;
    struct request_queue *queue;
    struct gendisk *gdisk;
};

static struct block_dev *block_device = NULL;

static int blockdev_open(struct block_device *bdev, fmode_t mode) {
    pr_info("Device %s opened\n", bdev->bd_disk->disk_name);
    return 0;
}

static void blockdev_release(struct gendisk *gd, fmode_t mode) {
    pr_info("Device %s closed\n", gd->disk_name);
}

static int blockdev_ioctl(struct block_device *bdev, fmode_t mode, unsigned cmd, unsigned long arg) {
    return -ENOTTY;  // ioctl не поддерживается
}

static const struct block_device_operations blockdev_ops = {
    .owner = THIS_MODULE,
    .open = blockdev_open,
    .release = blockdev_release,
    .ioctl = blockdev_ioctl
};

static int do_request(struct request *rq, unsigned int *nr_bytes) {
    int ret = 0;
    struct bio_vec bvec;
    struct req_iterator iter;
    struct block_dev *dev = rq->q->queuedata;
    loff_t pos = blk_rq_pos(rq) * SECTOR_SIZE;  // Позиция в байтах
    loff_t dev_size = (loff_t)(dev->capacity * SECTOR_SIZE);

    pr_debug("Request: start sector %lld, pos %lld, dev_size %lld\n",
             (long long)blk_rq_pos(rq), pos, dev_size);

    rq_for_each_segment(bvec, rq, iter) {
        unsigned long b_len = bvec.bv_len;
        void *b_buf = page_address(bvec.bv_page) + bvec.bv_offset;

        // Проверка на выход за пределы устройства
        if ((pos + b_len) > dev_size) {
            b_len = (unsigned long)(dev_size - pos);
        }

        if (rq_data_dir(rq) == WRITE) {
            memcpy(dev->data + pos, b_buf, b_len);  // Запись в буфер
        } else {
            memcpy(b_buf, dev->data + pos, b_len);  // Чтение из буфера
        }

        pos += b_len;
        *nr_bytes += b_len;
    }

    return ret;
}

static blk_status_t queue_rq(struct blk_mq_hw_ctx *hctx, const struct blk_mq_queue_data *bd) {
    unsigned int nr_bytes = 0;
    blk_status_t status = BLK_STS_OK;
    struct request *rq = bd->rq;

    blk_mq_start_request(rq);

    if (do_request(rq, &nr_bytes) != 0) {
        status = BLK_STS_IOERR;
    }

    if (blk_update_request(rq, status, nr_bytes)) {
        pr_err("Failed to update request\n");
        BUG();
    }

    __blk_mq_end_request(rq, status);

    return status;
}

static const struct blk_mq_ops mq_ops = {
    .queue_rq = queue_rq,
};

static int __init myblock_driver_init(void) {
    int err = 0;

    dev_major = register_blkdev(0, DEVICE_NAME);  // Динамический major
    if (dev_major < 0) {
        pr_err("Failed to register block device\n");
        return dev_major;
    }

    block_device = kmalloc(sizeof(struct block_dev), GFP_KERNEL);
    if (!block_device) {
        err = -ENOMEM;
        goto err_unregister;
    }

    block_device->capacity = CAPACITY_SECTORS;
    block_device->data = kmalloc(block_device->capacity * SECTOR_SIZE, GFP_KERNEL);
    if (!block_device->data) {
        err = -ENOMEM;
        goto err_free_dev;
    }
    memset(block_device->data, 0, block_device->capacity * SECTOR_SIZE);  // Инициализация нулями

    // Инициализация tag_set
    block_device->tag_set.ops = &mq_ops;
    block_device->tag_set.nr_hw_queues = 1;
    block_device->tag_set.queue_depth = 128;
    block_device->tag_set.numa_node = NUMA_NO_NODE;
    block_device->tag_set.flags = BLK_MQ_F_SHOULD_MERGE;

    err = blk_mq_alloc_tag_set(&block_device->tag_set);
    if (err) {
        pr_err("Failed to alloc tag set\n");
        goto err_free_data;
    }

    // Инициализация очереди
    block_device->queue = blk_mq_init_queue(&block_device->tag_set);
    if (IS_ERR(block_device->queue)) {
        err = PTR_ERR(block_device->queue);
        pr_err("Failed to init queue: %d\n", err);
        goto err_free_tag_set;
    }

    block_device->queue->queuedata = block_device;
    blk_queue_logical_block_size(block_device->queue, SECTOR_SIZE);

    // Инициализация gendisk с использованием blk_alloc_disk
    block_device->gdisk = blk_alloc_disk(1);
    if (!block_device->gdisk) {
        err = -ENOMEM;
        pr_err("Failed to alloc disk\n");
        goto err_cleanup_queue;
    }

    block_device->gdisk->major = dev_major;
    block_device->gdisk->first_minor = 0;
    block_device->gdisk->fops = &blockdev_ops;
    block_device->gdisk->private_data = block_device;
    snprintf(block_device->gdisk->disk_name, sizeof(block_device->gdisk->disk_name), "%s", DEVICE_NAME);
    block_device->gdisk->flags |= GENHD_FL_NO_PART;  // Отключение партиций

    set_capacity(block_device->gdisk, block_device->capacity);

    err = add_disk(block_device->gdisk);
    if (err) {
        pr_err("Failed to add disk: %d\n", err);
        goto err_cleanup_disk;
    }

    pr_info("Block device %s added, major %d, capacity %lld sectors\n",
            block_device->gdisk->disk_name, dev_major, (long long)block_device->capacity);

    return 0;

err_cleanup_disk:
    // blk_cleanup_disk(block_device->gdisk);  
	// Очищает и gendisk, и queue
	put_disk(block_device->gdisk);
    goto err_free_tag_set;
err_cleanup_queue:
    // blk_cleanup_queue не нужен, так как очередь очищается через blk_cleanup_disk
err_free_tag_set:
    blk_mq_free_tag_set(&block_device->tag_set);
err_free_data:
    kfree(block_device->data);
err_free_dev:
    kfree(block_device);
err_unregister:
    unregister_blkdev(dev_major, DEVICE_NAME);
    return err;
}

static void __exit myblock_driver_exit(void) {
    if (block_device->gdisk) {
        del_gendisk(block_device->gdisk);
        put_disk(block_device->gdisk);  // Очищает gendisk и queue
    }
    blk_mq_free_tag_set(&block_device->tag_set);
    if (block_device->data) {
        kfree(block_device->data);
    }
    kfree(block_device);
    unregister_blkdev(dev_major, DEVICE_NAME);
    pr_info("Block device %s removed\n", DEVICE_NAME);
}

module_init(myblock_driver_init);
module_exit(myblock_driver_exit);




MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stv");
MODULE_DESCRIPTION("Block device for test in the RAM");
