include $(top_srcdir)/Makefile.config

#PROFILING = -p

EXTRA_DIST = index.html script.html site1.html site2.html site3.html site4.html title.html

LOCAL_DEFINES = -I$(top_builddir)/db/include -I$(top_srcdir)/db/include $(PROFILING)

noinst_PROGRAMS = dbbench

dbbench_SOURCES = dbbench.cc
dbbench_DEPENDENCIES = $(HTLIBS)
dbbench_LDFLAGS = $(PROFILING)
dbbench_LDADD = $(HTLIBS)

clean-local:
	rm -f gmon.out test test_weakcmpr

words:
	find /usr/info -name '*.gz' -print | xargs zcat | perl -n -e 'print join("\n", map { lc } grep(length() > 2 && length() < 32, m/[a-z]+/ig)) . "\n"' | grep -v '^$$' > words.all
	find /usr/man -type f -print | perl -n -e 'print join("\n", map { lc } grep(length() > 2 && length() < 32, m/[a-z]+/ig)) . "\n"' | grep -v '^$$' >> words.all
	sort -u < words.all > words.uniq
	find /usr/info -name '*.gz' -print | xargs zcat | dd of=/dev/null bs=1k
	find /usr/man -type f -print | xargs cat | dd of=/dev/null bs=1k
	find /usr/man /usr/info -type f -print | wc -l

.PHONY: words
