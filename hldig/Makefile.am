
include $(top_srcdir)/Makefile.config

bin_PROGRAMS = hldig

hldig_SOURCES = Document.cc HTML.cc \
	Parsable.cc Plaintext.cc \
	Retriever.cc Server.cc ExternalTransport.cc \
	URLRef.cc hldig.cc ExternalParser.cc

noinst_HEADERS = Document.h ExternalParser.h HTML.h \
	Parsable.h Plaintext.h Retriever.h Server.h  URLRef.h hldig.h \
	ExternalTransport.h

PACKAGE = @PACKAGE@
VERSION = @VERSION@

ACLOCAL_AMFLAGS = -I m4

top_srcdir = @top_srcdir@
subdir = @top_srcdir@

datadir = @datadir@
datarootdir= @datarootdir@
localedir = @localedir@
LIBS = @LIBINTL@ @LIBS@

hldig_DEPENDENCIES = $(HTLIBS)
hldig_LDFLAGS = $(PROFILING) ${extra_ldflags}
hldig_LDADD = $(HTLIBS) @LIBINTL@
