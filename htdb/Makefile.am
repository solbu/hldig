include $(top_srcdir)/Makefile.config

bin_PROGRAMS = htload htdump htstat

htload_SOURCES = htload.cc getlong.cc
htload_DEPENDENCIES = $(HTLIBS)
htload_LDFLAGS = $(PROFILING)
htload_LDADD = $(HTLIBS)

htdump_SOURCES = htdump.cc
htdump_DEPENDENCIES = $(HTLIBS)
htdump_LDFLAGS = $(PROFILING)
htdump_LDADD = $(HTLIBS)

htstat_SOURCES = htstat.cc
htstat_DEPENDENCIES = $(HTLIBS)
htdump_LDFLAGS = $(PROFILING)
htstat_LDADD = $(HTLIBS)
