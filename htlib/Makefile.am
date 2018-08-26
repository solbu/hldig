include $(top_srcdir)/Makefile.config

pkglib_LTLIBRARIES = libht.la

libht_la_SOURCES = \
	Configuration.cc \
	Database.cc \
	DB2_db.cc \
	Dictionary.cc \
	getcwd.c \
	good_strtok.cc \
	HtCodec.cc \
	HtDateTime.cc \
	HtHeap.cc \
	HtMaxMin.cc \
	HtPack.cc \
	HtRegex.cc \
	HtRegexList.cc \
	HtRegexReplace.cc \
	HtRegexReplaceList.cc \
	HtVector.cc \
	HtVectorGeneric.cc \
	HtWordCodec.cc \
	HtWordType.cc \
	IntObject.cc \
	List.cc \
	md5.cc \
	memcpy.c \
	memmove.c \
	mhash_md5.c \
	mktime.c \
	myqsort.c \
	Object.cc \
	ParsedString.cc \
	Queue.cc \
	QuotedStringList.cc \
	raise.c \
	Stack.cc \
	strcasecmp.cc \
	strerror.c \
	String.cc \
	String_fmt.cc \
	StringList.cc \
	StringMatch.cc \
	strptime.cc \
	timegm.c

libht_la_LIBADD=@LTLIBOBJS@
# this includes regex.c

libht_la_LDFLAGS = -release $(HLDIG_MAJOR_VERSION).$(HLDIG_MINOR_VERSION).$(HLDIG_MICRO_VERSION) ${extra_ldflags}

pkginclude_HEADERS = \
	ber.h \
	clib.h \
	Configuration.h \
	Database.h \
	DB2_db.h \
	Dictionary.h \
	filecopy.h \
	good_strtok.h \
	gregex.h  \
	HtCodec.h \
	HtDateTime.h \
	HtHeap.h \
	HtMaxMin.h \
	HtPack.h \
	HtRandom.h \
	HtRegex.h \
	HtRegexList.h \
	HtRegexReplace.h \
	HtRegexReplaceList.h \
	htString.h \
	HtTime.h \
	HtVector.h \
	HtVector_int.h \
	HtVector_String.h \
	HtVectorGeneric.h   \
	HtVectorGenericCode.h \
	HtWordCodec.h \
	HtWordType.h \
	IntObject.h \
	lib.h \
	libdefs.h \
	List.h \
	md5.h \
	mhash_md5.h \
	myqsort.h \
	Object.h \
	ParsedString.h \
	Queue.h \
	QuotedStringList.h \
	Stack.h \
	StringList.h \
	StringMatch.h
