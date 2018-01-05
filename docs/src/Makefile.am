bin_PROGRAMS=simplecgen
AM_CFLAGS= -std=c99 -Wall -pedantic -g
simplecgen_SOURCES=			simplecgen.c template_functions.c

noinst_HEADERS = 				template_functions.h

PACKAGE = @PACKAGE@
VERSION = @VERSION@

sysconfdir = @sysconfdir@
DEFS = -DSYSCONFDIR=\"$(sysconfdir)\" @DEFS@
