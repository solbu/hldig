
include $(top_srcdir)/Makefile.config

bin_PROGRAMS = htmerge

htmerge_SOURCES = db.cc docs.cc htmerge.cc words.cc 
noinst_HEADERS = htmerge.h
htmerge_DEPENDENCIES = $(HTLIBS)
htmerge_LDADD = $(HTLIBS)
