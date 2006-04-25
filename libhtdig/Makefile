#libhtdig.so makefile for Unix systems

LIBHTDIG_BUILD_VER = 4.0

libhtdig = libhtdig.so.$(LIBHTDIG_BUILD_VER)
libhtdigphp = libhtdigphp.so.$(LIBHTDIG_BUILD_VER)

.PHONY: clucene tidy default all

default: libhtdig libhtdigphp
all: tidy clucene default




#Berkeley DB Specific defines 
BDB_INC_DIRS = -I../db -I/usr/local/include
BDB_DEFS = -DHAVE_CONFIG_H -DHAVE_SHMGET

CL_DIR = ../CLucene
TIDY_DIR = ../HTMLTidy


## TODO TODO - the PATH to CLucene/src must be configured by the 'builder'
## just run make, not make install
CL_INC_DIR = -I$(CL_DIR)/src

## TODO TODO - the PATH to Tidy/src must be configured by the 'builder'
## just run make, not make install
TIDY_INC_DIR = -I$(TIDY_DIR)/include



INC_DIRS= -I. -I./CLuceneAPI -I../htdig -I../include -I../db -I../htlib -I../htnet -I../htcommon $(TIDY_INC_DIR) $(CL_INC_DIR)

PHP_INC_DIRS = $(INC_DIRS) -I/nfs/local/src/php/php-4.1.2-80/linux/ -I/nfs/local/src/php/php-4.1.2-80/linux/main -I/nfs/local/src/php/php-4.1.2-80/linux/Zend -I/nfs/local/src/php/php-4.1.2-80/linux/TSRM
#PHP_INC_DIRS = $(INC_DIRS) -I/nfs/local/src/php/php-4.1.2-73/linux/ -I/nfs/local/src/php/php-4.1.2-73/linux/main -I/nfs/local/src/php/php-4.1.2-73/linux/Zend -I/nfs/local/src/php/php-4.1.2-73/linux/TSRM
#PHP_INC_DIRS = $(INC_DIRS) -I/nfs/local/src/php/php-4.1.2-73/linux/-73 -I/nfs/local/src/php/php-4.1.2-73/linux/-73/main -I/nfs/local/src/php/php-4.1.2-73/linux/-73/Zend -I/nfs/local/src/php/php-4.1.2-73/linux/-73/TSRM

##INC_DIRS= -I. -I./CLuceneAPI -I../htdig -I../include -I../db -I../htlib -I../htsearch -I../htnet -I../htcommon -I/usr/local/include $(TIDY_INC_DIR) $(CL_INC_DIR)

