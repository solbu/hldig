#
# Part of the ht://Dig package   <http://www.htdig.org/>
# Copyright (c) 1999, 2000 The ht://Dig Group
# For copyright details, see the file COPYING in your distribution
# or the GNU General Public License version 2 or later
# <http://www.gnu.org/copyleft/gpl.html>
#
include $(top_srcdir)/Makefile.config

bin_PROGRAMS = htdb_load htdb_dump htdb_stat mifluzdump mifluzdict mifluzload

htdb_load_SOURCES = htdb_load.cc util_sig.cc
htdb_load_DEPENDENCIES = $(HTLIBS)
htdb_load_LDFLAGS = $(PROFILING)
htdb_load_LDADD = $(HTLIBS)

htdb_dump_SOURCES = htdb_dump.cc util_sig.cc
htdb_dump_DEPENDENCIES = $(HTLIBS)
htdb_dump_LDFLAGS = $(PROFILING)
htdb_dump_LDADD = $(HTLIBS)

htdb_stat_SOURCES = htdb_stat.cc util_sig.cc
htdb_stat_DEPENDENCIES = $(HTLIBS)
htdb_stat_LDFLAGS = $(PROFILING)
htdb_stat_LDADD = $(HTLIBS)

mifluzdump_SOURCES = mifluzdump.cc
mifluzdump_DEPENDENCIES = $(HTLIBS)
mifluzdump_LDFLAGS = $(PROFILING)
mifluzdump_LDADD = $(HTLIBS)

mifluzdict_SOURCES = mifluzdict.cc
mifluzdict_DEPENDENCIES = $(HTLIBS)
mifluzdict_LDFLAGS = $(PROFILING)
mifluzdict_LDADD = $(HTLIBS)

mifluzload_SOURCES = mifluzload.cc
mifluzload_DEPENDENCIES = $(HTLIBS)
mifluzload_LDFLAGS = $(PROFILING)
mifluzload_LDADD = $(HTLIBS)

noinst_HEADERS = util_sig.h

diff:
	rm -fr 1.tmp ; mkdir 1.tmp
	cp *.cc 1.tmp
	cd 1.tmp ; \
	perl -pi -e 's/CDB_//g' *.cc ; \
	for i in stat dump load ; do diff -c ../../../db-3.0.55/*/db_$$i.c htdb_$$i.cc  ; done
