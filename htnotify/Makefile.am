
include $(top_srcdir)/Makefile.config

LOCAL_DEFINES=  -DSENDMAIL=\"$(SENDMAIL)\"

bin_PROGRAMS = htnotify

htnotify_SOURCES = htnotify.cc
htnotify_DEPENDENCIES = $(HTLIBS)
htnotify_LDFLAGS = $(PROFILING)
htnotify_LDADD = $(HTLIBS)