# Clucene library build variables
# NOTE: CLUCENE_PATH must point to the CLucene src/.libs directory, where the -fPIC'ed objects are
CLUCENE_PATH= $(CL_DIR)/src/.libs
OLD__CLUCENE_OBJS = $(CLUCENE_PATH)/repl_wcs.o $(CLUCENE_PATH)/gunichartables.o $(CLUCENE_PATH)/repl_wchar.o $(CLUCENE_PATH)/StdHeader.o  $(CLUCENE_PATH)/BitVector.o $(CLUCENE_PATH)/dirent.o $(CLUCENE_PATH)/FastCharStream.o $(CLUCENE_PATH)/MD5Digester.o $(CLUCENE_PATH)/Misc.o $(CLUCENE_PATH)/Reader.o $(CLUCENE_PATH)/Equators.o $(CLUCENE_PATH)/StringBuffer.o $(CLUCENE_PATH)/StringIntern.o $(CLUCENE_PATH)/FSDirectory.o $(CLUCENE_PATH)/InputStream.o $(CLUCENE_PATH)/Lock.o $(CLUCENE_PATH)/OutputStream.o $(CLUCENE_PATH)/RAMDirectory.o $(CLUCENE_PATH)/TransactionalRAMDirectory.o $(CLUCENE_PATH)/condition.o $(CLUCENE_PATH)/error.o $(CLUCENE_PATH)/mempool.o $(CLUCENE_PATH)/memtracking.o $(CLUCENE_PATH)/Analyzers.o $(CLUCENE_PATH)/PorterStemmer.o $(CLUCENE_PATH)/AnalysisHeader.o $(CLUCENE_PATH)/StandardAnalyzer.o $(CLUCENE_PATH)/StandardFilter.o $(CLUCENE_PATH)/StandardTokenizer.o $(CLUCENE_PATH)/DateField.o $(CLUCENE_PATH)/Document.o $(CLUCENE_PATH)/Field.o $(CLUCENE_PATH)/CompoundFile.o $(CLUCENE_PATH)/DocumentWriter.o $(CLUCENE_PATH)/FieldInfos.o $(CLUCENE_PATH)/FieldsReader.o $(CLUCENE_PATH)/FieldsWriter.o $(CLUCENE_PATH)/IndexReader.o $(CLUCENE_PATH)/IndexWriter.o $(CLUCENE_PATH)/MultiReader.o $(CLUCENE_PATH)/SegmentInfos.o $(CLUCENE_PATH)/SegmentMergeInfo.o $(CLUCENE_PATH)/SegmentMerger.o $(CLUCENE_PATH)/SegmentMergeQueue.o $(CLUCENE_PATH)/SegmentReader.o $(CLUCENE_PATH)/SegmentTermDocs.o $(CLUCENE_PATH)/SegmentTermEnum.o $(CLUCENE_PATH)/SegmentTermPositions.o $(CLUCENE_PATH)/SegmentTermVector.o $(CLUCENE_PATH)/Term.o $(CLUCENE_PATH)/TermInfo.o $(CLUCENE_PATH)/TermInfosReader.o $(CLUCENE_PATH)/TermInfosWriter.o $(CLUCENE_PATH)/TermVectorReader.o $(CLUCENE_PATH)/TermVectorWriter.o $(CLUCENE_PATH)/Lexer.o $(CLUCENE_PATH)/QueryParser.o $(CLUCENE_PATH)/QueryParserBase.o $(CLUCENE_PATH)/QueryToken.o $(CLUCENE_PATH)/TokenList.o $(CLUCENE_PATH)/MultiFieldQueryParser.o $(CLUCENE_PATH)/BooleanQuery.o $(CLUCENE_PATH)/BooleanScorer.o $(CLUCENE_PATH)/DateFilter.o $(CLUCENE_PATH)/ConjunctionScorer.o $(CLUCENE_PATH)/ExactPhraseScorer.o $(CLUCENE_PATH)/Explanation.o $(CLUCENE_PATH)/FieldCache.o $(CLUCENE_PATH)/FieldCacheImpl.o $(CLUCENE_PATH)/FieldSortedHitQueue.o $(CLUCENE_PATH)/FieldDocSortedHitQueue.o $(CLUCENE_PATH)/FilteredTermEnum.o $(CLUCENE_PATH)/FuzzyQuery.o $(CLUCENE_PATH)/Hits.o $(CLUCENE_PATH)/HitQueue.o $(CLUCENE_PATH)/IndexSearcher.o $(CLUCENE_PATH)/MultiSearcher.o $(CLUCENE_PATH)/MultiTermQuery.o $(CLUCENE_PATH)/PhrasePositions.o $(CLUCENE_PATH)/PhraseQuery.o $(CLUCENE_PATH)/PhraseScorer.o $(CLUCENE_PATH)/PrefixQuery.o $(CLUCENE_PATH)/RangeQuery.o $(CLUCENE_PATH)/SearchHeader.o $(CLUCENE_PATH)/Similarity.o $(CLUCENE_PATH)/SloppyPhraseScorer.o $(CLUCENE_PATH)/Sort.o $(CLUCENE_PATH)/TermQuery.o $(CLUCENE_PATH)/TermScorer.o $(CLUCENE_PATH)/WildcardQuery.o $(CLUCENE_PATH)/WildcardTermEnum.o

