#include "ADT/linked_list.h"
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

typedef struct simple_ll_db {
  char *name;
  long id;
  struct list_head node;
} record_db_t;

static LIST_HEAD(g_records);

__attribute__((unused)) static void record_db_init_list(void) {
  INIT_LIST_HEAD(&g_records);
}

static record_db_t *record_db_new(const char *name, long id) {
  record_db_t *rec = malloc(sizeof(*rec));
  if (!rec)
    return NULL;

  rec->name = strdup(name ? name : "");
  if (!rec->name) {
    free(rec);
    return NULL;
  }
  rec->id = id;
  return rec;
}

static int record_db_add(const char *name, long id) {
  record_db_t *rec = record_db_new(name, id);
  if (!rec)
    return -1;

  list_add_tail(&rec->node, &g_records);
  return 0;
}

static void record_db_free(record_db_t *rec) {
  if (!rec)
    return;
  free(rec->name);
  free(rec);
}

static int count = 42;
long *current_ids;

#define NUM_NAMES 24
#define NAME_LENGTH 20
static pthread_rwlock_t db_lock;
#define READERS 5
#define WRITERS 2

static void *writer_thread(void *args);
static void *reader_thread(void *args);
static volatile int running = 1;

pthread_mutex_t rmutex = PTHREAD_MUTEX_INITIALIZER; // Защищает readers_count
pthread_mutex_t rw_mutex =
    PTHREAD_MUTEX_INITIALIZER; // Защищает запись/чтение shared_data
int readers_count = 0;
int shared_data = 0;

const char *random_names[] = {
    "Alice", "Bob",   "Charlie", "David",    "Eva",     "Frank",
    "Grace", "Henry", "Ivy",     "Jack",     "Kate",    "Liam",
    "Mia",   "Noah",  "Olivia",  "Peter",    "Quinn",   "Rachel",
    "Sam",   "Tina",  "Ulysses", "Victoria", "William", "Zoe"};

static void init_db() {
  current_ids = malloc(sizeof(long) * count);
  srand(time(NULL));
  for (int i = 0; i < count; i++) {
    int name_index = rand() % NUM_NAMES;
    char *name = (char *)malloc(NAME_LENGTH * sizeof(char));
    if (name != NULL) {
      snprintf(name, NAME_LENGTH, "%s", random_names[name_index]);

    } else {
      continue;
    }
    long id = 1000 + rand() % 9000;
    current_ids[i] = id;
    record_db_add(name, id);
    free(name);
  }
}

static void free_db() {
  record_db_t *pos, *n;
  list_for_each_entry_safe(pos, n, &g_records, node) {
    list_del(&pos->node);
    record_db_free(pos);
  }
  free(current_ids);
}

static record_db_t *record_db_find_by_id(long id) {
  record_db_t *pos;

  list_for_each_entry(pos, &g_records, node) {
    if (pos->id == id) {
      return pos;
    }
  }
  return NULL;
}

/* Поиск по имени */
__attribute__((unused)) static record_db_t *
record_db_find_by_name(const char *name) {
  record_db_t *pos;

  if (!name)
    return NULL;

  list_for_each_entry(pos, &g_records, node) {
    if (strcmp(pos->name, name) == 0) {
      return pos;
    }
  }
  return NULL;
}

static void print_db() {
  printf("=== Local Simple DataBase ===\n");
  record_db_t *pos;

  list_for_each_entry(pos, &g_records, node) {
    printf("record: id=%ld name=%s\n", pos->id, pos->name);
  }
  printf("===============================\n");

  //  record_db_find_by_id
}

static int record_db_update_by_id(long old_id, long new_id,
                                  const char *new_name) {
  record_db_t *pos;
  char *dup;
  const char *nm = new_name ? new_name : "";

  dup = strdup(nm);
  if (!dup)
    return -1;

  list_for_each_entry(pos, &g_records, node) {
    if (pos->id == old_id) {
      char *old = pos->name;
      pos->id = new_id;
      pos->name = dup;

      free(old);
      return 0;
    }
  }

  free(dup);
  return -1;
}

