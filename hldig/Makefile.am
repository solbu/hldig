
include $(top_srcdir)/Makefile.config

bin_PROGRAMS = hldig

hldig_SOURCES = Document.cc HTML.cc \
	Parsable.cc Plaintext.cc \
	Retriever.cc Server.cc ExternalTransport.cc \
	URLRef.cc hldig.cc ExternalParser.cc

noinst_HEADERS = Document.h ExternalParser.h HTML.h \
	Parsable.h Plaintext.h Retriever.h Server.h  URLRef.h hldig.h \
	ExternalTransport.h

ACLOCAL_AMFLAGS = -I m4

hldig_DEPENDENCIES = $(HTLIBS)
hldig_LDFLAGS = $(PROFILING) ${extra_ldflags}
