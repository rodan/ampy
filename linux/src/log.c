
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

int log_write(const char *fmt, ...)
{
    va_list ap;
    char *str;
    int fd;

    va_start(ap, fmt);
    if ( vasprintf(&str, fmt, ap) < 0 ) {
        return EXIT_FAILURE;
    }
    va_end(ap);

    fd = open(DEBUG_FILE, O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR);
    if ( write(fd, str, strlen(str)) < 0 ) {
        return EXIT_FAILURE;
    }
    close(fd);

    return EXIT_SUCCESS;
}


