#libhtdig.so makefile for Unix systems

LIBHTDIG_BUILD_VER = 3.2.1


#Berkeley DB Specific defines 
BDB_INC_DIRS = -I../db -I/usr/local/include
BDB_DEFS = -DHAVE_CONFIG_H

INC_DIRS= -I. -I../htdig -I../include -I../db -I../htlib -I../htnet -I../htcommon -I../htword -I../htfuzzy -I../htsearch -I/usr/local/include

#HTDIG_INC_DIR= -I../htdig

#define your own defaults here!
DEFAULT_CONFIG_FILE=\"/etc/htdig/htdig.conf\"
DEFAULT_DB_PATH=\"/etc/htdig/\"
BIN_DIR=\"/usr/local/bin\"
COMMON_DIR=\"/usr/local/share/htdig\"
CONFIG_DIR=\"/etc/htdig\"
DATABASE_DIR=\"/var/lib/htdig\"
IMAGE_URL_PREFIX=\"/htdig\"

DEFS= -DHAVE_CONFIG_H -DDEFAULT_CONFIG_FILE=$(DEFAULT_CONFIG_FILE) -DBIN_DIR=$(BIN_DIR) -DCOMMON_DIR=$(COMMON_DIR) 
DEFS+= -DCONFIG_DIR=$(CONFIG_DIR) -DDATABASE_DIR=$(DATABASE_DIR) -DIMAGE_URL_PREFIX=$(IMAGE_URL_PREFIX)

#LEX define based on your system
LEX = flex
AM_LFLAGS = -L
LEX_OUTPUT_ROOT = lex.yy

#YACC define based on your system
YACC = bison -y
AM_YFLAGS = -l -d
YACC_OUTPUT_ROOT = y.tab




ifdef INSURE
CC=insure gcc
CXX=insure g++
else
CC=gcc
CXX=g++
endif


#OPTIMZ=-O2
OPTIMZ=-O0
#OPTS=$(OPTIMZ) -Wall -fno-rtti -fno-exceptions -Werror
OPTS=$(OPTIMZ) -Wall -fno-rtti -fno-exceptions -fPIC
DBG= -g $(PROFILING)

ifdef FULLDEBUG
DBG+= -DDEBUG -DDEBUG_CMPR $(INC_DIRS)
endif

ifdef DEBUG
DBG+= -DDEBUG
endif

ifdef EXPKEY
OPTS += -DEXPKEY
endif

LIB_DIRS=

#use '-G' with Solaris
LFLAGS = -lc -lstdc++ 
#LFLAGS = -lc -lstdc++ -G


LIBZ  = -lz
#use this if you've built an -fPIC version of libz.a to link into the libhtdig.so
#LIBZ  = -lz-fpic

#berkeley db c files
BDB_C_OBJS += bt_compare.o bt_conv.o bt_curadj.o bt_cursor.o bt_delete.o bt_method.o bt_open.o bt_put.o bt_rec.o bt_reclaim.o bt_recno.o bt_rsearch.o bt_search.o bt_split.o bt_stat.o bt_upgrade.o btree_auto.o crdel_auto.o crdel_rec.o db.o db_am.o db_auto.o db_byteorder.o db_conv.o db_dispatch.o db_dup.o db_err.o db_getlong.o db_iface.o db_join.o db_log2.o db_meta.o db_method.o db_overflow.o db_pr.o db_rec.o db_reclaim.o db_ret.o db_salloc.o db_shash.o db_upgrade.o env_method.o env_open.o env_recover.o env_region.o hash.o hash_auto.o hash_conv.o hash_dup.o hash_func.o hash_meta.o hash_method.o hash_page.o hash_rec.o hash_reclaim.o hash_stat.o hash_upgrade.o lock.o lock_conflict.o lock_deadlock.o lock_region.o lock_util.o log.o log_archive.o log_auto.o log_compare.o log_findckp.o log_get.o log_method.o log_put.o log_rec.o log_register.o mp_alloc.o mp_bh.o mp_cmpr.o mp_fget.o mp_fopen.o mp_fput.o mp_fset.o mp_method.o mp_region.o mp_register.o mp_stat.o mp_sync.o mp_trickle.o mut_fcntl.o mut_pthread.o mut_tas.o mutex.o os_abs.o os_alloc.o os_dir.o os_errno.o os_fid.o os_finit.o os_fsync.o os_handle.o os_map.o os_method.o os_oflags.o os_open.o os_region.o os_rename.o os_root.o os_rpath.o os_rw.o os_seek.o os_sleep.o os_spin.o os_stat.o os_tmpdir.o os_unlink.o qam.o qam_auto.o qam_conv.o qam_method.o qam_open.o qam_rec.o qam_stat.o txn.o txn_auto.o txn_rec.o txn_region.o xa.o xa_db.o xa_map.o 

