
include $(top_srcdir)/Makefile.config

bindir = $(CGIBIN_DIR)

LOCAL_DEFINES = -DCONFIG_DIR=\"$(CONFIG_DIR)\"

bin_PROGRAMS = htsearch

htsearch_SOURCES = Display.cc DocMatch.cc ResultList.cc ResultMatch.cc \
		Template.cc TemplateList.cc WeightWord.cc htsearch.cc \
		Searcher.cc
noinst_HEADERS = Display.h DocMatch.h ResultList.h ResultMatch.h \
	Template.h TemplateList.h WeightWord.h htsearch.h Searcher.h
htsearch_DEPENDENCIES = ../htfuzzy/libfuzzy.la $(HTLIBS)
htsearch_LDFLAGS = $(PROFILING)
htsearch_LDADD = ../htfuzzy/libfuzzy.la $(HTLIBS)
