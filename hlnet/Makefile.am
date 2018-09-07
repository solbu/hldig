
include $(top_srcdir)/Makefile.config

pkglib_LTLIBRARIES = libhlnet.la

libhlnet_la_SOURCES =  \
	Connection.cc \
	HtCookie.cc \
	HtCookieInFileJar.cc \
	HtCookieJar.cc \
	HtCookieMemJar.cc \
	HtFTP.cc \
	HtFile.cc \
	HtHTTP.cc \
	HtHTTPBasic.cc \
	HtHTTPSecure.cc \
	HtNNTP.cc \
	SSLConnection.cc \
	Transport.cc

libhlnet_la_LDFLAGS = -release $(HLDIG_MAJOR_VERSION).$(HLDIG_MINOR_VERSION).$(HLDIG_MICRO_VERSION) ${extra_ldflags}

pkginclude_HEADERS = \
	Connection.h \
	Transport.h \
	HtHTTP.h \
	HtFile.h \
	HtFTP.h \
	HtNNTP.h \
	HtCookie.h \
	HtCookieJar.h \
	HtCookieMemJar.h \
	HtCookieInFileJar.h \
	HtHTTPBasic.h \
	HtHTTPSecure.h \
	SSLConnection.h
