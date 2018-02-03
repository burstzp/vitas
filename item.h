#ifndef ITEM
#define ITEM
#include <stdlib.h>

typedef struct _item {
    int           rbytes;
    int           nbytes;     /* size of data */
    unsigned char nsuffix;    /* length of flags-and-length string */
    unsigned char nkey;       /* key length, w/terminating null and padding */
    void *end[];
} item;

#define ITEM_key(item) ((char*)&((item)->end[0]))
#define ITEM_suffix(item) ((char*) &((item)->end[0]) + (item)->nkey + 1)
#define ITEM_flags(item) ((char*) &((item)->end[0]) + (item)->nkey + 1 + (item)->nsuffix)
#define ITEM_data2(item) ((char*) &((item)->end[0]) + (item)->nkey + 1 + (item)->nsuffix)
#define ITEM_expired(item) ((char*) &((item)->end[0]) + (item)->nkey + 1 + (item)->nsuffix) + sizeof(int)
#define ITEM_data(item) ((char*) &((item)->end[0]) + (item)->nkey + 1 + (item)->nsuffix) + sizeof(int) * 2
#define ITEM_ntotal(item) (sizeof(struct _stritem) + (item)->nkey + 1 + (item)->nsuffix + (item)->nbytes)

item* new_item(size_t);
item* item_alloc(const char*, size_t, const int, int nbytes);
int item_alloc1(item**, char*, size_t*);//,const int,const int nbytes);
int item_alloc2(item**, const char*, size_t, const int, const int, int nbytes);//,const int,const int nbytes);
void item_set_data(item*,char*,size_t);
void free_item(item*);
#endif
