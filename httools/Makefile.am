
include $(top_srcdir)/Makefile.config

LOCAL_DEFINES=  -DSENDMAIL=\"$(SENDMAIL)\"

bin_PROGRAMS = htpurge htnotify htdump htstat htload htmerge

htpurge_SOURCES = htpurge.cc
htpurge_DEPENDENCIES = $(HTLIBS)
htpurge_LDFLAGS = $(PROFILING)
htpurge_LDADD = $(HTLIBS)

htnotify_SOURCES = htnotify.cc
htnotify_DEPENDENCIES = $(HTLIBS)
htnotify_LDFLAGS = $(PROFILING)
htnotify_LDADD = $(HTLIBS)

htdump_SOURCES = htdump.cc
htdump_DEPENDENCIES = $(HTLIBS)
htdump_LDFLAGS = $(PROFILING)
htdump_LDADD = $(HTLIBS)

htstat_SOURCES = htstat.cc
htstat_DEPENDENCIES = $(HTLIBS)
htstat_LDFLAGS = $(PROFILING)
htstat_LDADD = $(HTLIBS)

htload_SOURCES = htload.cc
htload_DEPENDENCIES = $(HTLIBS)
htload_LDFLAGS = $(PROFILING)
htload_LDADD = $(HTLIBS)

htmerge_SOURCES = htmerge.cc
htmerge_DEPENDENCIES = $(HTLIBS)
htmerge_LDFLAGS = $(PROFILING)
htmerge_LDADD = $(HTLIBS)