#htlib c files  $(REGEX)
HTLIB_C_OBJS += getcwd.o mhash_md5.o regex.o vsnprintf.o memcmp.o mktime.o snprintf.o memcpy.o myqsort.o strerror.o memmove.o raise.o timegm.o

#htlib c++ files
HTLIB_CXX_OBJS += Configuration.o Database.o Dictionary.o DB2_db.o IntObject.o List.o Object.o ParsedString.o Queue.o QuotedStringList.o Stack.o String.o StringList.o StringMatch.o String_fmt.o good_strtok.o strcasecmp.o strptime.o HtCodec.o HtWordCodec.o HtVector.o HtHeap.o HtPack.o HtDateTime.o HtRegex.o HtRegexList.o HtRegexReplace.o HtRegexReplaceList.o HtVectorGeneric.o HtMaxMin.o HtWordType.o md5.o 

#htword c++ files
HTWORD_CXX_OBJS += WordBitCompress.o WordContext.o WordCursor.o WordDB.o WordDBCompress.o WordDBInfo.o WordDBPage.o WordKey.o WordKeyInfo.o WordList.o WordMonitor.o WordRecord.o WordRecordInfo.o WordReference.o WordStat.o WordType.o 

#htcommon c++ files
HTCOMMON_CXX_OBJS += DocumentDB.o DocumentRef.o HtWordReference.o HtWordList.o defaults.o HtURLCodec.o URL.o URLTrans.o HtZlibCodec.o cgi.o HtSGMLCodec.o HtConfiguration.o HtURLRewriter.o

#htnet c++ files
HTNET_CXX_OBJS += Connection.o Transport.o HtHTTP.o HtFile.o HtNNTP.o HtCookie.o HtCookieJar.o HtCookieMemJar.o HtHTTPBasic.o HtHTTPSecure.o SSLConnection.o HtFTP.o HtCookieInFileJar.o

#htdig c++ files
HTDIG_CXX_OBJS += Document.o ExternalTransport.o Parsable.o Retriever.o URLRef.o ExternalParser.o HTML.o Plaintext.o Server.o

#htfuzzy c++ files
HTFUZZY_CXX_OBJS += Accents.o EndingsDB.o Fuzzy.o Prefix.o Soundex.o Substring.o Synonym.o Endings.o Exact.o Metaphone.o Regexp.o Speling.o SuffixEntry.o filecopy.o

#HTFUZZY_C_OBJS += filecopy.o

#htsearch c++ files
HTSEARCH_CXX_OBJS += Collection.o DocMatch.o  ResultList.o SplitMatches.o TemplateList.o Display.o  HtURLSeedScore.o ResultMatch.o Template.o WeightWord.o parser.o

#libhtdig c++ files
LIBHTDIG_CXX_OBJS += ResultFetch.o BasicDocument.o TextCollector.o libhtdig_htdig.o libhtdig_htmerge.o libhtdig_htfuzzy.o libhtdig_log.o libhtdig_htsearch.o

#htcommon lex & yacc targets
LIBHTDIG_CXX_OBJS += conf_lexer.o conf_parser.o

#libhtdig c files
#LIBHTDIG_C_OBJS += filecopy.o

LXX_TARGETS += conf_lexer.cc

YXX_TARGETS += conf_parser.cc

OBJS += $(BDB_C_OBJS) $(HTLIB_C_OBJS) $(HTLIB_CXX_OBJS) $(HTWORD_CXX_OBJS)
OBJS += $(HTCOMMON_CXX_OBJS) $(HTNET_CXX_OBJS) $(HTDIG_CXX_OBJS)
OBJS += $(HTFUZZY_CXX_OBJS) $(HTFUZZY_C_OBJS) $(HTSEARCH_CXX_OBJS) $(LIBHTDIG_CXX_OBJS) $(LIBHTDIG_C_OBJS)


