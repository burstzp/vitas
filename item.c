#include "item.h"
#include "common.h"

item* 
new_item(size_t sz) 
{
  item* item_t = (item*)calloc(sizeof(item) + sz, 1);
  if (item_t == NULL) {
    return NULL;
  }

  return item_t;
}

int 
item_alloc1(item** it, char* tk, size_t* n) 
{
  if (tk == NULL) {
    return 0;
  } 
  
  size_t i = 0;
  size_t j = 0;
  const char* tk_array[6] = {NULL,NULL,NULL,NULL,NULL,NULL};
  *n = 0;
  char* ttk = tk;
  while (*ttk !='\r' && *ttk !='\n') {
    ++*n;
    ttk++;
  }
  
  if (*n == 0) {
    return 0;
  }

  for (; i < *n; i++) {  
    if (tk_array[j] == NULL) {
      tk_array[j] = tk + i;
    } else if (tk[i] == ' ') {
      tk[i] = '\0';
      j++;
    }  
  }

  tk[*n] = '\0';
  if (tk_array[0] == NULL) {
    return 0;
  } 

  //*command = tk;
  if (tk_array[1] == NULL) {
    return 0;
  } 

  int flags = 0;
  if (tk_array[2] != NULL) {
    flags = atoi(tk_array[2]);
  }
  
  int expired = 0;
  if (tk_array[3] != NULL) {
    expired =  atoi(tk_array[3]);
    if (expired > 0) {
      expired += time(NULL);
    }
  }

  int nbytes = 0;
  if (tk_array[4] != NULL) {
    nbytes = atoi(tk_array[4]);
  }
  
  return item_alloc2(it, tk_array[1], strlen(tk_array[1]), flags, expired, nbytes);
}

int
item_alloc2(item** it, const char* key, size_t nkey, const int flags, const int expired, int nbytes)
{
  if ((*it) == NULL) {
    return 0;
  }

  size_t ntotal = nbytes + nkey + 1 + 2 * sizeof(int);
  if (ntotal > DATA_BUFFER) {
    item* tt = realloc(*it, ntotal);    
    if (tt == NULL) {
      return 0;
    }
    
    memset(tt, 0, ntotal);
    *it = tt;
  }

  (*it)->nkey = nkey; 
  memcpy(ITEM_key((*it)), key, nkey);
  (*it)->nbytes = nbytes;
  (*it)->rbytes = 0;
  memcpy(ITEM_flags((*it)), &flags, sizeof(int));
  memcpy(ITEM_expired((*it)), &expired, sizeof(int));
  int t = 0;
  memcpy(&t, ITEM_expired((*it)), sizeof(int));
  return 1;
}

item* 
item_alloc(const char* key,size_t nkey, const int flags, int nbytes) 
{
  item* item_t = new_item(nbytes + nkey);
  if (item_t == NULL) {
    return NULL;
  }

  item_t->nkey = nkey; 
  memcpy(ITEM_key(item_t), key, nkey);
  item_t->nbytes = nbytes;
  return item_t;
}

void 
free_item(item* it) 
{
  if (it != NULL) {
    free(it);
    it = NULL;
  }
}

void 
item_set_data(item* item_t, char* body, size_t bodylen) 
{
  if (item_t != NULL) {
    memcpy(ITEM_data(item_t), body, bodylen); 
  }
}

#ifdef TEST
int main(void) {
  char buf[16] = "get test 0 0 1";
  item *it = item_alloc1(buf,strlen(buf));//, 0, 1);
  printf("%s\n", ITEM_key(it));
  free_item(it);
  return 0;
}
#endif
