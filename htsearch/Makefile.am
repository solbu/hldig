
include $(top_srcdir)/Makefile.config

bindir = $(CGIBIN_DIR)

LOCAL_DEFINES = -DCONFIG_DIR=\"$(CONFIG_DIR)\" -I$(top_srcdir)/htfuzzy

bin_PROGRAMS = htsearch qtest

htsearch_SOURCES = Display.cc DocMatch.cc ResultList.cc ResultMatch.cc \
		Template.cc TemplateList.cc WeightWord.cc htsearch.cc \
		parser.cc Collection.cc SplitMatches.cc HtURLSeedScore.cc 
noinst_HEADERS = Display.h DocMatch.h ResultList.h ResultMatch.h \
	Template.h TemplateList.h WeightWord.h htsearch.h parser.h \
	Collection.h SplitMatches.h HtURLSeedScore.h \
	WordSearcher.h AndQuery.h AndQueryParser.h BooleanLexer.h \
	BooleanQueryParser.h ExactWordQuery.h FuzzyExpander.h GParser.h \
	NearQuery.h NotQuery.h OperatorQuery.h OrFuzzyExpander.h \
	OrQuery.h OrQueryParser.h PhraseQuery.h Query.h QueryCache.h \
	QueryLexer.h QueryParser.h SimpleLexer.h SimpleQueryParser.h \
	VolatileCache.h
htsearch_DEPENDENCIES = $(top_builddir)/htfuzzy/libfuzzy.la $(HTLIBS)
htsearch_LDFLAGS = $(PROFILING) ${extra_ldflags}
htsearch_LDADD = $(top_builddir)/htfuzzy/libfuzzy.la $(HTLIBS)

qtest_SOURCES = DocMatch.cc ResultList.cc AndQuery.cc \
	BooleanLexer.cc BooleanQueryParser.cc ExactWordQuery.cc \
	GParser.cc NearQuery.cc NotQuery.cc OperatorQuery.cc \
	OrFuzzyExpander.cc OrQuery.cc PhraseQuery.cc Query.cc \
	QueryLexer.cc QueryParser.cc SimpleQueryParser.cc VolatileCache.cc \
	WordSearcher.cc qtest.cc
qtest_DEPENDENCIES = $(top_builddir)/htfuzzy/libfuzzy.la $(HTLIBS)
qtest_LDFLAGS = $(PROFILING) ${extra_ldflags}
qtest_LDADD = $(top_builddir)/htfuzzy/libfuzzy.la $(HTLIBS)

