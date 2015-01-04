
bin_PROGRAMS = ampy_mixer

ampy_mixer_SOURCES =  ampy_mixer.c colors.c lm4780_helper.c mem.c mixer_display.c pga2311_helper.c textbox.c widget.c cli.c die.c mainloop.c mixer_controls.c mixer_widget.c sysdep1.c utils.c

noinst_HEADERS = ampy_mixer.h colors.h lm4780_helper.h mem.h mixer_display.h pga2311_helper.h sysdep.h textbox.h version.h cli.h die.h mainloop.h mixer_controls.h mixer_widget.h proj.h sysdep1.h utils.h widget.h

AM_CPPFLAGS = @CPPFLAGS@ -I$(top_srcdir)/lib -DCONFDIR=\"$(confdir)\"

