
CC      = gcc
#CFLAGS  = -g -Wall -O2
CFLAGS  = -g -Wall -ggdb
LDFLAGS = -lncurses -lpanel -lm 
PROGS   = amp_mixer

amp_mixer_OBJS    = cli.o amp_mixer.o pga2311_helper.o colors.o widget.o mem.o utils.o mixer_display.o mixer_controls.o mixer_widget.o textbox.o die.o mainloop.o
amp_mixer_EXTRA   = amp_mixer.h

SRCS   = amp_mixer.c cli.c pga2311_helper.c colors.c widget.c mem.c utils.c mixer_display.c mixer_controls.c mixer_widget.c textbox.c die.c mainloop.c
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