static void *reader_thread(void *args) {
  int id_local = *(int *)args;
  /*  first version reading  */
  unsigned int seed =
      (unsigned int)time(NULL) ^ (unsigned int)(uintptr_t)pthread_self();
  char name_copy[NAME_LENGTH];

  free(args);
  for (int i = 0; i < READERS; i++) {
    int idx = rand_r(&seed) % count;
    // Имитация времени между чтениями
    usleep(1000);

    // >>> ВХОД В КРИТИЧЕСКУЮ СЕКЦИЮ ДЛЯ ЧТЕНИЯ >>>
    // Заблокировать доступ к readers_count
    pthread_mutex_lock(&rmutex);
    readers_count++;
    if (readers_count == 1) {
      // Если это первый читатель, заблокировать писателей
      pthread_mutex_lock(&rw_mutex);
    }
    record_db_t *id = record_db_find_by_id(idx);
    pthread_mutex_unlock(&rmutex); // Разблокировать readers_count

    // >>> НАЧАЛО ЧТЕНИЯ >>>
    if (id == NULL) {
      printf("[R] Record with idx=%d not found\n", idx);
      return NULL; // или обработать ошибку appropriately
    }

    char *out_id = strdup(id->name);
    if (out_id == NULL) {
      printf("[R] Memory allocation failed\n");
      return NULL; // обработка ошибки выделения памяти
    }
    printf("[R] read idx=%d name=%s id=%ld\n", idx, out_id, id->id);
    usleep(500); // Имитация времени чтения
    // <<< КОНЕЦ ЧТЕНИЯ <<<

    // >>> ВЫХОД ИЗ КРИТИЧЕСКОЙ СЕКЦИИ ДЛЯ ЧТЕНИЯ >>>
    free(out_id);
    // Заблокировать доступ к readers_count
    pthread_mutex_lock(&rmutex);
    readers_count--;
    if (readers_count == 0) {
      // Если это последний читатель, разблокировать писателей
      pthread_mutex_unlock(&rw_mutex);
    }
    pthread_mutex_unlock(&rmutex); // Разблокировать readers_count
  }

  pthread_exit(NULL);
  return NULL;
}

static void *writer_thread(void *args) {
  int id_local = *(int *)args;
  unsigned int seed =
      (unsigned int)time(NULL) ^ (unsigned int)(uintptr_t)pthread_self();

  /* while (id_local) {
    int idx = rand_r(&seed) % count;

    char *new_name = (char *)malloc(NAME_LENGTH);
    if (!new_name) {
      perror("malloc");
      break;
    }
    int name_index = rand_r(&seed) % NUM_NAMES;
    snprintf(new_name, NAME_LENGTH, "%s", random_names[name_index]);
    long new_id = 1000 + (rand_r(&seed) % 9000);

    pthread_rwlock_wrlock(&db_lock);


    record_db_t *old_id = record_db_find_by_id(idx);

    record_db_update_by_id(idx, new_id,  new_name);


    pthread_rwlock_unlock(&db_lock);


    printf("Thread: %d [W] write idx=%d %ld/%s -> %ld/%s\n", id_local, idx,
  old_id->id, "(old)", new_id, new_name);

    usleep(120 * 1000); // 120 ms
    free(new_name);
    id_local--;

  } */
  for (int i = 0; i < WRITERS; i++) {
    // Имитация времени между записями
    usleep(1500);

    // >>> ВХОД В КРИТИЧЕСКУЮ СЕКЦИЮ ДЛЯ ЗАПИСИ >>>
    pthread_mutex_lock(&rw_mutex);

    // >>> НАЧАЛО ЗАПИСИ >>>
    printf("\tWriter: %d starts record\n", id_local);
    shared_data++; // Изменение разделяемых данных
    usleep(1000);  // Имитация времени записи
    printf("\tWriter %d ended make record: new value = %d\n", id_local,
           shared_data);
    // <<< КОНЕЦ ЗАПИСИ <<<

    // >>> ВЫХОД ИЗ КРИТИЧЕСКОЙ СЕКЦИИ ДЛЯ ЗАПИСИ >>>
    pthread_mutex_unlock(&rw_mutex);
  }

  pthread_exit(NULL);
  free(args);
  return NULL;
}

int main(void) {

  init_db();
  /*  xx
    pthread_t readers[READERS], writers[WRITERS];
    for (int i = 0; i < READERS; i++) {
      printf("Create reader\n");
      int *id = malloc(sizeof(int));
      *id = i + 1;
      if (pthread_create(&readers[i], NULL, reader_thread, id)) {
        perror("Cannot create reader");
        return 1;
      }
    }

    for (int i = 0; i < WRITERS; i++) {
      printf("Create writer\n");
      int *id = malloc(sizeof(int));
      *id = i + 1;
      if (pthread_create(&writers[i], NULL, writer_thread, id)) {
        perror("Cannot create writer");
        return 1;
      }
    }

    // Ожидание завершения всех читателей
    for (int i = 0; i < READERS; i++) {
      pthread_join(readers[i], NULL);
    }

    // Ожидание завершения всех писателей
    for (int i = 0; i < WRITERS; i++) {
      pthread_join(writers[i], NULL);
    }
  */
  print_db();

  free_db();
  return 0;
}
