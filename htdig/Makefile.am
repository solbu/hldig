
include $(top_srcdir)/Makefile.config

bin_PROGRAMS = htdig

htdig_SOURCES = Document.cc HTML.cc \
	Parsable.cc Plaintext.cc \
	Retriever.cc Server.cc ExternalTransport.cc \
	URLRef.cc htdig.cc ExternalParser.cc

noinst_HEADERS = Document.h ExternalParser.h HTML.h \
	Parsable.h Plaintext.h Retriever.h Server.h  URLRef.h htdig.h \
	ExternalTransport.h
htdig_DEPENDENCIES = $(HTLIBS)
htdig_LDFLAGS = $(PROFILING) ${extra_ldflags}
htdig_LDADD = $(HTLIBS)
