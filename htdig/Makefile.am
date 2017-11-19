
include $(top_srcdir)/Makefile.config

bin_PROGRAMS = htdig

htdig_SOURCES = Document.cc HTML.cc \
	Parsable.cc Plaintext.cc \
	Retriever.cc Server.cc ExternalTransport.cc \
	URLRef.cc htdig.cc ExternalParser.cc

noinst_HEADERS = Document.h ExternalParser.h HTML.h \
	Parsable.h Plaintext.h Retriever.h Server.h  URLRef.h htdig.h \
	ExternalTransport.h

PACKAGE = @PACKAGE@
VERSION = @VERSION@

ACLOCAL_AMFLAGS = -I m4

top_srcdir = @top_srcdir@
subdir = @top_srcdir@

datadir = @datadir@
datarootdir= @datarootdir@
localedir = @localedir@
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@
LIBS = @LIBINTL@ @LIBS@

htdig_DEPENDENCIES = $(HTLIBS)
htdig_LDFLAGS = $(PROFILING) ${extra_ldflags}
htdig_LDADD = $(HTLIBS) @LIBINTL@

distdir = ../$(PACKAGE)-$(VERSION)/$(subdir)
dist: Makefile $(DISTFILES)
	for file in $(DISTFILES); do \
	  ln $$file $(distdir) 2>/dev/null || cp -p $$file $(distdir) || exit 1; \
	done
