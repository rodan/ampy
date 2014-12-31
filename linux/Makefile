
CC      = gcc
#CFLAGS  = -g -Wall -O2
CFLAGS  = -g -Wall -ggdb
LDFLAGS = -lncurses -lpanel -lm 
PROGS   = ampy_mixer

ampy_mixer_OBJS    = cli.o ampy_mixer.o pga2311_helper.o lm4780_helper.o colors.o widget.o mem.o utils.o mixer_display.o mixer_controls.o mixer_widget.o textbox.o die.o mainloop.o
ampy_mixer_EXTRA   = ampy_mixer.h

SRCS   = ampy_mixer.c cli.c pga2311_helper.c lm4780_helper.c colors.c widget.c mem.c utils.c mixer_display.c mixer_controls.c mixer_widget.c textbox.c die.c mainloop.c
EXTRA  = ../AUTHORS ../COPYING ../ChangeLog \
			../INSTALL ../NEWS ../README \
			Makefile

all: ${PROGS} tags

.c.o:
	$(CC) -c $(CFLAGS) $*.c

ampy_mixer : $(ampy_mixer_OBJS) $(ampy_mixer_EXTRA)
	$(CC) -o ampy_mixer $(ampy_mixer_OBJS) $(LIBS) $(LDFLAGS)

tags : $(ampy_mixer_OBJS) $(ampy_mixer_EXTRA)
	exuberant-ctags -h ".h.c" *

clean :
	rm -f *.o *.sibling *.combine $(PROGS)

strip : $(ampy_mixer_OBJS) $(ampy_mixer_EXTRA)
	strip ${PROGS}


