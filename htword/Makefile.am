
include $(top_srcdir)/Makefile.config

EXTRA_DIST = word.desc 

LOCAL_DEFINES = 

lib_LTLIBRARIES = libhtword.la

libhtword_la_SOURCES = WordKey.cc WordList.cc WordReference.cc WordType.cc \
	WordDB.cc WordStat.cc WordDBCompress.cc WordBitCompress.cc \
	WordRecord.cc WordMonitor.cc WordContext.cc WordKeyInfo.cc WordDBPage.cc

libhtword_la_LDFLAGS = -release $(HTDIG_MAJOR_VERSION).$(HTDIG_MINOR_VERSION).$(HTDIG_MICRO_VERSION)

include_HEADERS = WordList.h \
	WordRecord.h \
	WordReference.h \
	WordType.h \
	WordDBCompress.h \
	WordBitCompress.h \
	WordKey.h \
	WordDB.h \
	WordStat.h \
	WordCaseIsAStatements.h \
	WordMonitor.h\
	WordContext.h\
	WordKeyInfo.h \
	WordDBPage.h
