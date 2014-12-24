
CC      = gcc
CFLAGS  = -g -Wall -O2
LDFLAGS = -lncurses -lpanel 
PROGS   = amp_mixer

amp_mixer_OBJS    = cli.o amp_mixer.o pga2311_helper.o colors.o widget.o
amp_mixer_EXTRA   = amp_mixer.h

SRCS   = amp_mixer.c cli.c pga2311_helper.c colors.c widget.c
EXTRA  = ../AUTHORS ../COPYING ../ChangeLog \
			../INSTALL ../NEWS ../README \
			Makefile

all: ${PROGS} tags

.c.o:
	$(CC) -c $(CFLAGS) $*.c

amp_mixer : $(amp_mixer_OBJS) $(amp_mixer_EXTRA)
	$(CC) -o amp_mixer $(amp_mixer_OBJS) $(LIBS) $(LDFLAGS)

tags : $(amp_mixer_OBJS) $(amp_mixer_EXTRA)
	exuberant-ctags -h ".h.c" *

clean :
	rm -f *.o *.sibling *.combine $(PROGS)

strip : $(amp_mixer_OBJS) $(amp_mixer_EXTRA)
	strip ${PROGS}


