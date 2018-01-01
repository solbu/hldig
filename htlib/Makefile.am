# include $(top_srcdir)/Makefile.config
AUTOMAKE_OPTIONS = foreign no-dependencies

PACKAGE = @PACKAGE@
VERSION = @VERSION@

AM_CPPFLAGS=-pedantic	\
		-DDEFAULT_CONFIG_FILE=\"$(DEFAULT_CONFIG_FILE)\" \
		-I$(top_srcdir)/include -I$(top_srcdir)/htlib \
		-I$(top_srcdir)/htnet -I$(top_srcdir)/htcommon \
		-I$(top_srcdir)/htword \
		-I$(top_srcdir)/db -I$(top_builddir)/db \
		$(LOCAL_DEFINES) $(PROFILING) \
		-DLOCALEDIR=\"$(localedir)\"

HTLIBS=		$(top_builddir)/htnet/libhtnet.la \
		$(top_builddir)/htcommon/libcommon.la \
		$(top_builddir)/htword/libhtword.la \
		$(top_builddir)/htlib/libht.la \
		$(top_builddir)/db/libhtdb.la

localedir = @localedir@
LIBS = @LIBINTL@ @LIBS@

LDADD = $(HTLIBS) @LIBINTL@

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

libht_la_LDFLAGS = -release $(HLDIG_MAJOR_VERSION).$(HLDIG_MINOR_VERSION).$(HLDIG_MICRO_VERSION) ${extra_ldflags}

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
