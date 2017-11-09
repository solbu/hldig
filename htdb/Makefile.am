include $(top_srcdir)/Makefile.config

bin_PROGRAMS = htdb_load htdb_dump htdb_stat

htdb_load_SOURCES = htdb_load.cc
htdb_load_DEPENDENCIES = $(HTLIBS)
htdb_load_LDFLAGS = $(PROFILING) ${extra_ldflags}
htdb_load_LDADD = $(HTLIBS)

htdb_dump_SOURCES = htdb_dump.cc
htdb_dump_DEPENDENCIES = $(HTLIBS)
htdb_dump_LDFLAGS = $(PROFILING) ${extra_ldflags}
htdb_dump_LDADD = $(HTLIBS)

htdb_stat_SOURCES = htdb_stat.cc
htdb_stat_DEPENDENCIES = $(HTLIBS)
htdb_stat_LDFLAGS = $(PROFILING) ${extra_ldflags}
htdb_stat_LDADD = $(HTLIBS)
