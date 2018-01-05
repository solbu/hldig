
include $(top_srcdir)/Makefile.config

LOCAL_DEFINES=  -DSENDMAIL=\"$(SENDMAIL)\"

bin_PROGRAMS = hlpurge hlnotify hldump hlstat hlload hlmerge

hlpurge_SOURCES = hlpurge.cc
hlpurge_DEPENDENCIES = $(HTLIBS)
hlpurge_LDFLAGS = $(PROFILING) ${extra_ldflags}
hlpurge_LDADD = $(HTLIBS)

hlnotify_SOURCES = hlnotify.cc
hlnotify_DEPENDENCIES = $(HTLIBS)
hlnotify_LDFLAGS = $(PROFILING) ${extra_ldflags}
hlnotify_LDADD = $(HTLIBS)

hldump_SOURCES = hldump.cc
hldump_DEPENDENCIES = $(HTLIBS)
hldump_LDFLAGS = $(PROFILING) ${extra_ldflags}
hldump_LDADD = $(HTLIBS)

hlstat_SOURCES = hlstat.cc
hlstat_DEPENDENCIES = $(HTLIBS)
hlstat_LDFLAGS = $(PROFILING) ${extra_ldflags}
hlstat_LDADD = $(HTLIBS)

hlload_SOURCES = hlload.cc
hlload_DEPENDENCIES = $(HTLIBS)
hlload_LDFLAGS = $(PROFILING) ${extra_ldflags}
hlload_LDADD = $(HTLIBS)

hlmerge_SOURCES = hlmerge.cc
hlmerge_DEPENDENCIES = $(HTLIBS)
hlmerge_LDFLAGS = $(PROFILING) ${extra_ldflags}
hlmerge_LDADD = $(HTLIBS)
