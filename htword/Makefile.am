
include $(top_srcdir)/Makefile.config

EXTRA_DIST = word.desc word_builder.pl WordKey.h.tmpl

lib_LTLIBRARIES = libhtword.la

libhtword_la_SOURCES = WordKey.cc WordList.cc WordReference.cc WordType.cc \
	WordDB.cc WordStat.cc

libhtword_la_LDFLAGS = -release $(HTDIG_MAJOR_VERSION).$(HTDIG_MINOR_VERSION).$(HTDIG_MICRO_VERSION)

include_HEADERS = WordList.h \
	WordRecord.h \
	WordReference.h \
	WordType.h \
	WordKey.h \
	WordDB.h \
	WordStat.h

WordKey.h: $(top_srcdir)/htword/WordKey.h.tmpl $(top_srcdir)/htword/word.desc $(top_srcdir)/htword/word_builder.pl
	@PERL@ $(top_srcdir)/htword/word_builder.pl @KEYDESC@
