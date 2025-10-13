// bd_sysfs.h
#ifndef _BD_SYSFS_H
#define _BD_SYSFS_H

#include <linux/blkdev.h>
#include <linux/device.h>
// Объявления функций для регистрации/удаления атрибутов
int bd_sysfs_init(struct gendisk *disk);
void bd_sysfs_exit(struct gendisk *disk);

#endif