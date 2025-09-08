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

static void record_db_init_list(void) { INIT_LIST_HEAD(&g_records); }

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
#define NUM_NAMES 24
#define NAME_LENGTH 20
static pthread_rwlock_t db_lock;
#define READERS 5
#define WRITERS 2

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
  for (int i = 0; i < count; i++) {
    int name_index = rand() % NUM_NAMES;
    char *name = (char *)malloc(NAME_LENGTH * sizeof(char));
    snprintf(name, NAME_LENGTH, "%s", random_names[name_index]); 
    long id = 1000 + rand() % 9000;
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
}

static record_db_t *record_db_find_by_id(long id)
{
    record_db_t *pos;

    list_for_each_entry(pos, &g_records, node) {
        if (pos->id == id) {
            return pos;
        }
    }
    return NULL;
}



/* Поиск по имени */
static record_db_t *record_db_find_by_name(const char *name)
{
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
}

static int record_db_update_by_id(long old_id, long new_id, const char *new_name)
{
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



int main(void) {
  init_db();
  /*

  */
  print_db();
/*
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
*/
  free_db();
  return 0;
}

static void *reader_thread(void *args) {
  unsigned int seed =
      (unsigned int)time(NULL) ^ (unsigned int)(uintptr_t)pthread_self();
  char name_copy[NAME_LENGTH];
  while (running) {
    int idx = rand_r(&seed) % count;

    pthread_rwlock_rdlock(&db_lock);
    record_db_t* id = record_db_find_by_id(idx);
    snprintf(name_copy, sizeof(name_copy), "%s", id->name);
    pthread_rwlock_unlock(&db_lock);

    printf("[R] read idx=%d name=%s id=%ld\n", idx, name_copy, id->id);

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


    record_db_t *old_id = record_db_find_by_id(idx);

    record_db_update_by_id(idx, new_id,  new_name);


    pthread_rwlock_unlock(&db_lock);


    printf("[W] write idx=%d %ld/%s -> %ld/%s\n", idx, old_id->id, "(old)", new_id,
           new_name);

    usleep(120 * 1000); // 120 ms
    free(new_name);

  }
  return NULL;
}