#include "client.h"
#include "item.h"
#include "common.h"

client* new_client() {
  client* client_t = (client*) calloc(1, sizeof(client));
  if (client_t == NULL) {
    return NULL;
  }
  
  item* it = new_item(DATA_BUFFER);
  if (it == NULL) {
    free(client_t);
    return NULL;
  }

  client_t->it = it;
  client_t->itbytes = DATA_BUFFER;
  return client_t;
} 

void destroy_client(client* client_t) {
  if (client_t != NULL) {
    close(client_t->fd);
    bufferevent_free(client_t->buf_ev);
    free_item(client_t->it);
    free(client_t);
  } 
}
