#ifndef SERVER
#define SERVER
typedef struct _server {
  char *host;
  char *logfile;
  int log_max_level;
  int listenfd;
  int max_children;
  int running_children;
  int running;
  int timeout;
  pid_t pid;
  char* pidfile;
  int daemon;
  char *dbpath;
  int max_files;
} server;

typedef enum _server_opt {
    SERVER_TIMEOUT,
    SERVER_DAEMON,
    SERVER_MAX_CHILDREN,
    SERVER_LOGFILE,
    SERVER_PIDFILE,
    SERVER_LOG_MAX_LEVEL,
    SERVER_DBPATH,
    SERVER_MAX_FILES,
} server_opt;

server* new_server(char*);
int server_set_option(server*,server_opt,void*);
const void* server_get_option(server*,server_opt);
void destroy_server(server*);
void write_pid_file(char*,pid_t);
int daemonize();
#endif