CLUCENE_OBJS = $(CLUCENE_PATH)/gunichartables.o $(CLUCENE_PATH)/repl_lltot.o $(CLUCENE_PATH)/repl_tcscasecmp.o $(CLUCENE_PATH)/repl_tcslwr.o $(CLUCENE_PATH)/repl_tcstod.o $(CLUCENE_PATH)/repl_tcstoll.o $(CLUCENE_PATH)/repl_tprintf.o $(CLUCENE_PATH)/utf8.o $(CLUCENE_PATH)/StdHeader.o $(CLUCENE_PATH)/BitVector.o $(CLUCENE_PATH)/dirent.o $(CLUCENE_PATH)/Equators.o $(CLUCENE_PATH)/FastCharStream.o $(CLUCENE_PATH)/MD5Digester.o $(CLUCENE_PATH)/Misc.o $(CLUCENE_PATH)/Reader.o $(CLUCENE_PATH)/StringBuffer.o $(CLUCENE_PATH)/StringIntern.o $(CLUCENE_PATH)/FSDirectory.o $(CLUCENE_PATH)/IndexInput.o $(CLUCENE_PATH)/Lock.o $(CLUCENE_PATH)/IndexOutput.o $(CLUCENE_PATH)/MMapInput.o $(CLUCENE_PATH)/RAMDirectory.o $(CLUCENE_PATH)/TransactionalRAMDirectory.o $(CLUCENE_PATH)/condition.o $(CLUCENE_PATH)/error.o $(CLUCENE_PATH)/mempool.o $(CLUCENE_PATH)/memtracking.o $(CLUCENE_PATH)/Analyzers.o $(CLUCENE_PATH)/AnalysisHeader.o $(CLUCENE_PATH)/PorterStemmer.o $(CLUCENE_PATH)/StandardAnalyzer.o $(CLUCENE_PATH)/StandardFilter.o $(CLUCENE_PATH)/StandardTokenizer.o $(CLUCENE_PATH)/DateField.o $(CLUCENE_PATH)/Document.o $(CLUCENE_PATH)/Field.o $(CLUCENE_PATH)/CompoundFile.o $(CLUCENE_PATH)/DocumentWriter.o $(CLUCENE_PATH)/FieldInfos.o $(CLUCENE_PATH)/FieldsReader.o $(CLUCENE_PATH)/FieldsWriter.o $(CLUCENE_PATH)/IndexModifier.o $(CLUCENE_PATH)/IndexReader.o $(CLUCENE_PATH)/IndexWriter.o $(CLUCENE_PATH)/MultiReader.o $(CLUCENE_PATH)/SegmentInfos.o $(CLUCENE_PATH)/SegmentMergeInfo.o $(CLUCENE_PATH)/SegmentMerger.o $(CLUCENE_PATH)/SegmentMergeQueue.o $(CLUCENE_PATH)/SegmentReader.o $(CLUCENE_PATH)/SegmentTermDocs.o $(CLUCENE_PATH)/SegmentTermEnum.o $(CLUCENE_PATH)/SegmentTermPositions.o $(CLUCENE_PATH)/SegmentTermVector.o $(CLUCENE_PATH)/Term.o $(CLUCENE_PATH)/TermInfo.o $(CLUCENE_PATH)/TermInfosReader.o $(CLUCENE_PATH)/TermInfosWriter.o $(CLUCENE_PATH)/TermVectorReader.o $(CLUCENE_PATH)/TermVectorWriter.o $(CLUCENE_PATH)/Lexer.o $(CLUCENE_PATH)/QueryParser.o $(CLUCENE_PATH)/QueryParserBase.o $(CLUCENE_PATH)/QueryToken.o $(CLUCENE_PATH)/TokenList.o $(CLUCENE_PATH)/MultiFieldQueryParser.o $(CLUCENE_PATH)/BooleanQuery.o $(CLUCENE_PATH)/BooleanScorer.o $(CLUCENE_PATH)/DateFilter.o $(CLUCENE_PATH)/CachingWrapperFilter.o $(CLUCENE_PATH)/ChainedFilter.o $(CLUCENE_PATH)/ConjunctionScorer.o $(CLUCENE_PATH)/ExactPhraseScorer.o $(CLUCENE_PATH)/Explanation.o $(CLUCENE_PATH)/FieldCache.o $(CLUCENE_PATH)/FieldCacheImpl.o $(CLUCENE_PATH)/FieldSortedHitQueue.o $(CLUCENE_PATH)/FieldDocSortedHitQueue.o $(CLUCENE_PATH)/FilteredTermEnum.o $(CLUCENE_PATH)/FuzzyQuery.o $(CLUCENE_PATH)/Hits.o $(CLUCENE_PATH)/HitQueue.o $(CLUCENE_PATH)/IndexSearcher.o $(CLUCENE_PATH)/MultiSearcher.o $(CLUCENE_PATH)/MultiTermQuery.o $(CLUCENE_PATH)/PhrasePositions.o $(CLUCENE_PATH)/PhraseQuery.o $(CLUCENE_PATH)/PhraseScorer.o $(CLUCENE_PATH)/PrefixQuery.o $(CLUCENE_PATH)/QueryFilter.o $(CLUCENE_PATH)/RangeFilter.o $(CLUCENE_PATH)/RangeQuery.o $(CLUCENE_PATH)/SearchHeader.o $(CLUCENE_PATH)/Similarity.o $(CLUCENE_PATH)/SloppyPhraseScorer.o $(CLUCENE_PATH)/Sort.o $(CLUCENE_PATH)/TermQuery.o $(CLUCENE_PATH)/TermScorer.o $(CLUCENE_PATH)/WildcardQuery.o $(CLUCENE_PATH)/WildcardTermEnum.o


