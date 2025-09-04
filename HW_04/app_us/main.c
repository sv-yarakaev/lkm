#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
#include <time.h>

typedef struct rec_some {
  char *name;
  long id;

} rec_some;

static int count = 10;
#define NUM_NAMES 10
#define NAME_LENGTH 20
struct rec_some *simple_db;

static rec_some* write_row();

const char *random_names[] = {"Alice", "Bob",   "Charlie", "David", "Eva",
                              "Frank", "Grace", "Henry",   "Ivy",   "Jack"};

static void init_db() {
  srand(time(NULL));
  simple_db = (struct rec_some *)malloc(sizeof(struct rec_some) * count);
  for (int i = 0; i < count; i++) {
    simple_db[i] = *write_row();
  }
}

static rec_some* write_row() {
    struct rec_some* row = (rec_some *)malloc(sizeof(rec_some));
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

struct rec_some read_row_db(int row) { return simple_db[row]; }

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