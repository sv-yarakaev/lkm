#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/init.h>
#include <linux/initrd.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/major.h>
#include <linux/blkdev.h>
#include <linux/bio.h>
#include <linux/highmem.h>
#include <linux/mutex.h>
#include <linux/pagemap.h>
#include <linux/radix-tree.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/backing-dev.h>
#include <linux/debugfs.h>
#include <linux/version.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

//#include "pr_blk_proc.h"

static struct proc_dir_entry *proc_dir = NULL;
static struct proc_dir_entry *proc_file = NULL;
#define PROC_DIR_NAME "bramdev"
#define PROC_FILE_NAME "stat"


static int bd_proc_show(struct seq_file *m, void *v) {
    extern int brd_nr;
    extern int brd_size;
    seq_printf(m, "brd_nr: %d\n", brd_nr);
    seq_printf(m, "brd_size: %d MB\n", brd_size);
    return 0;
}
// Функция открытия файла
static int proc_open(struct inode *inode, struct file *file) { 
    return single_open(file, bd_proc_show, NULL); 
}

const struct proc_ops proc_fops = {
    .proc_open = proc_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};




/* Проверка версии ядра */
#if LINUX_VERSION_CODE < KERNEL_VERSION(6,1,0) || LINUX_VERSION_CODE > KERNEL_VERSION(6,1,255)
#warning "Этот модуль предназначен для ядра 6.1.x (например, 6.1.130)"
#endif

#define DEVICE_NAME "testblk"
#define DISK_SIZE_KiB 448
#define DISK_SIZE_BYTES (DISK_SIZE_KiB * 1024)  // 448 KiB
#define CAPACITY_SECTORS (DISK_SIZE_BYTES / SECTOR_SIZE)  // 896 секторов
#define BDISKMAJOR		1

//static unsigned long rd_size = 4096;

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
static int bd_do_bvec(struct block_dev *bd, struct page *page, unsigned int len, unsigned int off, blk_opf_t opf, sector_t sector);
static void copy_from_bd(void *dst, struct block_dev *bd, sector_t sector, size_t n);
static int copy_to_bd_setup(struct block_dev *bd, sector_t sector, size_t n, gfp_t gfp);
static int bd_insert_page(struct block_dev *bd, sector_t sector, gfp_t gfp);
static struct page *bd_lookup_page(struct block_dev *bd, sector_t sector);
static void copy_from_bd(void *dst, struct block_dev *brd, sector_t sector, size_t n);
static void copy_to_bd(struct block_dev *bd, const void *src, sector_t sector, size_t n);




static const struct block_device_operations bd_fops = {
	.owner =		THIS_MODULE,
	.submit_bio =		bd_submit_bio,
	.rw_page =		bd_rw_page,
};

static int bd_rw_page(struct block_device *bdev, sector_t sector, struct page *page, enum req_op op) {
    struct block_dev *bd = bdev->bd_disk->private_data;
	int err;

	if (PageTransHuge(page))
		return -ENOTSUPP;
	err = bd_do_bvec(bd, page, PAGE_SIZE, 0, op, sector);
	page_endio(page, op_is_write(op), err);
	return err;
}
/*
bd_insert_page() — чтобы проверить, существует ли уже страница;
в bd_do_bvec() — при чтении или записи данных, чтобы получить страницу, в которую нужно копировать данные;
в copy_to_brd() / copy_from_brd() — при фактическом копировании данных.*/
static struct page *bd_lookup_page(struct block_dev *bd, sector_t sector)
{
	pgoff_t idx = sector >> PAGE_SECTORS_SHIFT;
	struct page *page;

	rcu_read_lock();
	page = radix_tree_lookup(&bd->bd_pages, idx);
	rcu_read_unlock();

	return page;
}