# HTMLtidy library build variables
# NOTE: TIDY_PATH must point to the tidy src/ directory and use the .lo objects, which are -fPIC'ed 
TIDY_PATH= $(TIDY_DIR)/src
TIDY_OBJS=$(TIDY_PATH)/access.lo $(TIDY_PATH)/attrs.lo $(TIDY_PATH)/istack.lo $(TIDY_PATH)/parser.lo $(TIDY_PATH)/tags.lo $(TIDY_PATH)/entities.lo $(TIDY_PATH)/lexer.lo $(TIDY_PATH)/pprint.lo $(TIDY_PATH)/clean.lo $(TIDY_PATH)/localize.lo $(TIDY_PATH)/config.lo $(TIDY_PATH)/alloc.lo $(TIDY_PATH)/attrask.lo $(TIDY_PATH)/attrdict.lo $(TIDY_PATH)/attrget.lo $(TIDY_PATH)/buffio.lo $(TIDY_PATH)/fileio.lo $(TIDY_PATH)/streamio.lo $(TIDY_PATH)/tagask.lo $(TIDY_PATH)/tmbstr.lo $(TIDY_PATH)/utf8.lo $(TIDY_PATH)/tidylib.lo

TIDY_LIB = libhttidy.a
CLUCENE_LIB = libhtclucene.a



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
DEFS+= -DLIBHTDIG

#these are for compatability things...
DEFS+= -DHAVE_VSNPRINTF

PHP_DEFS = $(DEFS) -DPHP_ATOM_INC -DCOMPILE_DL_HTDIG


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
GCCOPTS=$(OPTIMZ) -Wall -fno-exceptions -fPIC
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

LFLAGS = -lgcc -lstdc++ 

ifeq ($(ARCH),linux)
LFLAGS = -lc -lstdc++ XXXX
endif

#use '-G' with Solaris
ifeq ($(ARCH),sunos)
LFLAGS = -lc -lstdc++ -G
endif


#LIBZ  = -lz
#LIBZ  = /usr/lib/libz.a
#use this if you've built an -fPIC version of libz.a to link into the libhtdig.so
#LIBZ  = -lz-fpic

