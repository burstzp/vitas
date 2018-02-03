#include "item.h"
#include "memcached_protocal.h"
#include "client.h"
#include "common.h"

size_t 
__memcached_make_header(char* header, size_t sz, int flags, char* key, size_t nkey, size_t bodylen)
{
  return snprintf(header, sz, "VALUE %s %d %zu\r\n", key, flags, bodylen);
}

void 
__memcached_value(client* c, int flags, char* body, size_t bodylen) {
  char header[256] = {0};
  size_t n = __memcached_make_header(header, sizeof(header), flags, ITEM_key(c->it), c->it->nkey, bodylen); 
  bufferevent_write(c->buf_ev, header, strlen(header));
  bufferevent_write(c->buf_ev, body, bodylen);
  bufferevent_write(c->buf_ev, END, strlen(END));
}

void 
__memcached_stored(client* c) 
{
  writen(c->fd, STORED, strlen(STORED));
}

void 
__memcached_end(client* c) 
{
  bufferevent_write(c->buf_ev, "END\r\n", strlen("END\r\n"));
}

void 
__memcached_deleted(client* c) 
{
    bufferevent_write(c->buf_ev, DELETED, strlen(DELETED));
}

void 
__memcached_string(client* c, char* buf) 
{
  bufferevent_write(c->buf_ev, buf, strlen(buf));
}

int 
memcached_get(client* c, void* arg) 
{
  int expired = 0;
  char* header = (char*)arg;
  size_t kn = 0;
  memset(c->it, 0, c->itbytes);
  if (!item_alloc1(&c->it, header, &kn)) {  
    return 0;
  } 
  
  char* read = NULL;
  size_t read_len = 0;
  int iz = sizeof(int);
  int iz2 = 2 * iz;
  if (db_get(ITEM_key(c->it), c->it->nkey, &read, &read_len) && read != NULL && read_len > iz2) {
    int flags = 0, expired = 0;
    memcpy(&flags, read, iz);
    memcpy(&expired, read + iz, iz);
    
    if (expired == 0 || time(NULL) < expired) {
      __memcached_value(c, flags, read + iz2, read_len - iz2);
    } else {
      if (db_delete(ITEM_key(c->it), c->it->nkey)) {
        __memcached_end(c);
      } else {
        LOG(L_INFO, "db_delete() fail! key = %s", ITEM_key(c->it));   
      }
    }

    free(read);
  } else {
    __memcached_end(c);
  } 
  
  return 1;
}

int
memcached_set(client* c, void* arg)
{
  char* header = (char*)arg;
  size_t n =0;  
  if (c->it != NULL && c->it->rbytes > 0 && c->it->rbytes < c->it->nbytes) {
    
    int lb = c->it->nbytes - c->it->rbytes;  
    char* b = ITEM_data(c->it) + c->it->rbytes;
    do {  
      n = bufferevent_read(c->buf_ev, b, lb);
    } while (n == -1);
    c->it->rbytes += n;
    LOG(L_INFO, "nbytes=%d,rbytes=%d", c->it->nbytes, c->it->rbytes);
  } else if (c->it->rbytes == 0 ) {
    size_t kn = 0; 
    n = strlen(header);
    LOG(L_INFO, "header=%s,n=%zu", header, n);
    if (!item_alloc1(&c->it, header, &kn)) {  
      return 0;
    } 
    
    if (c->it->nbytes > 0 && c->it->nbytes < 1024000){
      kn += 2;
      item_set_data(c->it, header + kn, n - kn);
      size_t rb = n - kn - 2;// read numbers
      size_t lb = c->it->nbytes - rb;// + 2 is for read \r\n;
      if (lb == 0)  {
        c->it->rbytes = c->it->nbytes;
      } else {
        char* b = ITEM_data(c->it) + n - kn;
        LOG(L_INFO, "lb=%zu", lb);
        do {  
          n = bufferevent_read(c->buf_ev, b, lb);
        } while (n == -1);
        c->it->rbytes += rb + n;
      }
    } 
  }
  
  if (c->it->rbytes == c->it->nbytes) {
    int expired = 0;
    memcpy(&expired, ITEM_expired((c->it)), sizeof(int));
    if (db_put(ITEM_key(c->it), c->it->nkey, ITEM_data2(c->it), c->it->nbytes + sizeof(int)*2, expired)) {
      __memcached_stored(c);
      LOG(L_INFO, "stored OK");// ITEM_data(c->it));
      memset(c->it, 0, sizeof(item));
      return 1;
    } else {
      __memcached_end(c);
    } 

    memset(c->it, 0, sizeof(item));
  }

  return 1;
}

int
memcached_delete(client* c, void* arg)
{
  char* header = (char*)arg;
  size_t kn = 0;
  memset(c->it, 0, c->itbytes);
  if (!item_alloc1(&c->it, header, &kn)) {  
    return 0;
  } 
  
  if (db_delete(ITEM_key(c->it), c->it->nkey)) {
    __memcached_deleted(c);
  } else {
    __memcached_end(c);
  }

  return 1;
}

int 
memcached_quit(client* c, void* arg)
{
  destroy_client(c);
  return 0;
}

int 
memcached_stats(client* c, void* arg) {
  __memcached_string(c, STATS);
  return 0;
}
#ifdef TEST
int main() {
  return 0;
}
#endif
