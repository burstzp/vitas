#ifndef ACTION
#define ACTION
#include "item.h"
#include "client.h"

#define STORED  "STORED\r\n"
#define END     "\r\nEND\r\n"
#define DELETED "DELETED\r\n"
#define UNKOWN  "UNKOWN\r\n"
#define STATS   "STATS\r\n"
#define ERROR   "ERROR\r\n"

int memcached_get(client*, void*);
int memcached_set(client*, void*);
int memcached_delete(client*, void*);
int memcached_quit(client*, void*);
int memcached_stats(client*, void*);
int memcached_unkown(client*, void*);
void __memcached_string(client*,char*);

#define IS_MEMCACHED_CMD_QUIT(x) strncmp(x,"quit",strlen("quit")) == 0
#define IS_MEMCACHED_CMD_STATS(x) strncmp(x,"stats",strlen("stats")) == 0
#define IS_MEMCACHED_CMD_GET(x) strncmp(x,"get",strlen("get")) == 0
#define IS_MEMCACHED_CMD_SET(x) strncmp(x,"set",strlen("set")) == 0
#define IS_MEMCACHED_CMD_DELETE(x) strncmp(x,"delete",strlen("delete")) == 0

#define MEMCACHED_ERROR(x) __memcached_string(c, ERROR)
#define MEMCACHED_UNKOWN(x) __memcached_string(c, UNKOWN)
#define MEMCACHED_END(x) __memcached_string(c, END)
#endif
