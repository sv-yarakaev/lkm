#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ADT/linked_list.h"
#include <pthread.h>
#include <time.h>
#include <unistd.h>

typedef struct record_db {
  char *name;
  long id;
} record_db;


typedef struct simple_ll_db {
  char *name;
  long id;
  struct list_head node;
} record_db_t;

static LIST_HEAD(g_records);

/* Инициализация головного списка вручную (если нужен явный вызов)
   И не нужна, если используется LIST_HEAD(g_records) выше.
*/
static void record_db_init_list(void) {
    INIT_LIST_HEAD(&g_records);
}


/* Вспомогательные функции управления элементом */
static record_db_t *record_db_new(const char *name, long id)
{
    record_db_t *rec = malloc(sizeof(*rec));
    if (!rec)
        return NULL;

    rec->name = strdup(name ? name : "");
    if (!rec->name) {
        free(rec);
        return NULL;
    }
    rec->id = id;
    // Для обычных элементов инициализировать rec->node не обязательно
    return rec;
}




/* Добавление элемента в хвост списка */
static int record_db_add(const char *name, long id)
{
    record_db_t *rec = record_db_new(name, id);
    if (!rec)
        return -1;

    list_add_tail(&rec->node, &g_records);
    return 0;
}
static void record_db_free(record_db_t *rec)
{
    if (!rec) return;
    free(rec->name);
    free(rec);
}

/* Полная очистка списка */
static void record_db_clear(void)
{
    record_db_t *pos, *n;

    list_for_each_entry_safe(pos, n, &g_records, node) {
        list_del(&pos->node);
        record_db_free(pos);
    }
}




static int count = 42;
#define NUM_NAMES 24
#define NAME_LENGTH 20
struct record_db *simple_db;
static pthread_rwlock_t db_lock;
#define READERS 5
#define WRITERS 2

static record_db *write_row();
static void *writer_thread(void *args);
static void *reader_thread(void *args);
static volatile int running = 1;

const char *random_names[] = {
    "Alice", "Bob",   "Charlie", "David",    "Eva",     "Frank",
    "Grace", "Henry", "Ivy",     "Jack",     "Kate",    "Liam",
    "Mia",   "Noah",  "Olivia",  "Peter",    "Quinn",   "Rachel",
    "Sam",   "Tina",  "Ulysses", "Victoria", "William", "Zoe"};

static void init_db() {
  srand(time(NULL));
  simple_db = (struct record_db *)malloc(sizeof(struct record_db) * count);
  for (int i = 0; i < count; i++) {
    simple_db[i] = *write_row();
    record_db_add(simple_db[i].name, simple_db[i].id);
  }
}

static record_db *write_row() {
  struct record_db *row = (record_db *)malloc(sizeof(record_db));
  int name_index = rand() % NUM_NAMES;
  row->name = (char *)malloc(NAME_LENGTH * sizeof(char));
  snprintf(row->name, NAME_LENGTH, "%s", random_names[name_index]);
  row->id = 1000 + rand() % 9000;
  return row;
}



static void free_db() {
  for (int i = 0; i < count; i++) {
    if (simple_db[i].name != NULL) {
      free(simple_db[i].name);
    }
  }
  free(simple_db);
  simple_db = NULL;
}

struct record_db read_row_db(int row) { return simple_db[row]; }

static void print_db() {
  printf("=== Local Simple DataBase ===\n");
  for (int i = 0; i < count; i++) {
    printf("Record %d: ID = %ld, Name = %s\n", i, simple_db[i].id,
           simple_db[i].name);
  }
  printf("===============================\n");
  record_db_t *pos;

    list_for_each_entry(pos, &g_records, node) {
        printf("record: id=%ld name=%s\n", pos->id, pos->name);
    }
}



int main(void) {
  init_db();
  /*

  */
  print_db();

  if (pthread_rwlock_init(&db_lock, NULL)) {
    perror("cannot make lock init");
    return 1;
  }
  pthread_t readers[READERS];
  pthread_t writers[WRITERS];

  for (int i = 0; i < READERS; i++) {
    if (pthread_create(&readers[i], NULL, reader_thread, NULL) != 0) {
      perror("pthread_create reader");
      return 1;
    }
  }
  for (int i = 0; i < WRITERS; i++) {
    if (pthread_create(&writers[i], NULL, writer_thread, NULL) != 0) {
      perror("pthread_create writer");
      return 1;
    }
  }
  
  free_db();
  return 0;
}

static void *reader_thread(void *args) {
  (void)args;
  unsigned int seed =
      (unsigned int)time(NULL) ^ (unsigned int)(uintptr_t)pthread_self();
  char name_copy[NAME_LENGTH];
  while (running) {
    int idx = rand_r(&seed) % count;

    pthread_rwlock_rdlock(&db_lock);
    long id = simple_db[idx].id;
    snprintf(name_copy, sizeof(name_copy), "%s", simple_db[idx].name);
    pthread_rwlock_unlock(&db_lock);

    printf("[R] read idx=%d name=%s id=%ld\n", idx, name_copy, id);

    usleep(50 * 1000); // 50 ms
  }
  return NULL;
}

static void *writer_thread(void *args) {
  unsigned int seed =
      (unsigned int)time(NULL) ^ (unsigned int)(uintptr_t)pthread_self();

  while (running) {
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

    char *old_name = simple_db[idx].name;
    long old_id = simple_db[idx].id;

    simple_db[idx].name = new_name;
    simple_db[idx].id = new_id;

    pthread_rwlock_unlock(&db_lock);

    free(old_name);

    printf("[W] write idx=%d %ld/%s -> %ld/%s\n", idx, old_id, "(old)", new_id,
           new_name);

    usleep(120 * 1000); // 120 ms
  }
  return NULL;
}