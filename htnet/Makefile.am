
include $(top_srcdir)/Makefile.config

pkglib_LTLIBRARIES = libhtnet.la

libhtnet_la_SOURCES = Connection.cc Transport.cc HtHTTP.cc HtFile.cc HtNNTP.cc

libhtnet_la_LDFLAGS = -release $(HTDIG_MAJOR_VERSION).$(HTDIG_MINOR_VERSION).$(HTDIG_MICRO_VERSION)

pkginclude_HEADERS = \
	Connection.h \
	Transport.h \
	HtHTTP.h \
	HtFile.h \
	HtNNTP.h 
