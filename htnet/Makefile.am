
include $(top_srcdir)/Makefile.config

pkglib_LTLIBRARIES = libhtnet.la

libhtnet_la_SOURCES = Connection.cc Transport.cc HtHTTP.cc HtFile.cc \
	HtNNTP.cc HtFTP.cc HtCookie.cc HtCookieJar.cc HtCookieMemJar.cc \
	HtCookieInFileJar.cc HtHTTPBasic.cc HtHTTPSecure.cc SSLConnection.cc

libhtnet_la_LDFLAGS = -release $(HLDIG_MAJOR_VERSION).$(HLDIG_MINOR_VERSION).$(HLDIG_MICRO_VERSION) ${extra_ldflags}

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
