AUTOMAKE_OPTIONS = subdir-objects foreign no-dependencies

bin_PROGRAMS=simplecgen
AM_CFLAGS=	-Wall -std=c99 -g
simplecgen_SOURCES=		\
simplecgen.c	\
utils.c	\
template_functions.c

noinst_HEADERS =		\
simplecgen.h	\
utils.h	\
template_functions.h

PACKAGE = @PACKAGE@
VERSION = @VERSION@

sysconfdir = @sysconfdir@
DEFS = -DSYSCONFDIR=\"$(sysconfdir)\" @DEFS@

# These 4 lines are needed because of a bug in GNU autotools?
# https://github.com/andy5995/rmw/issues/21
ACLOCAL = true
AUTOCONF = true
AUTOHEADER = true
AUTOMAKE = true
