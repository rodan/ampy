#ifndef __PROJ_H__
#define __PROJ_H__

#include <inttypes.h>

char *stty_device;
int fd_device;

extern int portfd_is_socket;
extern int portfd_is_connected;

extern int debug;

#endif
