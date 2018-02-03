#include "common.h"
#include <errno.h>

typedef int memcachedCommandProc(client *c, void*);

struct memcachedCommand {
  char* name;
  memcachedCommandProc* proc;
  int arity;
  int len;
};

static struct memcachedCommand memcachedCmdTable[] = {
  {"get",memcached_get,2,3},
  {"set",memcached_set,5,3},
  {"delete",memcached_delete,2,6},
  {"quit",memcached_quit,1,4},
  {"stats",memcached_stats,2,5},
  {NULL,NULL,0}
};

static struct memcachedCommand *memcachedLookupCommand(char* name) 
{
  int j = 0;
  while (memcachedCmdTable[j].name != NULL) {
    if (!strncmp(memcachedCmdTable[j].name,name,memcachedCmdTable[j].len)) return &memcachedCmdTable[j];
    j++;
  }

  return NULL;
}

struct timeval tv; 
struct event *ev; 
void server_shutdown() {
  server_t->running = 0;
}

void init_config() {
  LOG(L_INFO, "reload config pid=%d,ppid=%d",getpid(),getppid());
}

void child_sig_handler(int signo) {
  event_base_loopbreak(evbase);
}

void parent_sig_handler(int signo) {
  if(signo == SIGHUP) {
    init_config();
  } else {
    signal(SIGQUIT, SIG_IGN);
    server_shutdown();
  }
}

void child_init() {
  signal(SIGPIPE, SIG_IGN);
  signal(SIGCHLD, SIG_IGN);
  signal(SIGTERM, child_sig_handler);
  signal(SIGINT, child_sig_handler);
  signal(SIGQUIT, child_sig_handler);
}

void
server_on_error(struct bufferevent *bev, short what, void *arg)
{
  client *c = (client *)arg;
  destroy_client(c);
}

int bufferevent_readn(struct bufferevent *bev, char *buf, size_t buf_sz)
{
  int n;
  do {  
    n = bufferevent_read(bev, buf, MAX_HEADER_SIZE);
  } while (n == -1);
  return n;
}
void server_on_write(struct bufferevent* bev, void* arg) {
  LOG(L_INFO, "server_on_wrtie");
 
  bufferevent_enable(bev, EV_READ);

}
/**
 * Called by libevent when there is data to read.
 */
void
server_on_read(struct bufferevent *bev, void *arg)
{
  LOG(L_INFO, "read bytes()");
  time_t st1 = clock();
  client *c = (client*)arg;
  if (c->it->nbytes > 0) {
    memcached_set(c, NULL);
  } else {
    LOG(L_INFO, "memcached protocol start");
    char header[MAX_HEADER_SIZE + 1] = {0};
    size_t n = bufferevent_readn(bev, header, MAX_HEADER_SIZE - 1);
    header[MAX_HEADER_SIZE] = '\0';
    struct memcachedCommand *cmd = memcachedLookupCommand(header);
    if (cmd == NULL) {
      LOG(L_INFO,"-1,header=%s",header)
      MEMCACHED_UNKOWN(c);
    } else if (!cmd->proc(c, header)) {
      LOG(L_INFO,"-2,header=%s",header)
      MEMCACHED_ERROR(c);
    }
    LOG(L_INFO,"here")
  }

  long cost_time = clock() - st1;
  if (cost_time > 10000)
    LOG(L_INFO, "[SLOW] costtime = %lu", clock() - st1);
}

void
server_on_accept(evutil_socket_t fd, short ev, void *arg)
{
  int client_fd;
  struct sockaddr_in client_addr;
  socklen_t client_len  = sizeof(client_addr);
  
  client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_len);
  if (client_fd < 0) {
    LOG(L_ERROR, "module=%s, %s socket_accept_error",
        "server_on_accept", strerror(errno));
    return;
  }

  if (socket_set_non_blocking(client_fd) < 0) {
    LOG(L_ERROR, "module=%s, client_fd=%d, socket_set_non_blocking_error",
        "server_on_accept", client_fd);
    return;
  }

  client* c = new_client();
  struct event_base *base = (struct event_base *)arg;
  if (c == NULL) {
    LOG(L_ERROR, "new_client() fail!");
    return;
  }

  c->fd = client_fd;
  c->buf_ev = bufferevent_socket_new(base, client_fd, BEV_OPT_CLOSE_ON_FREE);
  bufferevent_setcb(c->buf_ev, server_on_read, server_on_write,
      server_on_error, c);

  bufferevent_enable(c->buf_ev, EV_READ|EV_WRITE);
}

void worker_run() {
  child_init();
  event_reinit(evbase);
  struct event ev_accept;
  event_assign(&ev_accept, 
      evbase, 
      server_t->listenfd, 
      EV_WRITE|EV_READ|EV_PERSIST,
      server_on_accept, 
      evbase);
  event_add(&ev_accept, NULL);
  event_base_dispatch(evbase);
}

void parent_init() {
  struct sigaction act;
  act.sa_handler = parent_sig_handler; 
  sigemptyset(&act.sa_mask); 
#ifdef SA_INTERRUPT
  act.sa_flags = SA_INTERRUPT;
#else
  act.sa_flags = 0;
#endif
  signal(SIGPIPE, SIG_IGN);
  sigaction(SIGTERM, &act, NULL);
  sigaction(SIGINT, &act, NULL);
  sigaction(SIGQUIT, &act, NULL);
  signal(SIGHUP, parent_sig_handler); 
  if (server_t->pidfile) {
    write_pid_file(server_t->pidfile, getpid());
  }
}

