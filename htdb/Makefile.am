include $(top_srcdir)/Makefile.config

bin_PROGRAMS = htdb_load htdb_dump htdb_stat

htdb_load_SOURCES = htdb_load.cc err.c getlong.c
htdb_load_DEPENDENCIES = $(HTLIBS)
htdb_load_LDFLAGS = $(PROFILING)
htdb_load_LDADD = $(HTLIBS)

htdb_dump_SOURCES = htdb_dump.cc err.c
htdb_dump_DEPENDENCIES = $(HTLIBS)
htdb_dump_LDFLAGS = $(PROFILING)
htdb_dump_LDADD = $(HTLIBS)

htdb_stat_SOURCES = htdb_stat.cc err.c
htdb_stat_DEPENDENCIES = $(HTLIBS)
htdb_stat_LDFLAGS = $(PROFILING)
htdb_stat_LDADD = $(HTLIBS)
