#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<string.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<err.h>
#include<errno.h>
#include<netdb.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/fcntl.h>
#include "assert.h"
#include "common.h"

int
socket_create(char* host)
{
  assert(host != NULL);
  struct sockaddr sa;
  int sockfd;
  if (host[0]=='/') {
    struct sockaddr_un *usa;
    usa = (struct sockaddr_un*)&sa;
    sockfd = socket(AF_UNIX,SOCK_STREAM,0);
    usa->sun_family = AF_UNIX;
    memcpy(usa->sun_path, host, strlen(host) + 1);
  } else {
    char *pt;
    struct sockaddr_in* listen_addr;
    pt = strchr(host, ':');
    assert(pt);
    int port = atoi(pt + 1);
    char hostname[128];
    strncpy(hostname, host, pt - host);
    hostname[pt - host] = '\0';
    struct hostent *hptr;
    hptr = gethostbyname(hostname);
    if (hptr == NULL) {
      fprintf(stderr, "failed to resolve host name '%s'", strerror(errno));
      return -1;
    }
    char* p = hptr->h_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
      fprintf(stderr, "listen failed.\n");
      return -1;
    }
    listen_addr = (struct sockaddr_in*)&sa;
    memset(listen_addr, 0, sizeof(*listen_addr));
    listen_addr->sin_family = AF_INET;
    listen_addr->sin_port = htons(port);
    memcpy(&listen_addr->sin_addr, p, sizeof(struct in_addr));
  }

  int reuse_addr_on = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr_on, sizeof(reuse_addr_on)) < 0) {
    fprintf(stderr, "setsocketopt error '%s'\n", strerror(errno));
    return -1;
  }
  if (bind(sockfd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
    fprintf(stderr, "bind faild, error '%s'", strerror(errno));
    close(sockfd);
    return -1;
  }

  if (listen(sockfd,SOMAXCONN)<0) {
    fprintf(stderr, "listend failed, error '%s'", strerror(errno));
    close(sockfd);
    return -1;
  }

  return sockfd;
}
int 
socket_set_non_blocking(int fd) 
{
  int flags;
  flags = fcntl(fd, F_GETFL);
  if (flags < 0) {
    return flags;
  }

  flags |= O_NONBLOCK;
  if (fcntl(fd, F_SETFL, flags) < 0) {
    return -1;
  }

  return 0;
}
int readn(int fd, char* buf, size_t sz) {
  size_t read_bytes = 0;
  do {
    read_bytes = recv(fd, buf, sz, 0);
  } while (read_bytes == -1 && errno == EINTR);
  if (read_bytes == 0) {
    return -1;
  }

  return 0;
}
int writen(int fd, char* payload, size_t sz) {
  int bytes_sent = 0;
  int total_sent = 0;
  do {
    if ((bytes_sent = send(fd, payload + total_sent, sz - total_sent, 0)) > 0) {
      total_sent += bytes_sent;
    }
  } while (bytes_sent == -1 && errno == EINTR);
  
  if (total_sent < sz) {
    return 1;
  } else {
    return 0;
  }
}

#ifdef TEST
int main(void) {
  int sockfd = socket_create("127.0.0.1:16789");
  printf("sockfd = %d\n", sockfd);
  return 0;
}
#endif

