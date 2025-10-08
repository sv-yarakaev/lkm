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

static int blockdev_open(struct block_device *bdev, fmode_t mode)
{
    pr_info("Device %s opened\n", bdev->bd_disk->disk_name);
    return 0;
}

static void blockdev_release(struct gendisk *gd, fmode_t mode)
{
    pr_info("Device %s closed\n", gd->disk_name);
}

static const struct block_device_operations blockdev_ops = {
    .owner = THIS_MODULE,
    .open = blockdev_open,
    .release = blockdev_release,
};

static blk_status_t queue_rq(struct blk_mq_hw_ctx *hctx, const struct blk_mq_queue_data *bd)
{
    struct request *rq = bd->rq;
    struct block_dev *dev = rq->q->queuedata;
    blk_status_t status = BLK_STS_OK;

    blk_mq_start_request(rq);

    if (blk_rq_is_passthrough(rq)) {
        status = BLK_STS_IOERR;
        goto end_request;
    }

    loff_t pos = blk_rq_pos(rq) * SECTOR_SIZE;
    loff_t dev_size = (loff_t)dev->capacity * SECTOR_SIZE;

    struct bio *bio;
    __rq_for_each_bio(bio, rq) {
        struct bio_vec bvec;
        struct bvec_iter iter;

        bio_for_each_segment(bvec, bio, iter) {
            unsigned int b_len = bvec.bv_len;
            void *b_buf = page_address(bvec.bv_page) + bvec.bv_offset;

            if (pos >= dev_size) {
                // За пределами устройства — прерываем обработку
                goto end_request;
            }
            if (pos + b_len > dev_size) {
                b_len = dev_size - pos;
            }

            if (rq_data_dir(rq) == WRITE) {
                memcpy(dev->data + pos, b_buf, b_len);
            } else {
                memcpy(b_buf, dev->data + pos, b_len);
            }

            pos += b_len;
        }
    }

end_request:
    blk_mq_end_request(rq, status);
    return BLK_STS_OK;
}
static const struct blk_mq_ops mq_ops = {
    .queue_rq = queue_rq,
};

static int __init myblock_driver_init(void)
{
    int err;

    dev_major = register_blkdev(0, DEVICE_NAME);
    if (dev_major < 0) {
        pr_err("Failed to register block device\n");
        return dev_major;
    }

    block_device = kzalloc(sizeof(*block_device), GFP_KERNEL);
    if (!block_device) {
        err = -ENOMEM;
        goto err_unregister;
    }

    block_device->capacity = CAPACITY_SECTORS;
    block_device->data = vzalloc(DISK_SIZE_BYTES);
    if (!block_device->data) {
        pr_err("Failed to allocate data buffer\n");
        err = -ENOMEM;
        goto err_free_dev;
    }

    /* Создаём gendisk — очередь создаётся автоматически */
    block_device->gdisk = blk_alloc_disk(NUMA_NO_NODE);
    if (!block_device->gdisk) {
        pr_err("Failed to allocate gendisk\n");
        err = -ENOMEM;
        goto err_free_data;
    }

    block_device->gdisk->major = dev_major;
    block_device->gdisk->first_minor = 0;
    block_device->gdisk->fops = &blockdev_ops;
    block_device->gdisk->private_data = block_device;
    snprintf(block_device->gdisk->disk_name, sizeof(block_device->gdisk->disk_name),
             "%s", DEVICE_NAME);
    block_device->gdisk->flags |= GENHD_FL_NO_PART;
    set_capacity(block_device->gdisk, block_device->capacity);

    struct request_queue *q = block_device->gdisk->queue;
    q->queuedata = block_device;

    /* Настройка размера сектора */
    blk_queue_logical_block_size(q, SECTOR_SIZE);

    /* Инициализация tag set */
    block_device->tag_set.ops = &mq_ops;
    block_device->tag_set.nr_hw_queues = 1;
    block_device->tag_set.queue_depth = 128;
    block_device->tag_set.numa_node = NUMA_NO_NODE;
    block_device->tag_set.flags = BLK_MQ_F_SHOULD_MERGE;

    err = blk_mq_alloc_tag_set(&block_device->tag_set);
    if (err) {
        pr_err("Failed to alloc tag set: %d\n", err);
        goto err_put_disk;
    }

    /* Привязываем tag set к уже выделенной очереди */
    err = blk_mq_init_allocated_queue(&block_device->tag_set, q);
    if (err) {
        pr_err("Failed to init allocated queue: %d\n", err);
        goto err_free_tag_set;
    }

    err = add_disk(block_device->gdisk);
    if (err) {
        pr_err("Failed to add disk: %d\n", err);
        goto err_free_tag_set;
    }

    pr_info("Block device %s added, major %d, capacity %llu sectors (%u KiB)\n",
            block_device->gdisk->disk_name, dev_major,
            (unsigned long long)block_device->capacity, DISK_SIZE_KiB);

    return 0;


err_free_tag_set:
    blk_mq_free_tag_set(&block_device->tag_set);
err_put_disk:
    put_disk(block_device->gdisk);
err_free_data:
    vfree(block_device->data);
err_free_dev:
    kfree(block_device);
err_unregister:
    unregister_blkdev(dev_major, DEVICE_NAME);
    return err;
}

static void __exit myblock_driver_exit(void)
{
    del_gendisk(block_device->gdisk);
    blk_mq_free_tag_set(&block_device->tag_set);
    put_disk(block_device->gdisk);  // Это освободит очередь автоматически
    vfree(block_device->data);
    kfree(block_device);
    unregister_blkdev(dev_major, DEVICE_NAME);
    pr_info("Block device %s removed\n", DEVICE_NAME);
}

module_init(myblock_driver_init);
module_exit(myblock_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stv");
MODULE_DESCRIPTION("RAM-backed block device for kernel 6.1.x");
MODULE_VERSION("1.0");