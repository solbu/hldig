include $(top_srcdir)/Makefile.config

bin_PROGRAMS = hldb_load hldb_dump hldb_stat

hldb_load_SOURCES = htdb_load.cc
hldb_load_DEPENDENCIES = $(HLLIBS)
hldb_load_LDFLAGS = $(PROFILING) ${extra_ldflags}
hldb_load_LDADD = $(HLLIBS)

hldb_dump_SOURCES = htdb_dump.cc
hldb_dump_DEPENDENCIES = $(HLLIBS)
hldb_dump_LDFLAGS = $(PROFILING) ${extra_ldflags}
hldb_dump_LDADD = $(HLLIBS)

hldb_stat_SOURCES = htdb_stat.cc
hldb_stat_DEPENDENCIES = $(HLLIBS)
hldb_stat_LDFLAGS = $(PROFILING) ${extra_ldflags}
hldb_stat_LDADD = $(HLLIBS)
