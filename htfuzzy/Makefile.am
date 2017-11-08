
include $(top_srcdir)/Makefile.config

LOCAL_DEFINES=  -DMV=\"$(MV)\"

pkglib_LTLIBRARIES = libfuzzy.la

libfuzzy_la_SOURCES = Endings.cc EndingsDB.cc Exact.cc \
		Fuzzy.cc Metaphone.cc Soundex.cc Accents.cc \
		SuffixEntry.cc Synonym.cc \
		Substring.cc Prefix.cc Regexp.cc Speling.cc

libfuzzy_la_LDFLAGS = -release $(HTDIG_MAJOR_VERSION).$(HTDIG_MINOR_VERSION).$(HTDIG_MICRO_VERSION) ${extra_ldflags}

noinst_HEADERS = Accents.h \
	Endings.h \
	Exact.h \
	Fuzzy.h \
	Metaphone.h \
	Prefix.h \
	Regexp.h \
	Soundex.h \
	Speling.h \
	Substring.h \
	SuffixEntry.h \
	Synonym.h \
	htfuzzy.h

bin_PROGRAMS = htfuzzy

htfuzzy_SOURCES = htfuzzy.cc
htfuzzy_DEPENDENCIES = libfuzzy.la $(HTLIBS)
htfuzzy_LDFLAGS = $(PROFILING) ${extra_ldflags}
htfuzzy_LDADD = libfuzzy.la $(HTLIBS)
