
include $(top_srcdir)/Makefile.config

bindir = $(CGIBIN_DIR)

LOCAL_DEFINES = -DCONFIG_DIR=\"$(CONFIG_DIR)\"

bin_PROGRAMS = htsearch parsetest

htsearch_SOURCES = Display.cc DocMatch.cc ResultList.cc ResultMatch.cc \
		Template.cc TemplateList.cc WeightWord.cc htsearch.cc \
		parser.cc Collection.cc SplitMatches.cc HtURLSeedScore.cc 
noinst_HEADERS = Display.h DocMatch.h ResultList.h ResultMatch.h \
	Template.h TemplateList.h WeightWord.h htsearch.h parser.h \
	Collection.h SplitMatches.h HtURLSeedScore.h \
	ParseTree.h AndParseTree.h OrParseTree.h ExactParseTree.h
htsearch_DEPENDENCIES = ../htfuzzy/libfuzzy.la $(HTLIBS)
htsearch_LDFLAGS = $(PROFILING)
htsearch_LDADD = ../htfuzzy/libfuzzy.la $(HTLIBS)

parsetest_SOURCES = ParseTree.cc AndParseTree.cc OrParseTree.cc \
	ExactParseTree.cc WeightWord.cc ResultList.cc \
	parsetest.cc
parsetest_DEPENDENCIES = $(HTLIBS)
parsetest_LDFLAGS = $(PROFILING)
parsetest_LDADD = $(HTLIBS)
