
include $(top_srcdir)/Makefile.config

lib_LTLIBRARIES = libcommon.la

libcommon_la_SOURCES = WordKey.cc DocumentDB.cc DocumentRef.cc WordList.cc \
		WordReference.cc defaults.cc 

libcommon_la_LDFLAGS = -release $(HTDIG_MAJOR_VERSION).$(HTDIG_MINOR_VERSION).$(HTDIG_MICRO_VERSION)

noinst_HEADERS = DocumentDB.h \
	DocumentRef.h \
	WordList.h \
	WordRecord.h \
	WordReference.h \
	WordKey.h \
	defaults.h

LOCAL_DEFINES=	-DBIN_DIR=\"$(bindir)\" \
		-DCOMMON_DIR=\"$(COMMON_DIR)\" \
		-DCONFIG_DIR=\"$(CONFIG_DIR)\" \
		-DDATABASE_DIR=\"$(DATABASE_DIR)\" \
		-DIMAGE_URL_PREFIX=\"$(IMAGE_URL_PREFIX)\" \
		-DPDF_PARSER=\"$(PDF_PARSER)\"

WordKey.h: WordKey.h.tmpl word.desc word_builder.pl
	@PERL@ word_builder.pl word.desc
