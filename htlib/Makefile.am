include $(top_srcdir)/Makefile.config

pkglib_LTLIBRARIES = libht.la

libht_la_SOURCES = Configuration.cc Database.cc Dictionary.cc \
	DB2_db.cc IntObject.cc List.cc Object.cc \
	ParsedString.cc Queue.cc QuotedStringList.cc Stack.cc \
	String.cc StringList.cc StringMatch.cc String_fmt.cc \
	good_strtok.cc strcasecmp.cc \
	mktime.c strptime.cc timegm.c \
	getcwd.c memcpy.c memmove.c raise.c strerror.c \
	HtCodec.cc HtWordCodec.cc \
	HtVector.cc HtHeap.cc \
	HtPack.cc HtDateTime.cc HtRegex.cc HtRegexList.cc \
	HtRegexReplace.cc HtRegexReplaceList.cc \
	HtVectorGeneric.cc HtMaxMin.cc HtWordType.cc \
	myqsort.c \
	md5.cc mhash_md5.c 

libht_la_LIBADD=@LTLIBOBJS@
# this includes regex.c

libht_la_LDFLAGS = -release $(HTDIG_MAJOR_VERSION).$(HTDIG_MINOR_VERSION).$(HTDIG_MICRO_VERSION) ${extra_ldflags}

pkginclude_HEADERS = \
	clib.h \
	Configuration.h \
	DB2_db.h \
	Database.h \
	Dictionary.h \
	HtCodec.h \
	HtDateTime.h \
	HtHeap.h \
	HtPack.h \
	HtRegex.h \
	HtRegexList.h \
	HtRegexReplace.h \
	HtRegexReplaceList.h \
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
	gregex.h  \
	HtVectorGeneric.h   \
	HtVectorGenericCode.h \
	HtVector_int.h \
	HtVector_String.h \
	HtMaxMin.h \
	HtTime.h \
	HtWordType.h \
	HtRandom.h \
	ber.h \
	libdefs.h \
	myqsort.h \
	mhash_md5.h \
	md5.h


