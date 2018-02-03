#ifndef SOCK
#define SOCK

int socket_create(char *);
int socket_set_non_blocking(int);
int writen(int, char*, size_t);
int readn(int, char*,size_t);
#endif
