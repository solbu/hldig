
include $(top_srcdir)/Makefile.config

bin_PROGRAMS = htdig

htdig_SOURCES = Document.cc HTML.cc Images.cc \
	Parsable.cc Plaintext.cc \
	Retriever.cc Server.cc ExternalTransport.cc \
	URLRef.cc htdig.cc ExternalParser.cc PDF.cc

noinst_HEADERS = Document.h ExternalParser.h HTML.h Images.h PDF.h \
	Parsable.h Plaintext.h Retriever.h Server.h  URLRef.h htdig.h \
	ExternalTransport.h
htdig_DEPENDENCIES = $(HTLIBS)
htdig_LDFLAGS = $(PROFILING)
htdig_LDADD = $(HTLIBS)
