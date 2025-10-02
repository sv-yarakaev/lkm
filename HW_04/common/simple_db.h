#include "ADT/linked_list.h"

typedef struct record_db {
  char *name;
  long id;
  struct list_head node;
} record_db;


struct record_db *simple_db;


int init_db(record_db *simple_db);

int write_db(record_db *simple_db);
