#ifndef LOG_H

#define LOG_H

typedef enum _log_level {
  L_INFO,
  L_DEBUG,
  L_ERROR,
  L_WARN,
  L_TRACE,
  L_NOTICE,
  L_FATAL,
} log_level;

typedef struct _log {
  FILE* fp;
  log_level max_level;
} log;

log* new_log(char*, log_level);
void destroy_log(log*);
int write_log(log*,int,const char*,...);

#endif