int start_workers() {
  int i = 0;
  pid_t pid;
  server_t->running_children = 0;
  while(i++ < server_t->max_children && (pid=fork())) {
    server_t->running_children++;
  }

  if (pid) {
    return 1;	
  } else {
    return 0;
  }
}

void server_destroy() {
  destroy_log(log_t);
  destroy_server(server_t);
}

int server_dispatch() {
  int is_parent = start_workers();

  if (is_parent) {
    parent_init();
    while(server_t->running) {
      int status;
      pid_t pid,cid;
      if (((pid = waitpid(-1,&status,0))>0)) {
        cid = fork();
        if (cid == 0) {
          goto worker;
        }	
      }
    }

    pid_t cid;
    int status; 
    signal(SIGQUIT, SIG_IGN);
    kill(-(server_t->pid), SIGQUIT);
    while(server_t->running_children) {
      while((cid = waitpid(-1, &status, 0))>0) {
        --server_t->running_children;
        LOG(L_INFO, "child shutdown, with status = %d", status);
      }
    }

    LOG(L_INFO, "server shutdown");
    event_base_free(evbase);
    destroy_db(sdb_t);
    return 1;
  }

worker:    
  worker_run();
  return 0;
}

void time_cb(int fd,short _event,void *argc)  
{  
  db_cleanup();
  evtimer_add(ev, &tv);/*重新添加定时器*/  
}  

void server_init() 
{
  log_t = new_log(server_t->logfile, L_FATAL);  
  
  if (log_t == NULL) {
    fprintf(stderr, "new_log() fail!\n");
    server_destroy();
    exit(1);
  }
  
  sdb_t = new_db();
  if (sdb_t == NULL) {
    LOG(L_FATAL, "new_db() fail!");
    server_destroy();
    exit(1);
  }

  const char *db = server_get_option(server_t, SERVER_DBPATH);
  int max_files = *(int *)server_get_option(server_t, SERVER_MAX_FILES);
  if (!db_init(db, max_files)) {
    LOG(L_FATAL, "open db error");
    server_destroy();
    exit(1);
  }

  evbase = event_base_new();
  int sfd = socket_create(server_t->host);  
  if (sfd == -1) {
    LOG(L_FATAL, "socket_create() fail!");
    server_destroy();
    exit(1);
  }
  
  tv.tv_sec = 5; //间隔  
  tv.tv_usec = 0;  
  ev = evtimer_new(evbase,time_cb,NULL);//初始化关注的事件，并设置回调函数  
  event_add(ev,&tv);

  server_t->listenfd = sfd;
  if (socket_set_non_blocking(sfd) < -1) {
    server_destroy();
    exit(1);
  } 
  
  if (*(int *)server_get_option(server_t, SERVER_DAEMON) == 1) {
    daemonize();
  }
}
void server_use_age(char *argv0)
{
  char * prog = strrchr(argv0, '/');
	if (prog) {
		prog++;
	} else {
		prog = "vitas";
	}

	printf( "Usage: %s -s <host>:<port> -n <workers number> -l <log file> -p <pid file> -d <run at daemon> -D <storage path> -m <max files>\n"
			, prog );
}

void loop(int argc, char** argv) 
{
  int opt, max_childs = 1;
	char *hostname = NULL, *pid_file = NULL, *log_file = NULL;
	char *user = NULL, *group = NULL;
	char *dbpath = NULL;
	int max_files = 1024;
	int isdaemon = 0;

	while ((opt = getopt(argc, argv, "hs:n:du:g:l:p:D:m")) != -1) {
		switch (opt) {
			case 'n':
				//max_childs = atoi(optarg);
				break;
			case 's':
				hostname = optarg;
				break;
			case 'p':
				pid_file = optarg;
				break;
			case 'd':
				isdaemon = 1;
				break;
			case 'l':
				log_file = optarg;
				break;
			case 'u':
				user = optarg;
				break;
			case 'g':
				group = optarg;
				break;
			case 'D':
				dbpath = optarg;
				break;
			case 'm':
				max_files = atoi(optarg);
				break;
			default:
			  server_use_age(argv[0]);
				return ;
		}
	}

	if (!hostname || !log_file || !pid_file || !dbpath) {
		server_use_age(argv[0]);
		return ;
	}	

  server_t = new_server(hostname);
	if (server_t == NULL) {
    fprintf(stderr, "new_server() fail!\n");
    return ;
	}

  server_set_option(server_t, SERVER_LOGFILE, log_file);
  server_set_option(server_t, SERVER_PIDFILE, pid_file);
  server_set_option(server_t, SERVER_MAX_CHILDREN, &max_childs);
  server_set_option(server_t, SERVER_DAEMON, &isdaemon);
  server_set_option(server_t, SERVER_DBPATH, dbpath);
  server_set_option(server_t, SERVER_MAX_FILES, &max_files);
  server_init();
  server_dispatch();
  server_destroy();		
}

int main(int argc, char** argv) { 
  loop(argc, argv);
  return 0;
}
