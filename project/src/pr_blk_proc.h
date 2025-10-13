#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define PROC_DIR_NAME "bramdev"
#define PROC_FILE_NAME "stat"

// static struct proc_dir_entry *proc_dir = NULL;
// static struct proc_dir_entry *proc_file = NULL;

// buffer for data
static char *data_buffer = NULL;
static size_t buffer_size = 0;
static DEFINE_MUTEX(buffer_mutex);

// Функция открытия файла
static int proc_open(struct inode *inode, struct file *file) { return 0; }
// Функция закрытия файла
static int proc_release(struct inode *inode, struct file *file) { return 0; }

static ssize_t proc_read(struct file *file, char __user *buf, size_t count,
                         loff_t *ppos) {
  ssize_t ret = 0;

  mutex_lock(&buffer_mutex);

  if (*ppos >= buffer_size) {
    mutex_unlock(&buffer_mutex);
    return 0; // Конец файла
  }

  if (*ppos + count > buffer_size)
    count = buffer_size - *ppos;
  if (copy_to_user(buf, data_buffer + *ppos, count)) {
    mutex_unlock(&buffer_mutex);
    return -EFAULT;
  }

  *ppos += count;
  ret = count;

  mutex_unlock(&buffer_mutex);
  return ret;
}

static ssize_t proc_write(struct file *file, const char __user *buf,
                          size_t count, loff_t *ppos) {
  char *new_buffer = NULL;

  mutex_lock(&buffer_mutex);

  // Освобождаем старый буфер и выделяем новый
  kfree(data_buffer);
  data_buffer = NULL;
  buffer_size = 0;
  if (copy_from_user(new_buffer, buf, count)) {
    kfree(new_buffer);
    mutex_unlock(&buffer_mutex);
    return -EFAULT;
  }

  new_buffer[count] = '\0'; // Добавляем нулевой терминатор
  data_buffer = new_buffer;
  buffer_size = count;

  mutex_unlock(&buffer_mutex);
  return count;
}

// Структура файловых операций
const struct proc_ops proc_fops = {
    .proc_open = proc_open,
    .proc_read = proc_read,
    .proc_write = proc_write,
    .proc_release = proc_release,
};
