
include $(top_srcdir)/Makefile.config

bin_PROGRAMS = htdig

htdig_SOURCES = Document.cc HTML.cc Images.cc HtHTTP.cc Transport.cc \
	Parsable.cc Plaintext.cc \
	Retriever.cc Server.cc \
	URLRef.cc htdig.cc ExternalParser.cc PDF.cc
noinst_HEADERS = Document.h ExternalParser.h HTML.h HtHTTP.h Images.h PDF.h \
	Parsable.h Plaintext.h Retriever.h Server.h Transport.h URLRef.h htdig.h
htdig_DEPENDENCIES = $(HTLIBS)
htdig_LDADD = $(HTLIBS)