#berkeley db c files
BDB_C_OBJS += bt_compare.o bt_conv.o bt_curadj.o bt_cursor.o bt_delete.o bt_method.o bt_open.o bt_put.o bt_rec.o bt_reclaim.o bt_recno.o bt_rsearch.o bt_search.o bt_split.o bt_stat.o bt_upgrade.o btree_auto.o crdel_auto.o crdel_rec.o db.o db_am.o db_auto.o db_byteorder.o db_conv.o db_dispatch.o db_dup.o db_err.o db_getlong.o db_iface.o db_join.o db_log2.o db_meta.o db_method.o db_overflow.o db_pr.o db_rec.o db_reclaim.o db_ret.o db_salloc.o db_shash.o db_upgrade.o env_method.o env_open.o env_recover.o env_region.o hash.o hash_auto.o hash_conv.o hash_dup.o hash_func.o hash_meta.o hash_method.o hash_page.o hash_rec.o hash_reclaim.o hash_stat.o hash_upgrade.o lock.o lock_conflict.o lock_deadlock.o lock_region.o lock_util.o log.o log_archive.o log_auto.o log_compare.o log_findckp.o log_get.o log_method.o log_put.o log_rec.o log_register.o mp_alloc.o mp_bh.o mp_fget.o mp_fopen.o mp_fput.o mp_fset.o mp_method.o mp_region.o mp_register.o mp_stat.o mp_sync.o mp_trickle.o mut_fcntl.o mut_pthread.o mut_tas.o mutex.o os_abs.o os_alloc.o os_dir.o os_errno.o os_fid.o os_finit.o os_fsync.o os_handle.o os_map.o os_method.o os_oflags.o os_open.o os_region.o os_rename.o os_root.o os_rpath.o os_rw.o os_seek.o os_sleep.o os_spin.o os_stat.o os_tmpdir.o os_unlink.o qam.o qam_auto.o qam_conv.o qam_method.o qam_open.o qam_rec.o qam_stat.o txn.o txn_auto.o txn_rec.o txn_region.o xa.o xa_db.o xa_map.o 
# mp_cmpr.o

#htlib c files  $(REGEX)
HTLIB_C_OBJS += getcwd.o mhash_md5.o regex.o vsnprintf.o memcmp.o mktime.o snprintf.o memcpy.o myqsort.o strerror.o memmove.o raise.o timegm.o

#htlib c++ files
HTLIB_CXX_OBJS += Configuration.o Database.o Dictionary.o DB2_db.o IntObject.o List.o Object.o ParsedString.o Queue.o QuotedStringList.o Stack.o String.o StringList.o StringMatch.o String_fmt.o good_strtok.o strcasecmp.o strptime.o HtCodec.o HtWordCodec.o HtVector.o HtHeap.o HtPack.o HtDateTime.o HtRegex.o HtRegexList.o HtRegexReplace.o HtRegexReplaceList.o HtVectorGeneric.o HtMaxMin.o HtWordType.o md5.o filecopy.o WordType.o

#htnet c++ files
HTNET_CXX_OBJS += Connection.o Transport.o HtHTTP.o HtFile.o HtNNTP.o HtCookie.o HtCookieJar.o HtCookieMemJar.o HtHTTPBasic.o HtHTTPSecure.o SSLConnection.o HtFTP.o HtCookieInFileJar.o

#htdig c++ files
HTDIG_CXX_OBJS += ExternalTransport.o Spider.o Document.o URLRef.o Server.o TidyParser.o 

# CLuceneAPI files
CLAPI_CXX_OBJS = CLuceneAPI.o 

#htcommon c++ files
HTCOMMON_CXX_OBJS += HtStdHeader.o IndexDBRef.o IndexDB.o DocumentRef.o defaults.o HtURLCodec.o URL.o URLTrans.o cgi.o HtSGMLCodec.o HtConfiguration.o HtURLRewriter.o

#libhtdig c++ files
LIBHTDIG_CXX_OBJS += libhtdig_log.o libhtdig_htsearch.o libhtdig_htdig.o
LIBHTDIG_CXX_OBJS += conf_lexer.o conf_parser.o

#htcommon lex & yacc targets
LXX_TARGETS += conf_lexer.cc
YXX_TARGETS += conf_parser.cc

#htdig php wrapper
PHP_C_OBJS += htdigphp.o 



# The CLucene and HTMLtidy external libraries
EXT_LIBS += $(CLUCENE_LIB) $(TIDY_LIB)

OBJS += $(BDB_C_OBJS) $(HTLIB_C_OBJS) $(HTLIB_CXX_OBJS) 
OBJS += $(HTCOMMON_CXX_OBJS) $(HTNET_CXX_OBJS) $(HTDIG_CXX_OBJS)
OBJS += $(LIBHTDIG_CXX_OBJS) $(CLAPI_CXX_OBJS)

PHP_OBJS = $(OBJS) $(PHP_C_OBJS)

