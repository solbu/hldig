
include $(top_srcdir)/Makefile.config

lib_LTLIBRARIES = libcommon.la

libcommon_la_SOURCES = DocumentDB.cc DocumentRef.cc defaults.cc 

libcommon_la_LDFLAGS = -release $(HTDIG_MAJOR_VERSION).$(HTDIG_MINOR_VERSION).$(HTDIG_MICRO_VERSION)

noinst_HEADERS = DocumentDB.h \
	DocumentRef.h \
	defaults.h

LOCAL_DEFINES=	-DBIN_DIR=\"$(bindir)\" \
		-DCOMMON_DIR=\"$(COMMON_DIR)\" \
		-DCONFIG_DIR=\"$(CONFIG_DIR)\" \
		-DDATABASE_DIR=\"$(DATABASE_DIR)\" \
		-DIMAGE_URL_PREFIX=\"$(IMAGE_URL_PREFIX)\" \
		-DPDF_PARSER=\"$(PDF_PARSER)\"
