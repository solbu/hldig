
include $(top_srcdir)/Makefile.config

LOCAL_DEFINES=  -DSENDMAIL=\"$(SENDMAIL)\"

bin_PROGRAMS = htpurge htnotify

htpurge_SOURCES = htpurge.cc
htpurge_DEPENDENCIES = $(HTLIBS)
htpurge_LDFLAGS = $(PROFILING)
htpurge_LDADD = $(HTLIBS)

htnotify_SOURCES = htnotify.cc
htnotify_DEPENDENCIES = $(HTLIBS)
htnotify_LDFLAGS = $(PROFILING)
htnotify_LDADD = $(HTLIBS)


