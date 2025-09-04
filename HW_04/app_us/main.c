#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
#include <time.h>

typedef struct record_db {
  char *name;
  long id;
} record_db;

static int count = 42;
#define NUM_NAMES 24
#define NAME_LENGTH 20
struct record_db *simple_db;

static record_db* write_row();


const char* random_names[] = {
    "Alice", "Bob", "Charlie", "David", "Eva",
    "Frank", "Grace", "Henry", "Ivy", "Jack",
    "Kate", "Liam", "Mia", "Noah", "Olivia",
    "Peter", "Quinn", "Rachel", "Sam", "Tina",
    "Ulysses", "Victoria", "William", "Zoe"
};


static void init_db() {
  srand(time(NULL));
  simple_db = (struct record_db *)malloc(sizeof(struct record_db) * count);
  for (int i = 0; i < count; i++) {
    simple_db[i] = *write_row();
  }
}

static record_db* write_row() {
    struct record_db* row = (record_db *)malloc(sizeof(record_db));
    int name_index = rand() % NUM_NAMES;
    row->name = (char *) malloc(NAME_LENGTH * sizeof(char));
//    strcpy(row->name, random_names[name_index]);
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
        printf("Record %d: ID = %ld, Name = %s\n", 
               i, simple_db[i].id, simple_db[i].name);
    }
    printf("===============================\n");
}

int main(void) {
  init_db();
  /*

  */
  print_db();
  free_db();
  return 0;
}