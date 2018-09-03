
include $(top_srcdir)/Makefile.config

LOCAL_DEFINES=  -DSENDMAIL=\"$(SENDMAIL)\"

bin_PROGRAMS = hlpurge hlnotify hldump hlstat hlload hlmerge

hlpurge_SOURCES = hlpurge.cc
hlpurge_DEPENDENCIES = $(HLLIBS)
hlpurge_LDFLAGS = $(PROFILING) ${extra_ldflags}
hlpurge_LDADD = $(HLLIBS)

hlnotify_SOURCES = hlnotify.cc
hlnotify_DEPENDENCIES = $(HLLIBS)
hlnotify_LDFLAGS = $(PROFILING) ${extra_ldflags}
hlnotify_LDADD = $(HLLIBS)

hldump_SOURCES = hldump.cc
hldump_DEPENDENCIES = $(HLLIBS)
hldump_LDFLAGS = $(PROFILING) ${extra_ldflags}
hldump_LDADD = $(HLLIBS)

hlstat_SOURCES = hlstat.cc
hlstat_DEPENDENCIES = $(HLLIBS)
hlstat_LDFLAGS = $(PROFILING) ${extra_ldflags}
hlstat_LDADD = $(HLLIBS)

hlload_SOURCES = hlload.cc
hlload_DEPENDENCIES = $(HLLIBS)
hlload_LDFLAGS = $(PROFILING) ${extra_ldflags}
hlload_LDADD = $(HLLIBS)

hlmerge_SOURCES = hlmerge.cc
hlmerge_DEPENDENCIES = $(HLLIBS)
hlmerge_LDFLAGS = $(PROFILING) ${extra_ldflags}
hlmerge_LDADD = $(HLLIBS)
