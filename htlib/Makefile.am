
include $(top_srcdir)/Makefile.config

lib_LTLIBRARIES = libht.la

libht_la_SOURCES = Configuration.cc Database.cc Dictionary.cc \
	DB2_db.cc IntObject.cc List.cc Object.cc \
	ParsedString.cc Queue.cc QuotedStringList.cc Stack.cc \
	String.cc StringList.cc StringMatch.cc String_fmt.cc \
	URL.cc URLTrans.cc cgi.cc good_strtok.cc io.cc strcasecmp.cc \
	mktime.c strptime.c timegm.c HtCodec.cc HtWordCodec.cc \
	HtURLCodec.cc HtVector.cc HtHeap.cc regex.c HtSGMLCodec.cc \
	HtPack.cc HtDateTime.cc HtRegex.cc HtZlibCodec.cc

libht_la_LDFLAGS = -release $(HTDIG_MAJOR_VERSION).$(HTDIG_MINOR_VERSION).$(HTDIG_MICRO_VERSION)

noinst_HEADERS = \
	Configuration.h \
	DB2_db.h \
	Database.h \
	Dictionary.h \
	HtCodec.h \
	HtDateTime.h \
	HtHeap.h \
	HtPack.h \
	HtRegex.h \
	HtSGMLCodec.h \
	HtURLCodec.h \
	HtVector.h \
	HtWordCodec.h \
	HtZlibCodec.h \
	IntObject.h \
	List.h \
	Object.h \
	ParsedString.h \
	Queue.h \
	QuotedStringList.h \
	Stack.h \
	StringList.h \
	StringMatch.h \
	URL.h \
	cgi.h \
	good_strtok.h \
	htString.h \
	io.h \
	lib.h \
	langinfo.h \
	regex.h
