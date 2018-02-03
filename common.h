#ifndef COMMON
#define COMMON

#define TRUE 0
#define FALSE 1

#define DATA_BUFFER 20480
#define DEBUG 1

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<signal.h>
#include<string.h>
#include<assert.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<errno.h>
#include<arpa/inet.h>

#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include "log.h"
#include "server.h"
#include "sock.h"
#include "client.h"
#include "item.h"
#include "memcached_protocal.h"
#include "db.h"

#define MAX_HEADER_SIZE 256

//log* log_t;
server* server_t;

struct event_base *evbase;

log* log_t;

#ifdef DEBUG
#define LOG(level, fmt, ...)  printf("[%d] "fmt" %s", level, ##__VA_ARGS__, "\r\n"); 
#else
#define LOG(level, fmt, ...) write_log(log_t, level, fmt, ##__VA_ARGS__); 
#endif

#endif

