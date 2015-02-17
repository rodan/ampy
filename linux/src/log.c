
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "log.h"

int log_init()
{
    char debug_file[DEBUG_FILE_MAX];
    //char fd_proc[DEBUG_FILE_MAX];

    memset(debug_file,0,sizeof(debug_file));
    strncpy(debug_file, DEBUG_FILE, DEBUG_FILE_MAX);
    fd_debug = mkstemp(debug_file);

    if (fd_debug > -1 ) {
        snprintf(fname_debug, DEBUG_FILE_MAX, debug_file);
        //fcntl(fd_debug, F_GETPATH, fname_debug);
        //snprintf(fd_proc, DEBUG_FILE_MAX, "/proc/self/fd/%d", fd_debug);
        //readlink(fd_proc, fname_debug, DEBUG_FILE_MAX);
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}

void remove_char(char *str, char garbage) {

    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != garbage) dst++;
    }
    *dst = '\0';
}

int log_write(const char *fmt, ...)
{
    va_list ap;
    char *str;

    if (fd_debug > -1) {
        va_start(ap, fmt);
        if ( vasprintf(&str, fmt, ap) < 0 ) {
            return EXIT_FAILURE;
        }
        va_end(ap);

        lseek(fd_debug, 0, SEEK_END);

        remove_char(str, 0x0d);
        if ( write(fd_debug, str, strlen(str)) < 0 ) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}


