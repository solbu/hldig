CFLAGS=-g
include $(top_srcdir)/Makefile.config

lib_LTLIBRARIES = libht.la

libht_la_SOURCES = Configuration.cc Database.cc Dictionary.cc \
	DB2_db.cc IntObject.cc List.cc Object.cc \
	ParsedString.cc Queue.cc QuotedStringList.cc Stack.cc \
	String.cc StringList.cc StringMatch.cc String_fmt.cc \
	good_strtok.cc strcasecmp.cc \
	mktime.c strptime.c timegm.c HtCodec.cc HtWordCodec.cc \
	HtVector.cc HtHeap.cc regex.c \
	HtPack.cc HtDateTime.cc HtRegex.cc \
	conf.l.cc conf.tab.cc

libht_la_LDFLAGS = -release $(HTDIG_MAJOR_VERSION).$(HTDIG_MINOR_VERSION).$(HTDIG_MICRO_VERSION)

include_HEADERS = \
	Configuration.h \
	DB2_db.h \
	Database.h \
	Dictionary.h \
	HtCodec.h \
	HtDateTime.h \
	HtHeap.h \
	HtPack.h \
	HtRegex.h \
	HtVector.h \
	HtWordCodec.h \
	IntObject.h \
	List.h \
	Object.h \
	ParsedString.h \
	Queue.h \
	QuotedStringList.h \
	Stack.h \
	StringList.h \
	StringMatch.h \
	good_strtok.h \
	htString.h \
	lib.h \
	langinfo.h \
	regex.h

conf.l.cc:	$(srcdir)/conf.l $(srcdir)/conf.tab.cc.h
	flex -o$(srcdir)/conf.l.cc $(srcdir)/conf.l
conf.tab.cc:	$(srcdir)/conf.y
	bison -d -o $(srcdir)/conf.tab.cc $(srcdir)/conf.y
conf.tab.cc.h: conf.tab.cc
