include $(top_srcdir)/Makefile.config

#
# To run lengthy tests set the LONG_TEST='y' environment variable
#
# To run individual tests use (for instance):
# MAKE=make t_htdig
#
# To turn on verbosity us (for instance):
# MAKE=make t_htdig -vv
#
# To purify objects use:
# ( cd ../test ; rm -f word ; make CXXLD='purify g++' word )
# ( cd ../htmerge ; rm -f htmerge ; make CXXLD='purify g++' htmerge )
# ( cd ../htsearch ; rm -f htsearch ; make CXXLD='purify g++' htsearch )
# ( cd ../htdig ; rm -f htdig ; make CXXLD='purify g++' htdig )
#
TESTS = t_htdig t_htsearch t_htmerge t_wordkey t_wordlist # t_htnet
TESTS_ENVIRONMENT = $(top_srcdir)/test/test_prepare
AM_MAKEFLAGS = MAKE="$(MAKE)"

#PROFILING = -p

EXTRA_DIST = test_prepare htdocs conf $(TESTS) \
	word.try1 word.try2 word.try3 word.try4 word.try5

LOCAL_DEFINES = -I$(top_builddir)/db/include -I$(top_srcdir)/db/include $(PROFILING)

noinst_PROGRAMS = dbbench word testnet document

dbbench_SOURCES = dbbench.cc
dbbench_DEPENDENCIES = $(HTLIBS)
dbbench_LDFLAGS = $(PROFILING)
dbbench_LDADD = $(HTLIBS)

word_SOURCES = word.cc
word_DEPENDENCIES = $(HTLIBS)
word_LDFLAGS = $(PROFILING)
word_LDADD = $(HTLIBS)

testnet_SOURCES = testnet.cc
testnet_DEPENDENCIES = $(HTLIBS)
testnet_LDFLAGS = $(PROFILING)
testnet_LDADD = $(HTLIBS)

document_SOURCES = document.cc
document_DEPENDENCIES = $(HTLIBS)
document_LDFLAGS = $(PROFILING)
document_LDADD = $(HTLIBS)

clean-local:
	rm -fr gmon.out test test_weakcmpr logs var htdocs/tmp
	test -d conf && cd conf && $(MAKE) clean

distclean-local:
	rm -fr words.all words.uniq

dobench: dbbench
	$(MAKE) bench
	$(MAKE) CMPR='' bench

BASE = test
CACHESIZE = -C `expr 64 \* 1024 \* 1024`
PAGESIZE = -S 8192
CMPR = -z
WORDS = -w words.all
LOOP = -l 3
NWORDS = 

bench:
	rm -f $(BASE) $(BASE)_weakcmpr
	$(TIMEV) $(top_builddir)/test/dbbench $(CACHESIZE) $(PAGESIZE) $(CMPR) $(WORDS) $(LOOP) -B $(BASE) $(NWORDS)
	ls -l $(BASE)
	if [ -f $(BASE)_weakcmpr ] ; then $(top_builddir)/db/dist/db_dump -p $(BASE)_weakcmpr ; fi
	$(top_builddir)/db/dist/db_stat $(CMPR) -d $(BASE)

words:
	if [ -d /usr/info ] ; then root=/usr ; else root=/usr/share ; fi ; \
	find $$root/info -name '*.gz' -print | xargs zcat | perl -n -e 'print join("\n", map { lc } grep(length() > 2 && length() < 32, m/[a-z]+/ig)) . "\n"' | grep -v '^$$' > words.all ; \
	find $$root/man -type f -print | perl -n -e 'print join("\n", map { lc } grep(length() > 2 && length() < 32, m/[a-z]+/ig)) . "\n"' | grep -v '^$$' >> words.all 
	sort -u < words.all > words.uniq

.PHONY: words
