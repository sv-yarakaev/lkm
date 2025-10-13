// bd_sysfs.c
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/ktime.h>
#include "pr_blk.h"
#include "bd_sysfs.h"

static inline struct block_dev *dev_to_bd(struct device *dev)
{
    struct gendisk *disk = dev_to_disk(dev);
    return disk ? disk->private_data : NULL;
}

/* --- Все show/store функции --- */

static ssize_t reads_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct block_dev *bd = dev_to_bd(dev);
    return bd ? sprintf(buf, "%llu\n", atomic64_read(&bd->stat.reads)) : -ENODEV;
}
static DEVICE_ATTR_RO(reads);



static ssize_t writes_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct block_dev *bd = dev_to_bd(dev);
    return sprintf(buf, "%llu\n", atomic64_read(&bd->stat.writes));
}
static DEVICE_ATTR_RO(writes);

static ssize_t read_sectors_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct block_dev *bd = dev_to_bd(dev);
    return sprintf(buf, "%llu\n", atomic64_read(&bd->stat.read_sectors));
}
static DEVICE_ATTR_RO(read_sectors);

static ssize_t written_sectors_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct block_dev *bd = dev_to_bd(dev);
    return sprintf(buf, "%llu\n", atomic64_read(&bd->stat.written_sectors));
}
static DEVICE_ATTR_RO(written_sectors);

static ssize_t errors_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct block_dev *bd = dev_to_bd(dev);
    return sprintf(buf, "%llu\n", atomic64_read(&bd->stat.errors));
}
static DEVICE_ATTR_RO(errors);

static ssize_t multi_segment_bios_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct block_dev *bd = dev_to_bd(dev);
    return sprintf(buf, "%llu\n", atomic64_read(&bd->stat.multi_segment_bios));
}
static DEVICE_ATTR_RO(multi_segment_bios);

static ssize_t small_requests_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct block_dev *bd = dev_to_bd(dev);
    return sprintf(buf, "%llu\n", atomic64_read(&bd->stat.small_requests));
}
static DEVICE_ATTR_RO(small_requests);

static ssize_t size_bytes_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct block_dev *bd = dev_to_bd(dev);
    return sprintf(buf, "%lu\n", bd->stat.size_bytes);
}
static DEVICE_ATTR_RO(size_bytes);

static ssize_t uptime_ms_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct block_dev *bd = dev_to_bd(dev);
    s64 ms = ktime_to_ms(ktime_sub(ktime_get(), bd->stat.creation_time));
    return sprintf(buf, "%lld\n", ms);
}
static DEVICE_ATTR_RO(uptime_ms);


static ssize_t reset_stats_store(struct device *dev, struct device_attribute *attr,
                                 const char *buf, size_t count)
{
    struct block_dev *bd = dev_to_bd(dev);

    atomic64_set(&bd->stat.reads, 0);
    atomic64_set(&bd->stat.writes, 0);
    atomic64_set(&bd->stat.read_sectors, 0);
    atomic64_set(&bd->stat.written_sectors, 0);
    atomic64_set(&bd->stat.errors, 0);
    atomic64_set(&bd->stat.multi_segment_bios, 0);
    atomic64_set(&bd->stat.small_requests, 0);
    atomic64_set(&bd->stat.total_requests, 0);

    return count;
}
static DEVICE_ATTR_WO(reset_stats);

/* --- Группа атрибутов --- */

static struct attribute *bd_stat_attrs[] = {
    &dev_attr_reads.attr,
    &dev_attr_writes.attr,
    &dev_attr_read_sectors.attr,
    &dev_attr_written_sectors.attr,
    &dev_attr_errors.attr,
    &dev_attr_multi_segment_bios.attr,
    &dev_attr_small_requests.attr,
    &dev_attr_size_bytes.attr,
    &dev_attr_uptime_ms.attr,
    &dev_attr_reset_stats.attr,
    NULL,
};

static const struct attribute_group bd_stat_group = {
    .name = "ramstat",
    .attrs = bd_stat_attrs,
};

 const struct attribute_group *bd_dev_groups[] = {
    &bd_stat_group,
    NULL
};

/* --- Публичные функции --- */

int bd_sysfs_init(struct gendisk *disk)
{
    if (!disk)
        return -EINVAL;
    return sysfs_create_group(&disk_to_dev(disk)->kobj, &bd_stat_group);
}

void bd_sysfs_exit(struct gendisk *disk)
{
    // Ничего не нужно делать — ядро само освободит атрибуты при удалении диска
}

EXPORT_SYMBOL(bd_sysfs_init);