SSL_INC  = -I/nfs/local/linux/openssl/openssl-0.9.7-stable-SNAP-20040331/include/openssl -DHAVE_SSL
SSL_INC += -I/nfs/local/linux/openssl/openssl-0.9.7-stable-SNAP-20040331/include/
SSL_LIB = -L/nfs/local/linux/openssl/openssl-0.9.7-stable-SNAP-20040331/lib -lssl -lcrypto




libhtdig: $(OBJS) $(LXX_TARGETS) $(YXX_TARGETS) $(EXT_LIBS)
	$(CC) -shared $(OBJS) $(SSL_LIB) $(EXT_LIBS) $(LFLAGS) -Xlinker -h -Xlinker $(libhtdig) -o $(libhtdig)
#$(CC) -shared $(LIB_DIRS) $(OTHER_OBJS) $(OBJS) $(LIBZ) $(LFLAGS) $(SSL_LIB) $(EXT_LIBS) -Xlinker -h -Xlinker libhtdig.so.$(LIBHTDIG_BUILD_VER) -o libhtdig.so.$(LIBHTDIG_BUILD_VER)


libhtdigphp: $(PHP_OBJS) $(LXX_TARGETS) $(YXX_TARGETS) $(EXT_LIBS)
	$(CC) -shared $(PHP_OBJS) $(SSL_LIB) $(EXT_LIBS) $(LFLAGS) -Xlinker -h -Xlinker $(libhtdigphp) -o $(libhtdigphp)

clucene:
	@echo 
	@echo '---- Making CLucene ----'
	@echo '---- with RightNow modification ----'
	@perl -pi.bak -e 's/^postdeps="-lstdc\+\+ -lm -lgcc -lc -lgcc"/postdeps="-lm -lc"/g' $(CL_DIR)/libtool
	$(MAKE) -s -C $(CL_DIR)
	@perl -pi.bak -e 's/^postdeps="-lm -lc"/postdeps="-lstdc++ -lm -lgcc -lc -lgcc"/g' $(CL_DIR)/libtool
	@echo 


#perl -pi.bak -e 's/^postdeps="-lstdc\+\+ -lm -lgcc -lc -lgcc"/postdeps="-lstdc++ -lm -lc"/g' $(CL_DIR)/libtool
#perl -pi.bak -e 's/^postdeps="-lstdc\+\+ -lm -lc"/postdeps="-lstdc++ -lm -lgcc -lc -lgcc"/g' $(CL_DIR)/libtool

tidy:
	@echo 
	@echo '---- Making HTMLTidy ----'
	$(MAKE) -s -C $(TIDY_DIR)
	@echo 


#$(CLUCENE_LIB): clucene $(CLUCENE_OBJS)
$(CLUCENE_LIB): $(CLUCENE_OBJS)
	ar cru $(CLUCENE_LIB) $(CLUCENE_OBJS)

#$(TIDY_LIB): tidy $(TIDY_OBJS)
$(TIDY_LIB): $(TIDY_OBJS)
	ar cru $(TIDY_LIB) $(TIDY_OBJS)

$(BDB_C_OBJS): %.o: ../db/%.c
	$(CC) -MM $(BDB_INC_DIRS) $(BDB_DEFS) $(GCCOPTS) $(DBG) -c $< -MF $@.d
	$(CC) $(BDB_INC_DIRS) $(BDB_DEFS) $(GCCOPTS) $(DBG) -c $< -o $@

$(HTLIB_C_OBJS): %.o: ../htlib/%.c
	$(CC) -MM $(INC_DIRS) $(DEFS) $(GCCOPTS) $(DBG) -c $< -MF $@.d
	$(CC) $(INC_DIRS) $(DEFS) $(GCCOPTS) $(DBG) -c $< -o $@

$(HTLIB_CXX_OBJS): %.o: ../htlib/%.cc
	$(CXX) -MM $(INC_DIRS) $(DEFS) $(OPTS) $(DBG) -c $< -MF $@.d
	$(CXX) $(INC_DIRS) $(DEFS) $(OPTS) $(DBG) -c $< -o $@