/*
bd — RAM-диск (struct brd_device), содержащий radix_tree_root bd_pages;
sector — номер сектора, куда собираются писать данные.
gfp — флаги аллокации (GFP_NOIO, GFP_NOWAIT, и т.п.), определяют, можно ли спать при выделении памяти.
*/
static int bd_insert_page(struct block_dev *bd, sector_t sector, gfp_t gfp)
{
    //idx — индекс страницы (в единицах страниц), вычисляется из сектора позже.
	pgoff_t idx;
    // указатель на страницу, если она найдена или выделена.
	struct page *page;
	int ret = 0;
    //Проверка, существует ли уже страница
	page = bd_lookup_page(bd, sector);
	if (page)
		return 0;
    /*
   alloc_page() — выделяет физическую страницу памяти (struct page).
    Флаги:
    __GFP_ZERO — заполняет страницу нулями (чтобы содержимое RAM-диска было изначально чистым).
    __GFP_HIGHMEM — разрешает аллокацию страниц в зоне HIGHMEM (если архитектура поддерживает).
    Если памяти нет → -ENOMEM.
    */
	page = alloc_page(gfp | __GFP_ZERO | __GFP_HIGHMEM);
	if (!page)
		return -ENOMEM;
/*
radix_tree_maybe_preload() заранее резервирует память для внутренних узлов дерева (чтобы не делать аллокацию под спинлоком).
*/
	if (radix_tree_maybe_preload(gfp)) {
		__free_page(page);
		return -ENOMEM;
	}
/*Вычисляем индекс страницы:
PAGE_SECTORS_SHIFT = PAGE_SHIFT - SECTOR_SHIFT (обычно 12 - 9 = 3)
т.е. idx = sector / 8 (одна страница = 8 секторов)
Сохраняем page->index = idx — чтобы знать, какому смещению соответствует страница.*/
	spin_lock(&bd->bd_lock);
	idx = sector >> PAGE_SECTORS_SHIFT;
	page->index = idx;
    /*
    radix_tree_insert() вставляет page под ключ idx в дерево brd->brd_pages.
    Если вставка удачна:
    Увеличиваем счётчик brd_nr_pages — всего страниц в RAM-диске.
    Если ошибка:
    Освобождаем только что выделенную страницу (__free_page(page)).
    Проверяем, почему:
    Если страница с таким индексом всё-таки уже появилась — возможно, конкурентная вставка → просто lookup.
    Если lookup не возвращает страницу — ошибка аллокации.
    Если index не совпадает — повреждение данных (-EIO).
    */
	if (radix_tree_insert(&bd->bd_pages, idx, page)) {
		__free_page(page);
		page = radix_tree_lookup(&bd->bd_pages, idx);
		if (!page)
			ret = -ENOMEM;
		else if (page->index != idx)
			ret = -EIO;
	} else {
		bd->bd_nr_pages++;
	}
	spin_unlock(&bd->bd_lock);

	radix_tree_preload_end();
	return ret;
}
/*
copy_from_bd() выполняет чтение из виртуального RAM-диска в буфер, предоставленный подсистемой ввода-вывода (bio).
dst — адрес назначения (куда копировать данные, например bio-буфер).
brd — RAM-диск (struct brd_device).
sector — сектор, с которого читаем.
n — количество байт, которые нужно прочитать.*/
static void copy_from_bd(void *dst, struct block_dev *brd,
			sector_t sector, size_t n)
{
	struct page *page;
	void *src;
    /*
    Вычисляем байтовое смещение в первой странице.
    PAGE_SECTORS = PAGE_SIZE / 512 (обычно 8)
    SECTOR_SHIFT = 9 (512 байт)
    */
	unsigned int offset = (sector & (PAGE_SECTORS-1)) << SECTOR_SHIFT;
	size_t copy;
/*PAGE_SIZE - offset — сколько байт осталось в текущей странице.
copy — сколько реально скопируем сейчас.*/
	copy = min_t(size_t, n, PAGE_SIZE - offset);

/*brd_lookup_page() ищет страницу в brd->brd_pages (radix-tree).
Если страница существует — возвращает struct page *.
Если нет — NULL.
То есть, если мы читаем секторы, которые ещё не записывались — там нет страниц, и мы должны вернуть нули.*/
	page = bd_lookup_page(brd, sector);
	if (page) {
		src = kmap_atomic(page);
		memcpy(dst, src + offset, copy);
		kunmap_atomic(src);
	} else
		memset(dst, 0, copy);

	if (copy < n) {
		dst += copy;
		sector += copy >> SECTOR_SHIFT;
		copy = n - copy;
		page = bd_lookup_page(brd, sector);
		if (page) {
			src = kmap_atomic(page);
			memcpy(dst, src, copy);
			kunmap_atomic(src);
		} else
			memset(dst, 0, copy);
	}
}

