#include "common.h"

server* new_server(char* host) {
  assert(host);
  server* server_t = (server*)calloc(1,sizeof(server));
  assert(server_t);
  server_t->host = host;
  server_t->running = 1;
  return server_t;
}

int server_set_option(server* server_t, server_opt opt, void* value) {
  assert(value != NULL);
  switch (opt) {
    case SERVER_TIMEOUT:
      server_t->timeout = *(int *)value;
      break;

    case SERVER_DAEMON:
      server_t->daemon = *(int *)value;

      break;

    case SERVER_LOGFILE:
      server_t->logfile = (char*)value;
      break;

    case SERVER_PIDFILE:
      server_t->pidfile = (char*)value;
      break;
    
    case SERVER_DBPATH:
      server_t->dbpath = (char*)value;
      break;

    case SERVER_MAX_CHILDREN:
      server_t->max_children = *(int*)value;
      break;

    case SERVER_LOG_MAX_LEVEL: 
      server_t->log_max_level = *(int*)value;
      break;
    
    case SERVER_MAX_FILES: 
      server_t->max_files = *(int*)value;
      break;

    default:
      return 0;
  }

  return 1;
}

const void* server_get_option(server* server_t, server_opt opt) {
  switch (opt) {
    case SERVER_TIMEOUT:
      return &server_t->timeout;//= *(int *)value;
    case SERVER_DBPATH:
      return server_t->dbpath;//= *(int *)value;
    case SERVER_PIDFILE:
      return server_t->pidfile;//= *(int *)value;
    case SERVER_LOG_MAX_LEVEL:
      return &server_t->log_max_level;//= *(int *)value;
    case SERVER_DAEMON:
      return &server_t->daemon;//= *(int *)value;
    case SERVER_LOGFILE:
      return server_t->logfile;//= (char*)value;
    case SERVER_MAX_CHILDREN:
      return &server_t->max_children;// = *(int*)value;
    case SERVER_MAX_FILES:
      return &server_t->max_files;// = *(int*)value;
    default:
      return NULL;
  }
}

void destroy_server(server* server_t) {
  if (server_t != NULL) {
    free(server_t);
    server_t = NULL;
  }
}

int daemonize() {
  pid_t pid;
  int i, fd;
  pid = fork();
  if (pid < 0) {
    return 0;
  }

  if (pid==0) {
    setsid();
  } else {
    exit(0);
  }

  umask(0);
  for (i=0; i < 3; i++) {
    close(i);
  }

  fd = open("/dev/null", O_RDWR);
  if (dup2(fd, 0) < 0 || dup2(fd, 1) < 0 || dup2(fd, 2) < 0) {
    close(fd);
    return 0;
  }
  close(fd);
  return 0;
}

void write_pid_file(char* pidfile, pid_t pid)
{
  FILE* fp = fopen(pidfile,"w+");
  if(fp==NULL) {
    fprintf(stderr,"pid file cannot open error '%s'", strerror(errno));
    return;
  }

  fprintf(fp,"%d",pid);
  fclose(fp);
}

#ifdef TEST
int main(void) { 
  server* server_t = new_server("127.0.0.1:6666");
  server_set_option(server_t, SERVER_LOGFILE, "2.LOG");
  server_set_option(server_t, SERVER_PIDFILE, "/tmp/vitas.pid");
  int max_children = 4;
  server_set_option(server_t, SERVER_MAX_CHILDREN, &max_children);
  printf("logfile = %s\n", server_get_option(server_t, SERVER_LOGFILE));
  printf("maxchildren = %d\n", *(int *)server_get_option(server_t, SERVER_MAX_CHILDREN));
  destroy_server(server_t);
  return 0;
}
#endif
