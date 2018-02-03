#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdarg.h>
#include<time.h>
#include<assert.h>

#include "log.h"

log* new_log(char* filename, log_level max_level) {
  assert(max_level <= L_FATAL && max_level >= L_INFO); 
  log* log_t = (log*)malloc(sizeof(log));
  assert(log_t != NULL);
  FILE* fp = fopen(filename, "ab");
  if (fp == NULL) {
    free(log_t);
    return NULL;
  }
  log_t->fp = fp;
  log_t->max_level = max_level;
  return log_t;
}

void destroy_log(log* log_t) {
  if(log_t!=NULL) {
    fflush(log_t->fp);
    fclose(log_t->fp);
    free(log_t);
    log_t = NULL;
  }
}

int write_log(log* log_t, int type, const char *fmt, ...) {
  assert(log_t != NULL);
  va_list args;
  char buf[1024] = {0};    
  time_t t = time(NULL);
  char* st = ctime(&t);
  char* stype;
  switch(type) {
    case L_DEBUG:
      stype = "INFO";
      break; 
    case L_INFO:
      stype = "INFO";
      break; 
    case L_ERROR:
      stype = "ERROR";
      break; 
    case L_WARN:
      stype = "WARN";
      break; 
    case L_TRACE:
      stype = "TRACE";
      break; 
    case L_NOTICE:
      stype = "NOTICE";
      break; 
    case L_FATAL:
      stype = "FATAL";
      break;
    default:
      return 0;
  }

  if (type > log_t->max_level) {
    return 0;
  }
  va_start(args,fmt);
  vsnprintf(buf, sizeof(buf),fmt,args);
#ifdef DEBUG
  return printf("[%.*s] %s: %s\n", (int)(strlen(st) - 1), st, stype, buf);
#else
  int n = fprintf(log_t->fp, "[%.*s] %s: %s\n", (int)(strlen(st) - 1), st, stype, buf);
  fflush(log_t->fp);
  return n;
#endif
}

#ifdef TEST 
int main(void) {
  log* log_t = new_log("1.log", L_FATAL);
  write_log(log_t,L_INFO,"%s,%d","12340999999", 1000);
  write_log(log_t,L_INFO,"%s,%d","12340999999", 1000);
  write_log(log_t,L_INFO,"%s,%d","12340999999", 1000);
  destroy_log(log_t);
}
#endif

