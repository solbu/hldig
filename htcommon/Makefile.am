
include $(top_srcdir)/Makefile.config

#
# Do not output #line because it confuses the dependencies
# generator of GCC if configure run out of source tree. 
# Comment these to ease debugging.
#
AM_LFLAGS = -L
AM_YFLAGS = -l -d

EXTRA_DIST=conf_lexer.cxx conf_parser.cxx

pkglib_LTLIBRARIES = libcommon.la

libcommon_la_SOURCES = DocumentDB.cc DocumentRef.cc \
	HtWordReference.cc HtWordList.cc defaults.cc \
	HtURLCodec.cc URL.cc URLTrans.cc \
	HtZlibCodec.cc cgi.cc HtSGMLCodec.cc \
	HtConfiguration.cc HtURLRewriter.cc \
	conf_lexer.lxx conf_parser.yxx 

libcommon_la_LDFLAGS = -release $(HTDIG_MAJOR_VERSION).$(HTDIG_MINOR_VERSION).$(HTDIG_MICRO_VERSION) ${extra_ldflags}

noinst_HEADERS = DocumentDB.h \
	DocumentRef.h \
	HtWordReference.h \
	HtWordList.h \
	HtURLCodec.h \
	HtSGMLCodec.h \
	URL.h \
	cgi.h \
	HtZlibCodec.h \
	defaults.h \
	HtConfiguration.h \
	HtURLRewriter.h \
	conf_parser.h

LOCAL_DEFINES=	-DBIN_DIR=\"$(bindir)\" \
		-DCOMMON_DIR=\"$(COMMON_DIR)\" \
		-DCONFIG_DIR=\"$(CONFIG_DIR)\" \
		-DDATABASE_DIR=\"$(DATABASE_DIR)\" \
		-DIMAGE_URL_PREFIX=\"$(IMAGE_URL_PREFIX)\" 