static void copy_to_bd(struct block_dev *bd, const void *src, sector_t sector, size_t n){
    struct page *page;
	void *dst;

    /*Вычисляется смещение в первой странице (аналогично copy_from_brd()).
PAGE_SECTORS = PAGE_SIZE / 512 — количество секторов в одной странице.
SECTOR_SHIFT = 9 (512 байт).*/
	unsigned int offset = (sector & (PAGE_SECTORS-1)) << SECTOR_SHIFT;
	size_t copy;

	copy = min_t(size_t, n, PAGE_SIZE - offset);
	page = bd_lookup_page(bd, sector);
	BUG_ON(!page);

	dst = kmap_atomic(page);
	memcpy(dst + offset, src, copy);
	kunmap_atomic(dst);

	if (copy < n) {
		src += copy;
		sector += copy >> SECTOR_SHIFT;
		copy = n - copy;
		page = bd_lookup_page(bd, sector);
		BUG_ON(!page);

		dst = kmap_atomic(page);
		memcpy(dst, src, copy);
		kunmap_atomic(dst);
	}
}


/*
bd — указатель на RAM-диск (struct block_dev), содержащий radix-дерево с данными.
sector — начальный сектор операции (позиция в логических секторах).
n — количество байт, которые будут записаны.
gfp — флаги выделения памяти (GFP_NOIO, GFP_NOWAIT, и т.п.) для безопасной аллокации в контексте блочного драйвера.
*/
static int copy_to_bd_setup(struct block_dev *bd, sector_t sector,
			     size_t n, gfp_t gfp) {
    
    /*
    PAGE_SECTORS = PAGE_SIZE / SECTOR_SIZE (например, 4096 / 512 = 8).
    (sector & (PAGE_SECTORS - 1)) даёт номер сектора внутри текущей страницы (0–7).
    << SECTOR_SHIFT (обычно << 9) преобразует этот индекс в байтовое смещение внутри страницы.
    */
    unsigned int offset = (sector & (PAGE_SECTORS-1)) << SECTOR_SHIFT;
	size_t copy;
	int ret;

    /* PAGE_SIZE - offset — сколько байт помещается в текущую страницу, начиная с offset.
        min_t() гарантирует, что мы не выйдем за пределы данных (n). */
	copy = min_t(size_t, n, PAGE_SIZE - offset);

	ret = bd_insert_page(bd, sector, gfp);
	if (ret)
		return ret;
    // Если данные выходят за границу страницы — выделяем вторую
	if (copy < n) {
		sector += copy >> SECTOR_SHIFT;
		ret = bd_insert_page(bd, sector, gfp);
	}
	return ret;

        
}