libhtdig-3.2.0.so: $(OBJS) $(LXX_TARGETS) $(YXX_TARGETS)
	$(CC) -shared $(LIB_DIRS) $(OTHER_OBJS) $(OBJS) -L/usr/local/lib $(LIBZ) $(LFLAGS) -Xlinker -h -Xlinker libhtdig.so.$(LIBHTDIG_BUILD_VER) -o libhtdig.so.$(LIBHTDIG_BUILD_VER)


libhtdig.a: $(OBJS) $(LXX_TARGETS) $(YXX_TARGETS)
	ar cru  libhtdig.a  $(OTHER_OBJS) $(OBJS)
	ranlib libhtdig.a


$(BDB_C_OBJS): %.o: ../db/%.c
	$(CC) $(BDB_INC_DIRS) $(BDB_DEFS) $(OPTS) $(DBG) -c $< -o $@

$(HTLIB_C_OBJS): %.o: ../htlib/%.c
	$(CC) $(INC_DIRS) $(DEFS) $(OPTS) $(DBG) -c $< -o $@

$(HTLIB_CXX_OBJS): %.o: ../htlib/%.cc
	$(CXX) $(INC_DIRS) $(DEFS) $(OPTS) $(DBG) -c $< -o $@

$(HTWORD_CXX_OBJS): %.o: ../htword/%.cc
	$(CXX) $(INC_DIRS) $(DEFS) $(OPTS) $(DBG) -c $< -o $@

$(HTCOMMON_CXX_OBJS): %.o: ../htcommon/%.cc
	$(CXX) $(INC_DIRS) $(DEFS) $(OPTS) $(DBG) -c $< -o $@

$(HTNET_CXX_OBJS): %.o: ../htnet/%.cc
	$(CXX) $(INC_DIRS) $(DEFS) $(OPTS) $(DBG) -c $< -o $@

$(HTDIG_CXX_OBJS): %.o: ../htdig/%.cc
	$(CXX) $(HTDIG_INC_DIR) $(INC_DIRS) $(DEFS) $(OPTS) $(DBG) -c $< -o $@

$(HTFUZZY_C_OBJS): %.o: ../htfuzzy/%.c
	$(CC) $(INC_DIRS) $(DEFS) $(OPTS) $(DBG) -c $< -o $@

$(HTFUZZY_CXX_OBJS): %.o: ../htfuzzy/%.cc
	$(CXX) $(INC_DIRS) $(DEFS) $(OPTS) $(DBG) -c $< -o $@

$(HTSEARCH_CXX_OBJS): %.o: ../htsearch/%.cc
	$(CXX) $(INC_DIRS) $(DEFS) $(OPTS) $(DBG) -c $< -o $@

$(LIBHTDIG_CXX_OBJS): %.o: %.cc
	$(CXX) $(HTDIG_INC_DIR) $(INC_DIRS) $(DEFS) $(OPTS) $(DBG) -c $< -o $@

$(LIBHTDIG_C_OBJS): %.o: %.c
	$(CC) $(HTDIG_INC_DIR) $(INC_DIRS) $(DEFS) $(OPTS) $(DBG) -c $< -o $@

$(LXX_TARGETS): %.cc: ../htcommon/%.lxx
	$(LEX) $(AM_LFLAGS) $< && mv $(LEX_OUTPUT_ROOT).c $@
	#$(LEX) $(AM_LFLAGS) $(LFLAGS) -o$@  $<

$(YXX_TARGETS): %.cc: ../htcommon/%.yxx
	$(YACC) $(AM_YFLAGS) $< && mv $(YACC_OUTPUT_ROOT).c $@
	if test -f y.tab.h; then if cmp -s y.tab.h conf_parser.h; then rm -f y.tab.h; else mv y.tab.h conf_parser.h; fi; else :; fi


clean:
	rm -f *.o *~ *.bak *.lo *.a* *.so* core $(LXX_TARGETS) $(YXX_TARGETS)



