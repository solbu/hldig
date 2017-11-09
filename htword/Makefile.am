#
# Part of the ht://Dig package   <http://www.htdig.org/>
# Copyright (c) 1999-2004 The ht://Dig Group
# For copyright details, see the file COPYING in your distribution
# or the GNU Library General Public License version 2 or later
# <http://www.gnu.org/copyleft/lgpl.html>
#
include $(top_srcdir)/Makefile.config

LOCAL_DEFINES = 

pkglib_LTLIBRARIES = libhtword.la

libhtword_la_SOURCES = \
	WordBitCompress.cc \
	WordContext.cc \
	WordCursor.cc \
	WordDB.cc \
	WordDBCompress.cc \
	WordDBInfo.cc \
	WordDBPage.cc \
	WordKey.cc \
	WordKeyInfo.cc \
	WordList.cc \
	WordMonitor.cc \
	WordRecord.cc \
	WordRecordInfo.cc \
	WordReference.cc \
	WordStat.cc \
	WordType.cc

libhtword_la_LDFLAGS = -release $(HTDIG_MAJOR_VERSION).$(HTDIG_MINOR_VERSION).$(HTDIG_MICRO_VERSION) ${extra_ldflags}

pkginclude_HEADERS = \
	WordBitCompress.h \
	WordCaseIsAStatements.h \
	WordContext.h \
	WordCursor.h \
	WordDB.h \
	WordDBCompress.h \
	WordDBInfo.h \
	WordDBPage.h \
	WordKey.h \
	WordKeyInfo.h \
	WordList.h \
	WordMonitor.h \
	WordRecord.h \
	WordRecordInfo.h \
	WordReference.h \
	WordStat.h \
	WordType.h