/* функция выполняет одну операцию ввода/вывода для одного сегмента bio (bvec).
    bd — указатель на структуру RAM-диска (struct brd_device), содержащую radix-дерево страниц и метаданные.
    page — страница (структура struct page) из BIO (то есть, куда писать или откуда читать).
    len — длина сегмента в байтах.
    off — смещение внутри этой страницы.
    opf — флаги операции (REQ_OP_READ, REQ_OP_WRITE, REQ_NOWAIT, и др.).
    sector — сектор устройства, с которого начинается этот сегмент.
*/
static int bd_do_bvec(struct block_dev *bd, struct page *page, unsigned int len, 
    unsigned int off, blk_opf_t opf, sector_t sector) {
        int err = 0;
        void *mem; 
        /* Макрос op_is_write() проверяет тип операции (REQ_OP_WRITE, REQ_OP_FLUSH, и др.). */
        if (op_is_write(opf)) {  
            /*
            Если BIO помечено REQ_NOWAIT, то нельзя блокироваться → используем GFP_NOWAIT.
            Иначе — GFP_NOIO, чтобы запретить I/O при аллокации (во избежание рекурсии в блоковый слой)
            */
            gfp_t gfp = opf & REQ_NOWAIT ? GFP_NOWAIT : GFP_NOIO;
            err = copy_to_bd_setup(bd, sector, len, gfp);
            if (err) goto out;
           
        }
        mem = kmap_atomic(page);
        if (!op_is_write(opf)) {
            copy_from_bd(mem + off, bd, sector, len);
		    flush_dcache_page(page);
        } else {
            flush_dcache_page(page);
		    copy_to_bd(bd, mem + off, sector, len);
        }
        kunmap_atomic(mem);

out:
        return err;

}


/*
 * bd_submit_bio -- точка входа I/O 
 * - вызывается block layer при приходе BIO
 * - перебирает сегменты bio_for_each_segment и вызывает brd_do_bvec
 * - проверки: выравнивание (offset и len должны быть кратны SECTOR_SIZE)
 * - особое поведение: если copy_to_brd_setup вернул -ENOMEM и био с REQ_NOWAIT,
 *   то вызывается bio_wouldblock_error(bio) — правильная реакция для frontends
 *   которые не хотят ждать.
 
 Получить контекст устройства (bd) и стартовый сектор из BIO.
    Перебрать все сегменты BIO (bio_for_each_segment). Для каждого сегмента:
    проверить выравнивание (WARN_ON_ONCE — диагностически);
    вызвать bd_do_bvec, который:
    при записи — подготовит backing-страницы (alloc_page + radix tree insert), затем скопирует данные;
    при чтении — скопирует данные из backing-страниц в page (или заполнит нулями, если страница отсутствует).
    если brd_do_bvec вернул ошибку:
    если это -ENOMEM и BIO был REQ_NOWAIT — ответить bio_wouldblock_error;
    в противном случае — bio_io_error.
    сдвинуть сектор на длину сегмента.
    Если все сегменты успешно обработаны — вызвать bio_endio(bio).
 */
static void bd_submit_bio(struct bio* bio) {
    /*
    Извлекаем bd_device через связанное block_device (bio->bi_bdev) → gendisk → private_data. 
    То есть каждый gendisk хранит указатель на структуру bd, 
    и здесь мы получаем контекст устройства, на котором выполняется этот BIO.
    */
    struct block_dev *bd = bio->bi_bdev->bd_disk->private_data;
    
    
    /*
    Читаем стартовый сектор операции из BIO iterator. Это сектор на устройстве, с которого начинается I/O. 
    Дальше при обходе сегментов мы будем увеличивать sector на число секторов в каждом сегменте.
    */
    sector_t sector = bio->bi_iter.bi_sector;
    struct bio_vec bvec;
    struct bvec_iter iter;
    bio_for_each_segment(bvec, bio, iter) {
        unsigned int len = bvec.bv_len;
        int err;
        WARN_ON_ONCE((bvec.bv_offset & (SECTOR_SIZE -1)) || (len & (SECTOR_SIZE -1)));  // retuen EIO better
        /*
        обработать одну векторную часть BIO. Передаём:
        bd — наше устройство,
        bvec.bv_page — страница фронтенда (куда читать или откуда писать),
        len — длина в байтах,
        bvec.bv_offset — смещение в page,
        bio->bi_opf — operation flags (read/write, plus REQ_NOWAIT и др.),
        sector — сектор на устройстве, соответствующий началу этого сегмента.
        brd_do_bvec сделает необходимый kmap_atomic, подготовку backing-страниц (для записи), копирование и т.п.
        */
        err = bd_do_bvec(bd, bvec.bv_page, len, bvec.bv_offset, bio->bi_opf, sector);
        if (err) {
			if (err == -ENOMEM && bio->bi_opf & REQ_NOWAIT) {
				bio_wouldblock_error(bio);
				return;
			}
			bio_io_error(bio);
			return;
		}
		sector += len >> SECTOR_SHIFT;
	}
	bio_endio(bio);

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
    
    error = add_disk(disk);// завершение регистрации
    if (error)   goto cleanup_disk;

    return 0;



cleanup_disk:
    put_disk(disk);

out_free_dev:
    
    return error;
} 

