#
# Part of the ht://Dig package   <http://www.htdig.org/>
# Copyright (c) 1999, 2000 The ht://Dig Group
# For copyright details, see the file COPYING in your distribution
# or the GNU General Public License version 2 or later
# <http://www.gnu.org/copyleft/gpl.html>
#
include $(top_srcdir)/Makefile.config

LOCAL_DEFINES = 

pkglib_LTLIBRARIES = libhtword.la

libhtword_la_SOURCES = \
	WordBitCompress.cc \
	WordContext.cc \
	WordCursorOne.cc \
	WordDB.cc \
	WordDBCache.cc \
	WordDBCompress.cc \
	WordDBInfo.cc \
	WordDead.cc \
	WordDict.cc \
	WordKey.cc \
	WordKeyInfo.cc \
	WordList.cc \
	WordListOne.cc \
	WordListMulti.cc \
	WordMeta.cc \
	WordMonitor.cc \
	WordRecord.cc \
	WordRecordInfo.cc \
	WordReference.cc \
	WordType.cc

libhtword_la_LDFLAGS = -release $(HTDIG_MAJOR_VERSION).$(HTDIG_MINOR_VERSION).$(HTDIG_MICRO_VERSION)

pkginclude_HEADERS = \
	WordBitCompress.h \
	WordContext.h \
	WordCursor.h \
	WordCursorOne.h \
	WordDB.h \
	WordDBCache.h \
	WordDBCompress.h \
	WordDBInfo.h \
	WordDead.h \
	WordDict.h \
	WordKey.h \
	WordKeyInfo.h \
	WordList.h \
	WordListOne.h \
	WordListMulti.h \
	WordMeta.h \
	WordMonitor.h \
	WordRecord.h \
	WordRecordInfo.h \
	WordReference.h \
	WordType.h \
