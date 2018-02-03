#ifndef CLIENT
#define CLIENT
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include "item.h"

typedef struct _client {
    int fd;
    item* it;    
    int resp;
    size_t itbytes;
    struct bufferevent *buf_ev;
} client;

client* new_client(); 
void destroy_client(client*);
#endif