#define FREE_BATCH 16
static void bd_free_pages(struct block_dev *bd)
{
	unsigned long pos = 0;
	struct page *pages[FREE_BATCH];
	int nr_pages;

	do {
		int i;

		nr_pages = radix_tree_gang_lookup(&bd->bd_pages,
				(void **)pages, pos, FREE_BATCH);

		for (i = 0; i < nr_pages; i++) {
			void *ret;

			BUG_ON(pages[i]->index < pos);
			pos = pages[i]->index;
			ret = radix_tree_delete(&bd->bd_pages, pos);
			BUG_ON(!ret || ret != pages[i]);
			__free_page(pages[i]);
		}

		pos++;

		
		cond_resched();

		/*
		 * This assumes radix_tree_gang_lookup always returns as
		 * many pages as possible. If the radix-tree code changes,
		 * so will this have to.
		 */
	} while (nr_pages == FREE_BATCH);
}

static void bd_free_device(struct block_dev *brd)
{
	mutex_lock(&bd_devices_mutex);
	list_del(&brd->bd_list);
	mutex_unlock(&bd_devices_mutex);
	kfree(brd);
}
static void brd_cleanup(void)
{
	struct block_dev *bd, *next;

	debugfs_remove_recursive(bd_debugfs_dir);

	list_for_each_entry_safe(bd, next, &bd_devices, bd_list) {
		del_gendisk(bd->gdisk);
		put_disk(bd->gdisk);
		bd_free_pages(bd);
		bd_free_device(bd);
	}
}

static int __init mblock_driver_init(void)
{
    proc_dir = proc_mkdir(PROC_DIR_NAME, NULL);
    if (!proc_dir) {
        pr_warn("Failed to create proc dir\n");
        return -ENOMEM;
    }
    proc_file = proc_create(PROC_FILE_NAME, 0666, proc_dir, &proc_fops);
    if (!proc_file) {
        pr_warn("Failed to create proc file\n");
        remove_proc_entry(PROC_DIR_NAME, NULL);
        return -ENOMEM;
    }

    bd_debugfs_dir = debugfs_create_dir("ramdisk_pages", NULL);
    int error = 0;
    int major = register_blkdev(BDISKMAJOR, DEVICE_NAME);
    if (major < 0) {
        error = -EIO;
        return major;
    }
    for (int i = 0; i < 3; i++)
		bd_alloc(i);
    pr_info("module loaded\n");
    return 0;


}

static void __exit mblock_driver_exit(void)
{
    // Удаляем файл и каталог
    if (proc_file)
        remove_proc_entry(PROC_FILE_NAME, proc_dir);

    if (proc_dir)
        remove_proc_entry(PROC_DIR_NAME, NULL);
    
    unregister_blkdev(BDISKMAJOR, DEVICE_NAME);
    brd_cleanup();
	pr_info("module unloaded\n");
}

module_init(mblock_driver_init);
module_exit(mblock_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stv");
MODULE_DESCRIPTION("RAM-backed block device for kernel 6.1.130");
MODULE_VERSION("1.1");