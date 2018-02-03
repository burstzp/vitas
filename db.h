#ifndef DB
#define DB

#include "leveldb/c.h"

typedef struct _db {
  leveldb_t *db;
  leveldb_comparator_t* cmp;
  leveldb_cache_t* cache;
  leveldb_env_t* env;
  leveldb_options_t* options;
  leveldb_readoptions_t* roptions;
  leveldb_writeoptions_t* woptions;
} sdb;

sdb* sdb_t;

sdb* new_db();
void destroy_db(sdb*);

int db_init(const char*, int); 
int db_put(char*, size_t, char*, size_t, int);
int db_get(char*, size_t, char**, size_t*);
int db_delete(char*, size_t);
void db_cleanup();
#endif
