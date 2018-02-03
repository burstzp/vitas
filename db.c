#include "common.h"

#define MAX_DB_CLEAN_LOOP 100

sdb* new_db() 
{
  sdb* sdb_t = (sdb*)calloc(1, sizeof(sdb));
  if (sdb_t == NULL) {
    LOG(L_FATAL, "new_db() malloc error");
    return NULL;
  } 

  return sdb_t;
}

int db_init(const char* dbpath, int max_files) 
{
  char *err = NULL;
  sdb_t->roptions = leveldb_readoptions_create();
  leveldb_readoptions_set_verify_checksums(sdb_t->roptions, 0);
  leveldb_readoptions_set_fill_cache(sdb_t->roptions, 1);

  sdb_t->options = leveldb_options_create();
  sdb_t->env = leveldb_create_default_env();
  leveldb_options_set_create_if_missing(sdb_t->options, 1);
  leveldb_options_set_max_open_files(sdb_t->options, max_files);
  sdb_t->woptions = leveldb_writeoptions_create();
  leveldb_options_set_write_buffer_size(sdb_t->options, 1024*1024*128);
  leveldb_options_set_paranoid_checks(sdb_t->options, 1);
  leveldb_options_set_max_open_files(sdb_t->options, max_files);
  leveldb_options_set_block_size(sdb_t->options, 1024);
  leveldb_options_set_block_restart_interval(sdb_t->options, 8);

  sdb_t->db = leveldb_open(sdb_t->options, dbpath, &err);
  if (err != NULL) {
    LOG(L_FATAL, "leveldb_open() fail!, error '%s'", err);
    leveldb_free(err); 
    leveldb_options_destroy(sdb_t->options);
    leveldb_readoptions_destroy(sdb_t->roptions);
    leveldb_writeoptions_destroy(sdb_t->woptions);
    return 0;
  }
  
  return 1;
}

static inline
int big_endian(int v){
  return (v>>8) | (v<<8);
}

int is_little_endian(void)
{
  unsigned short flag=0x4321;
  if (*(unsigned char*)&flag==0x21)
    return 1;
  else
    return 0;
}
void
decode_ttl_key(const char *buf, size_t buflen, char* key, size_t* key_len, int *ttl)
{
  size_t sz = sizeof(int);
  memcpy(ttl, buf + 4, sz);
  sz += 4;
  *key_len = buflen - sz;
  if (*key_len > 0) {
    memcpy(key, buf + sz, *key_len);
  }
}

size_t 
encode_ttl_key(char *buf, size_t buflen, char* key, size_t key_len, int ttl)
{
  int t = big_endian(ttl);
  size_t sz = 4;
  memcpy(buf,"0TTL",sz);
  memcpy(buf + sz, &t, sizeof(int));
  sz += sizeof(int);
  memcpy(buf + sz, key, key_len);
  sz += key_len;
  return sz;
}

int 
db_put(char* key, size_t key_len, char* val, size_t val_len, int expired) 
{
  char* err = NULL;
  if (expired == 0) {
    leveldb_put(sdb_t->db, sdb_t->woptions, key, key_len, val, val_len, &err);
  } else {
    leveldb_writebatch_t* wb = leveldb_writebatch_create();
    leveldb_writebatch_put(wb, key, key_len, val, val_len);
    char buf[MAX_HEADER_SIZE] = {0};
    size_t buflen = encode_ttl_key(buf, sizeof(buf), key, key_len, expired);
    char ttl[11] = {0};
    snprintf(ttl, sizeof(ttl), "%d", expired);
    leveldb_writebatch_put(wb, buf, buflen, ttl, sizeof(ttl));
    leveldb_write(sdb_t->db, sdb_t->woptions, wb, &err);
    leveldb_writebatch_destroy(wb);
  }

  if (err == NULL) {
    return 1;
  } else {
    LOG(L_ERROR, "leveldb_put() fail!, error '%s'", err);
    leveldb_free(err);
    return 0;
  }
}

int 
db_get(char* key, size_t keylen, char**read, size_t *read_len)
{
  char* err = NULL;
  char* t_read = leveldb_get(sdb_t->db, sdb_t->roptions, key, strlen(key), read_len, &err);
  if (err != NULL) {
    LOG(L_ERROR, "leveldb_get() fail! error '%s'", err);
    return 0;
  }
  
  *read = t_read;
  leveldb_free(err);
  return 1;
}

int 
db_delete(char*key, size_t key_len)
{
  char *err = NULL;
  leveldb_delete(sdb_t->db, sdb_t->woptions, key, key_len, &err);
  if (err != NULL) {
    LOG(L_ERROR, "leveldb_delete() fail! error '%s'", err);
    leveldb_free(err); 
    return 0;
  }

  return 1;
}

void 
db_cleanup()
{
  leveldb_iterator_t* iter = leveldb_create_iterator(sdb_t->db, sdb_t->roptions);
  leveldb_iter_seek_to_first(iter);
  int loop = 0;
  leveldb_writebatch_t* wb = leveldb_writebatch_create();
  while (loop++ < MAX_DB_CLEAN_LOOP && leveldb_iter_valid(iter)) {
    size_t len;
    const char* str;
    str = leveldb_iter_key(iter, &len);
    if (strncmp(str, "0TTL", 4) != 0) {
      break;
    } 

    char key[MAX_HEADER_SIZE] = {0};
    size_t key_len = 0;
    int ttl = 0;
    decode_ttl_key(str, len, key, &key_len, &ttl);
    size_t val_len = 0;
    const char* sttl = leveldb_iter_value(iter, &val_len);
    if (val_len > 0 && atoi(sttl) < time(NULL)) {
      leveldb_writebatch_delete(wb, key, key_len);
      leveldb_writebatch_delete(wb, str, len);
    }

    leveldb_iter_next(iter);
  } 

  char *err = NULL;
  leveldb_write(sdb_t->db, sdb_t->woptions, wb, &err);
  if (err != NULL) {
    LOG(L_ERROR, "db_cleanup() fail! error '%s'", err);
  }

  leveldb_free(err);
  leveldb_writebatch_destroy(wb);
  leveldb_iter_destroy(iter);
}

void 
destroy_db(sdb* sdb_t) {
  if (sdb_t != NULL) {
    LOG(L_INFO, "destroy_db()");
    leveldb_close(sdb_t->db);
    leveldb_options_destroy(sdb_t->options);
    leveldb_readoptions_destroy(sdb_t->roptions);
    leveldb_writeoptions_destroy(sdb_t->woptions);
    //leveldb_cache_destroy(sdb_t->cache);
    //leveldb_comparator_destroy(sdb_t->cmp);
    //leveldb_env_destroy(sdb_t->env);
    free(sdb_t);
    sdb_t = NULL;
  }
}
