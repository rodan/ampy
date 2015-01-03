#ifndef __SYSDEP1_H__
#define __SYSDEP1_H__

void m_setparms(int fd, char *baudr, char *par, char *bits, char *stopb,
                int hwf, int swf);

void flush_fd(int fd);

#endif