$(HTCOMMON_CXX_OBJS): %.o: ../htcommon/%.cc
	$(CXX) -MM $(INC_DIRS) $(DEFS) $(OPTS) $(DBG) -c $< -MF $@.d
	$(CXX) $(INC_DIRS) $(DEFS) $(OPTS) $(DBG) -c $< -o $@

$(HTNET_CXX_OBJS): %.o: ../htnet/%.cc
	$(CXX) -MM $(INC_DIRS) $(DEFS) $(OPTS) $(SSL_INC) $(DBG) -c $< -MF $@.d
	$(CXX) $(INC_DIRS) $(DEFS) $(OPTS) $(SSL_INC) $(DBG) -c $< -o $@

$(CLAPI_CXX_OBJS): %.o: ./CLuceneAPI/%.cc 
	$(CXX) -MM -DHAVE_CONFIG_H $(INC_DIRS) -fPIC -fno-rtti $(DBG) -c $< -MF $@.d
	$(CXX) -DHAVE_CONFIG_H $(INC_DIRS) -fPIC -fno-rtti $(DBG) -c $< -o $@

$(HTDIG_CXX_OBJS): %.o: ../htdig/%.cc
	$(CXX) -MM $(INC_DIRS) $(DEFS) $(OPTS) $(SSL_INC) $(DBG) -c $< -MF $@.d
	$(CXX) $(INC_DIRS) $(DEFS) $(OPTS) $(SSL_INC) $(DBG) -c $< -o $@

$(LXX_TARGETS): %.cc: ../htcommon/%.lxx
	$(LEX) $(AM_LFLAGS) $< && mv $(LEX_OUTPUT_ROOT).c $@
	#$(LEX) $(AM_LFLAGS) $(LFLAGS) -o$@  $<

$(YXX_TARGETS): %.cc: ../htcommon/%.yxx
	$(YACC) $(AM_YFLAGS) $< && mv $(YACC_OUTPUT_ROOT).c $@
	if test -f y.tab.h; then if cmp -s y.tab.h conf_parser.h; then rm -f y.tab.h; else mv y.tab.h conf_parser.h; fi; else :; fi

$(LIBHTDIG_CXX_OBJS): %.o: %.cc
	$(CXX) -MM $(INC_DIRS) $(DEFS) $(SSL_INC) $(OPTS) $(DBG) -c $< -MF $@.d
	$(CXX) $(INC_DIRS) $(DEFS) $(SSL_INC) $(OPTS) $(DBG) -c $< -o $@

$(LIBHTDIG_C_OBJS): %.o: %.c
	$(CC) -MM $(INC_DIRS) $(DEFS) $(GCCOPTS) $(DBG) -c $< -MF $@.d
	$(CC) $(INC_DIRS) $(DEFS) $(GCCOPTS) $(DBG) -c $< -o $@

$(PHP_C_OBJS): %.o: %.c
	$(CC) -MM $(PHP_INC_DIRS) $(PHP_DEFS) $(GCCOPTS) $(DBG) -c $< -MF $@.d
	$(CC)  $(PHP_INC_DIRS) $(PHP_DEFS) $(GCCOPTS) $(DBG) -c $< -o $@


clean:
	rm -f $(OBJS) $(addsuffix .d, $(OBJS)) $(libhtdig) $(libhtdigphp) *~ *.bak *.lo core $(LXX_TARGETS) $(YXX_TARGETS) $(CLUCENE_LIB) $(TIDY_LIB)
#rm -f *.o *.o.d *~ *.bak *.lo *.a* *.so* core $(LXX_TARGETS) $(YXX_TARGETS) $(CLUCENE_LIB) $(TIDY_LIB)

cleanall: clean
	@echo 
	@echo '---- Cleaning CLucene ----'
	@$(MAKE) -s -C $(CL_DIR) clean
	@echo 
	@echo '---- Cleaning HTMLTidy ----'
	@$(MAKE) -s -C $(TIDY_DIR) clean
	@echo 


#depend stuff
-include $(addsuffix .d, $(PHP_OBJS))
#-include $(addprefix $(DEPDIR)/, $(subst ../,, $(SRC:.c=.d) $(CXXSRC:.cpp=.d)))
