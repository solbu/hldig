
include $(top_srcdir)/Makefile.config

lib_LTLIBRARIES = libfuzzy.la

libfuzzy_la_SOURCES = Endings.cc EndingsDB.cc Exact.cc \
		Fuzzy.cc Metaphone.cc Soundex.cc \
		SuffixEntry.cc Synonym.cc htfuzzy.cc \
		Substring.cc Prefix.cc Regex.cc Speling.cc

libfuzzy_la_LDFLAGS = -release $(HTDIG_MAJOR_VERSION).$(HTDIG_MINOR_VERSION).$(HTDIG_MICRO_VERSION)

noinst_HEADERS = Endings.h \
	Exact.h \
	Fuzzy.h \
	Metaphone.h \
	Prefix.h \
	Regex.h \
	Soundex.h \
	Speling.h \
	Substring.h \
	SuffixEntry.h \
	Synonym.h \
	htfuzzy.h

bin_PROGRAMS = htfuzzy

htfuzzy_SOURCES = htfuzzy.cc
htfuzzy_DEPENDENCIES = libfuzzy.la $(HTLIBS)
htfuzzy_LDADD = libfuzzy.la $(HTLIBS)
