#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([ampy_mixer], [0.1], [petre.rodan@simplex.ro])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([src/ampy_mixer.c])

#AM_INIT_AUTOMAKE([1.7])

dnl Checks for programs.
#AC_PROG_CC
#AM_PROG_AR

AC_AIX
AC_MINIX

AC_PROG_INSTALL
dnl AC_PROG_LIBTOOL
LT_INIT
AC_CONFIG_MACRO_DIR([m4])

AC_ISC_POSIX

AC_ARG_ENABLE([socket],
	AS_HELP_STRING([--disable-socket],
	               [Disable socket support (def: enabled)]),
	[], [enable_socket="yes"])
if test "x$enable_socket" = xyes; then
	AC_DEFINE(USE_SOCKET, [1], [Socket support is enabled])
fi

# Some GNU/Linux systems (e.g., SuSE 4.3, 1996) don't have curses, but
# rather ncurses.  So we check for it.
TERMLIBS=
for termlib in tinfo ncurses curses termcap terminfo termlib ; do
   AC_CHECK_LIB(${termlib}, tputs,
     [TERMLIBS="${TERMLIBS} -l${termlib}"; break])
done

LIBS="$LIBS $TERMLIBS"

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lm':
AC_CHECK_LIB([m], [lrint])
# FIXME: Replace `main' with a function in `-lncurses':
AC_CHECK_LIB([ncurses], [newwin])
# FIXME: Replace `main' with a function in `-lpanel':
AC_CHECK_LIB([panel], [new_panel])

dnl Checks for header files.
dnl Do not use <ncurses/termcap.h> unless we're linking with ncurses.
if test "x$termlib" = xncurses; then
  dnl Use AC_CHECK_HEADERS so the HAVE_*_H symbol gets defined.
  AC_CHECK_HEADERS(ncurses/termcap.h, [found_termcap_h=1])
fi
if test "x$found_termcap_h" != "x1"; then
  AC_CHECK_HEADERS(termcap.h)
fi

AC_CHECK_LIB(socket, socket)

AC_SYS_POSIX_TERMIOS
AC_HEADER_TIOCGWINSZ
if test $ac_cv_sys_posix_termios = yes; then
  AC_DEFINE(POSIX_TERMIOS, [1], [Define if your system supports POSIX termios])
fi

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h inttypes.h limits.h sgtty.h stddef.h stdint.h stdlib.h string.h strings.h sys/ioctl.h sys/wait.h sgtty.h ioctl.h termio.h termios.h unistd.h wchar.h features.h])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_UINT8_T
AC_CHECK_HEADER_STDBOOL

dnl Checks for library functions.
AC_TYPE_SIGNAL
AC_FUNC_REALLOC
AC_CHECK_FUNCS([alarm select strdup strerror wcwidth])

dnl ---------------------------------------------------------------

AC_MSG_CHECKING(for default serial port device)

if test "x$DFL_PORT" != x
then
	if test -c $DFL_PORT
	then
		AC_MSG_RESULT($DFL_PORT)
		AC_DEFINE_UNQUOTED(DFL_PORT, "$DFL_PORT",
		                   [Default /dev/... device])
	else
		DFL_PORT=""
	fi
fi

if test "x$DFL_PORT" = x
then
	for ac_port in /dev/rfcomm0 /dev/rfcomm1 /dev/ttyUSB0 /dev/ttyUSB1; do
		DFL_PORT=$ac_port
		if test -c $DFL_PORT; then
			AC_MSG_RESULT($DFL_PORT)
			AC_DEFINE_UNQUOTED(DFL_PORT, "$DFL_PORT")
			break
		else
			DFL_PORT=""
		fi
	done
fi

if test "x$DFL_PORT" = x
then
	DFL_PORT=/dev/rfcomm0
	AC_MSG_RESULT($DFL_PORT)
	AC_DEFINE_UNQUOTED(DFL_PORT, "$DFL_PORT", [Default device])
fi

if test "x$ac_cv_header_getopt_h" = xno
then
	CFLAGS="$CFLAGS -I\$(top_srcdir)/lib"
fi

if test "x$ac_cv_prog_gcc" = xyes
then
	CFLAGS="$CFLAGS -W -Wall -Wextra -std=gnu99"
fi

# this is a hack, if we need getopt_long we also need getopt
# but I don't know how to express that dependency in lib/Makefile.am
AC_SUBST(LIBPORT_GETOPT_LONG_DEP)
if test "x$ac_cv_func_getopt_long" = xno
then
	LIBPORT_GETOPT_LONG_DEP="getopt.o"
fi

dnl ---------------------------------------------------------------

AH_BOTTOM([
/* Defines from old config.h */
#define LOGFILE "minicom.log"        /* Not defined = not used */
#define CALLOUT ""                   /* Gets run to get into dialut mode */
#define CALLIN  ""                   /* Gets run to get into dial in mode */

/* This defines a special mode in the wwxgetch() routine. The
 * basic idea behind this probably works on the consoles of
 * most PC-based unices, but it's only implemented for Linux.
 */
#if defined (__linux__)
#  define KEY_KLUDGE 1
#endif

/* And this for the translation tables (vt100 -> ASCII) */
#if __STDC__
#  define TRANSLATE 1
#else
#  define TRANSLATE 0
#endif
])

AC_OUTPUT
