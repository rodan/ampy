#ifndef __LOG_H__
#define __LOG_H__

#define DEBUG_FILE_MAX 32
#define DEBUG_FILE "/tmp/ampy-mixer_XXXXXX"

int fd_debug;
char fname_debug[DEBUG_FILE_MAX];

int log_init();
int log_write(const char *fmt, ...);

#endif
