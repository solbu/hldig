#
# libhtdig.so makefile for Unix systems
#

ROOT_DIR = ../..

OBJS_DIR = .objs
DEPS_DIR = .deps
LIB_DIR = lib

SRC_DIR = $(ROOT_DIR)/src
BDB_DIR = $(ROOT_DIR)/BDB
CL_DIR = $(ROOT_DIR)/CLucene
CL_CONTRIB_DIR = $(CL_DIR)/contributions
TIDY_DIR = $(ROOT_DIR)/HTMLTidy

LIBHTDIG_BUILD_VER = 4.0.0

libhtdig = $(LIB_DIR)/libhtdig.so.$(LIBHTDIG_BUILD_VER)
libhtdigphp = $(LIB_DIR)/libhtdigphp.so.$(LIBHTDIG_BUILD_VER)

TIDY_LIB = $(LIB_DIR)/libhttidy.a
CLUCENE_LIB = $(LIB_DIR)/libhtclucene.a

# The CLucene and HTMLtidy external libraries
EXT_LIBS += $(CLUCENE_LIB) $(TIDY_LIB)


default: libhtdig libhtdigphp
all: tidy clucene default

.PHONY: clucene tidy default all



#Berkeley DB Specific defines 
BDB_DEFS = -DHAVE_CONFIG_H
BDB_INC_DIR = -I$(BDB_DIR) -I/usr/local/include

## TODO TODO - the PATH to CLucene/src must be configured by the 'builder'
## just run make, not make install
CL_INC_DIR = -I$(CL_DIR)/src
CL_CONTRIB_INC_DIR = -I$(CL_CONTRIB_DIR)/src

## TODO TODO - the PATH to Tidy/src must be configured by the 'builder'
## just run make, not make install
TIDY_INC_DIR = -I$(TIDY_DIR)/include


INC_DIRS= -I$(SRC_DIR)/libhtdig -I$(SRC_DIR)/CLuceneAPI -I$(SRC_DIR)/spider -I$(SRC_DIR)/common -I$(SRC_DIR)/config -I$(SRC_DIR)/network $(BDB_INC_DIR) $(TIDY_INC_DIR) $(CL_INC_DIR) $(CL_CONTRIB_INC_DIR)

#system specific PHP information
PHP_INC_DIRS = $(INC_DIRS) -I/nfs/local/linux/include/php-4.1.2-80/ -I/nfs/local/linux/include/php-4.1.2-80/main -I/nfs/local/linux/include/php-4.1.2-80/Zend -I/nfs/local/linux/include/php-4.1.2-80/TSRM

#system specific SSL library information
SSL_INC  = -I/nfs/local/linux/openssl/openssl-0.9.7-stable-SNAP-20040331/include/openssl -DHAVE_SSL -DHAVE_SSL_H
SSL_INC += -I/nfs/local/linux/openssl/openssl-0.9.7-stable-SNAP-20040331/include/
SSL_LIB = -L/nfs/local/linux/openssl/openssl-0.9.7-stable-SNAP-20040331/lib -lssl -lcrypto




###########################################
#
#
# objects
#
#
###########################################

# Clucene library build variables
# NOTE:  must point to the CLucene src/.libs directory, where the -fPIC'ed objects are
CL_OBJ_DIR = $(CL_DIR)/src/.libs
CL_OBJS = $(addprefix $(CL_OBJ_DIR)/, AnalysisHeader.o Analyzers.o BitVector.o BooleanQuery.o BooleanScorer.o CachingWrapperFilter.o ChainedFilter.o CompoundFile.o condition.o ConjunctionScorer.o DateField.o DateFilter.o dirent.o Document.o DocumentWriter.o Equators.o error.o ExactPhraseScorer.o Explanation.o FastCharStream.o FieldCacheImpl.o FieldCache.o FieldDocSortedHitQueue.o FieldInfos.o Field.o FieldSortedHitQueue.o FieldsReader.o FieldsWriter.o fileinputstream.o FilteredTermEnum.o FSDirectory.o FuzzyQuery.o gunichartables.o HitQueue.o Hits.o IndexInput.o IndexModifier.o IndexOutput.o IndexReader.o IndexSearcher.o IndexWriter.o Lexer.o Lock.o MD5Digester.o mempool.o memtracking.o Misc.o MMapInput.o MultiFieldQueryParser.o MultiReader.o MultiSearcher.o MultiTermQuery.o PhrasePositions.o PhraseQuery.o PhraseScorer.o PrefixQuery.o QueryFilter.o QueryParserBase.o QueryParser.o QueryToken.o RAMDirectory.o RangeFilter.o RangeQuery.o Reader.o repl_lltot.o repl_tcscasecmp.o repl_tcslwr.o repl_tcstod.o repl_tcstoll.o repl_tprintf.o SearchHeader.o SegmentInfos.o SegmentMergeInfo.o SegmentMergeQueue.o SegmentMerger.o SegmentReader.o SegmentTermDocs.o SegmentTermEnum.o SegmentTermPositions.o SegmentTermVector.o Similarity.o SloppyPhraseScorer.o Sort.o StandardAnalyzer.o StandardFilter.o StandardTokenizer.o StdHeader.o StringBuffer.o StringIntern.o TermInfo.o TermInfosReader.o TermInfosWriter.o Term.o TermQuery.o TermScorer.o TermVectorReader.o TermVectorWriter.o threads.o TokenList.o TransactionalRAMDirectory.o utf8.o WildcardQuery.o WildcardTermEnum.o)

#CL_OBJS = $(addprefix $(CL_OBJ_DIR)/, AnalysisHeader.o Analyzers.o api.o BitVector.o BooleanQuery.o BooleanScorer.o CachingWrapperFilter.o ChainedFilter.o CompoundFile.o condition.o ConjunctionScorer.o DateField.o DateFilter.o dirent.o Document.o DocumentWriter.o Encoder.o Equators.o error.o ExactPhraseScorer.o Explanation.o FastCharStream.o FieldCacheImpl.o FieldCache.o FieldDocSortedHitQueue.o FieldInfos.o Field.o FieldSortedHitQueue.o FieldsReader.o FieldsWriter.o fileinputstream.o FilteredTermEnum.o Formatter.o Fragmenter.o FSDirectory.o FuzzyQuery.o gunichartables.o Highlighter.o HitQueue.o Hits.o IndexInput.o IndexModifier.o IndexOutput.o IndexReader.o IndexSearcher.o IndexWriter.o Lexer.o libstemmer.o Lock.o MD5Digester.o mempool.o memtracking.o Misc.o MMapInput.o MultiFieldQueryParser.o MultiReader.o MultiSearcher.o MultiTermQuery.o PhrasePositions.o PhraseQuery.o PhraseScorer.o PrefixQuery.o QueryFilter.o QueryParserBase.o QueryParser.o QueryScorer.o QueryTermExtractor.o QueryToken.o RAMDirectory.o RangeFilter.o RangeQuery.o Reader.o repl_lltot.o repl_tcscasecmp.o repl_tcslwr.o repl_tcstod.o repl_tcstoll.o repl_tprintf.o SearchHeader.o SegmentInfos.o SegmentMergeInfo.o SegmentMergeQueue.o SegmentMerger.o SegmentReader.o SegmentTermDocs.o SegmentTermEnum.o SegmentTermPositions.o SegmentTermVector.o Similarity.o SimpleFragmenter.o SimpleHTMLEncoder.o SimpleHTMLFormatter.o SloppyPhraseScorer.o Snowball.o Sort.o StandardAnalyzer.o StandardFilter.o StandardTokenizer.o StdHeader.o stem_ISO_8859_1_danish.o stem_ISO_8859_1_dutch.o stem_ISO_8859_1_english.o stem_ISO_8859_1_finnish.o stem_ISO_8859_1_french.o stem_ISO_8859_1_german.o stem_ISO_8859_1_italian.o stem_ISO_8859_1_norwegian.o stem_ISO_8859_1_porter.o stem_ISO_8859_1_portuguese.o stem_ISO_8859_1_spanish.o stem_ISO_8859_1_swedish.o stem_KOI8_R_russian.o stem_UTF_8_danish.o stem_UTF_8_dutch.o stem_UTF_8_english.o stem_UTF_8_finnish.o stem_UTF_8_french.o stem_UTF_8_german.o stem_UTF_8_italian.o stem_UTF_8_norwegian.o stem_UTF_8_porter.o stem_UTF_8_portuguese.o stem_UTF_8_russian.o stem_UTF_8_spanish.o stem_UTF_8_swedish.o StringBuffer.o StringIntern.o TermInfo.o TermInfosReader.o TermInfosWriter.o Term.o TermQuery.o TermScorer.o TermVectorReader.o TermVectorWriter.o TextFragment.o threads.o TokenGroup.o TokenList.o TokenSources.o TransactionalRAMDirectory.o utf8.o utilities.o WeightedTerm.o WildcardQuery.o WildcardTermEnum.o)

CL_CONTRIB_OBJ_DIR = $(CL_CONTRIB_DIR)/src/.libs
CL_CONTRIB_OBJS = $(addprefix $(CL_CONTRIB_OBJ_DIR)/, api.o libclucene_contrib_la-Encoder.o libclucene_contrib_la-fileinputstream.o libclucene_contrib_la-filereader.o libclucene_contrib_la-Formatter.o libclucene_contrib_la-Fragmenter.o libclucene_contrib_la-Highlighter.o libclucene_contrib_la-inputstreamreader.o libclucene_contrib_la-QueryScorer.o libclucene_contrib_la-QueryTermExtractor.o libclucene_contrib_la-SimpleFragmenter.o libclucene_contrib_la-SimpleHTMLEncoder.o libclucene_contrib_la-SimpleHTMLFormatter.o libclucene_contrib_la-Snowball.o libclucene_contrib_la-subinputstream.o libclucene_contrib_la-TextFragment.o libclucene_contrib_la-TokenGroup.o libclucene_contrib_la-TokenSources.o libclucene_contrib_la-WeightedTerm.o libstemmer.o stem_ISO_8859_1_danish.o stem_ISO_8859_1_dutch.o stem_ISO_8859_1_english.o stem_ISO_8859_1_finnish.o stem_ISO_8859_1_french.o stem_ISO_8859_1_german.o stem_ISO_8859_1_italian.o stem_ISO_8859_1_norwegian.o stem_ISO_8859_1_porter.o stem_ISO_8859_1_portuguese.o stem_ISO_8859_1_spanish.o stem_ISO_8859_1_swedish.o stem_KOI8_R_russian.o stem_UTF_8_danish.o stem_UTF_8_dutch.o stem_UTF_8_english.o stem_UTF_8_finnish.o stem_UTF_8_french.o stem_UTF_8_german.o stem_UTF_8_italian.o stem_UTF_8_norwegian.o stem_UTF_8_porter.o stem_UTF_8_portuguese.o stem_UTF_8_russian.o stem_UTF_8_spanish.o stem_UTF_8_swedish.o utilities.o)

# HTMLtidy library build variables
# NOTE: TIDY_OBJ_DIR must point to the tidy src/ directory and use the .lo objects, which are -fPIC'ed ????? 
TIDY_OBJ_DIR= $(TIDY_DIR)/src
TIDY_OBJS = $(addprefix $(TIDY_OBJ_DIR)/, access.o attrs.o istack.o parser.o tags.o entities.o lexer.o pprint.o clean.o localize.o config.o alloc.o attrask.o attrdict.o attrget.o buffio.o fileio.o streamio.o tagask.o tmbstr.o utf8.o tidylib.o)
#TIDY_OBJS = $(addprefix $(TIDY_OBJ_DIR)/, access.lo attrs.lo istack.lo parser.lo tags.lo entities.lo lexer.lo pprint.lo clean.lo localize.lo config.lo alloc.lo attrask.lo attrdict.lo attrget.lo buffio.lo fileio.lo streamio.lo tagask.lo tmbstr.lo utf8.lo tidylib.lo)




#berkeley db c files
BDB_C_OBJS = $(addprefix $(OBJS_DIR)/, bt_compare.o bt_conv.o bt_curadj.o bt_cursor.o bt_delete.o bt_method.o bt_open.o bt_put.o bt_rec.o bt_reclaim.o bt_recno.o bt_rsearch.o bt_search.o bt_split.o bt_stat.o bt_upgrade.o btree_auto.o crdel_auto.o crdel_rec.o db.o db_am.o db_auto.o db_byteorder.o db_conv.o db_dispatch.o db_dup.o db_err.o db_getlong.o db_iface.o db_join.o db_log2.o db_meta.o db_method.o db_overflow.o db_pr.o db_rec.o db_reclaim.o db_ret.o db_salloc.o db_shash.o db_upgrade.o env_method.o env_open.o env_recover.o env_region.o hash.o hash_auto.o hash_conv.o hash_dup.o hash_func.o hash_meta.o hash_method.o hash_page.o hash_rec.o hash_reclaim.o hash_stat.o hash_upgrade.o lock.o lock_conflict.o lock_deadlock.o lock_region.o lock_util.o log.o log_archive.o log_auto.o log_compare.o log_findckp.o log_get.o log_method.o log_put.o log_rec.o log_register.o mp_alloc.o mp_bh.o mp_fget.o mp_fopen.o mp_fput.o mp_fset.o mp_method.o mp_region.o mp_register.o mp_stat.o mp_sync.o mp_trickle.o mut_fcntl.o mut_pthread.o mut_tas.o mutex.o os_abs.o os_alloc.o os_dir.o os_errno.o os_fid.o os_finit.o os_fsync.o os_handle.o os_map.o os_method.o os_oflags.o os_open.o os_region.o os_rename.o os_root.o os_rpath.o os_rw.o os_seek.o os_sleep.o os_spin.o os_stat.o os_tmpdir.o os_unlink.o qam.o qam_auto.o qam_conv.o qam_method.o qam_open.o qam_rec.o qam_stat.o txn.o txn_auto.o txn_rec.o txn_region.o xa.o xa_db.o xa_map.o)
# mp_cmpr.o

#htnet c++ files
NETWORK_CXX_OBJS = $(addprefix $(OBJS_DIR)/, Connection.o Transport.o HtHTTP.o HtFile.o HtNNTP.o HtCookie.o HtCookieJar.o HtCookieMemJar.o HtHTTPBasic.o HtHTTPSecure.o SSLConnection.o HtFTP.o HtCookieInFileJar.o)

#htdig c++ files
SPIDER_CXX_OBJS = $(addprefix $(OBJS_DIR)/, ExternalTransport.o Spider.o Document.o URLRef.o Server.o TidyParser.o ExternalParser.o)

# CLuceneAPI files
CLAPI_CXX_OBJS = $(addprefix $(OBJS_DIR)/, CLuceneAPI.o )

#htlib c files  $(REGEX)
COMMON_C_OBJS = $(addprefix $(OBJS_DIR)/, getcwd.o mhash_md5.o regex.o vsnprintf.o memcmp.o mktime.o snprintf.o memcpy.o myqsort.o strerror.o memmove.o raise.o timegm.o)

#htcommon c++ files
COMMON_CXX_OBJS = $(addprefix $(OBJS_DIR)/, HtStdHeader.o IndexDBRef.o IndexDB.o DocumentRef.o defaults.o HtURLCodec.o URL.o URLTrans.o cgi.o HtSGMLCodec.o HtConfiguration.o HtURLRewriter.o HtDebug.o Configuration.o Database.o Dictionary.o DB2_db.o IntObject.o List.o Object.o ParsedString.o Queue.o QuotedStringList.o Stack.o String.o StringList.o StringMatch.o String_fmt.o good_strtok.o strcasecmp.o strptime.o HtCodec.o HtWordCodec.o HtVector.o HtHeap.o HtPack.o HtDateTime.o HtRegex.o HtRegexList.o HtRegexReplace.o HtRegexReplaceList.o HtVectorGeneric.o HtMaxMin.o HtWordType.o md5.o filecopy.o WordType.o conf_lexer.o conf_parser.o)

#libhtdig c++ files
LIBHTDIG_CXX_OBJS = $(addprefix $(OBJS_DIR)/, libhtdig_htsearch.o libhtdig_htdig.o)

#htdig php wrapper
LIBHTDIGPHP_C_OBJS = $(addprefix $(OBJS_DIR)/, htdigphp.o)


#
# htcommon lex & yacc targets (not compiled in these targets, just created
# from lxx and yxx files. COMMON_CXX_OBJS has the actual objects)
#
LXX_TARGETS = $(addprefix $(SRC_DIR)/common/, conf_lexer.cc)
YXX_TARGETS = $(addprefix $(SRC_DIR)/common/, conf_parser.cc)


OBJS += $(BDB_C_OBJS) $(COMMON_C_OBJS) $(COMMON_CXX_OBJS)
OBJS += $(NETWORK_CXX_OBJS) $(SPIDER_CXX_OBJS)
OBJS += $(LIBHTDIG_CXX_OBJS) $(CLAPI_CXX_OBJS)

ALL_OBJS = $(OBJS) $(LIBHTDIGPHP_C_OBJS)


######################################################
#
#
# Definitions (probably should be moved to another file)
#
#
######################################################

#define your own defaults here!
DEFAULT_CONFIG_FILE=\"/etc/htdig/htdig.conf\"
DEFAULT_DB_PATH=\"/etc/htdig/\"
BIN_DIR=\"/usr/local/bin\"
COMMON_DIR=\"/usr/local/share/htdig\"
CONFIG_DIR=\"/etc/htdig\"
DATABASE_DIR=\"/var/lib/htdig\"
IMAGE_URL_PREFIX=\"/htdig\"

DEFS  = -DHAVE_CONFIG_H -DDEFAULT_CONFIG_FILE=$(DEFAULT_CONFIG_FILE) -DBIN_DIR=$(BIN_DIR) -DCOMMON_DIR=$(COMMON_DIR)
DEFS += -DCONFIG_DIR=$(CONFIG_DIR) -DDATABASE_DIR=$(DATABASE_DIR) -DIMAGE_URL_PREFIX=$(IMAGE_URL_PREFIX)
DEFS += -DLIBHTDIG
#DEFS += -DHAVE_MEMCMP -DHAVE_MEMCPY -DHAVE_MEMMOVE -DHAVE_STRERROR

#these are for compatability things...
DEFS+= -DHAVE_VSNPRINTF -DHAVE_MEMCPY

PHP_DEFS = $(DEFS) -DPHP_ATOM_INC -DCOMPILE_DL_HTDIG

#LEX define based on your system
LEX = flex
LEX_FLAGS = -L
LEX_OUTPUT_ROOT = lex.yy

#YACC define based on your system
YACC = bison -y
YACC_FLAGS = -l -d
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
#OPTS=$(OPTIMZ) -Wall -fno-rtti -fPIC
GCCOPTS=$(OPTIMZ) -Wall -fno-exceptions -fPIC
DBG= -g3 $(PROFILING)

ifdef FULLDEBUG
DBG+= -DDEBUG -DDEBUG_CMPR $(INC_DIRS)
endif

ifdef DEBUG
DBG+= -DDEBUG
endif

ifdef EXPKEY
OPTS += -DEXPKEY
endif

LFLAGS = -lgcc -lstdc++ 

#LIBZ  = -lz
#LIBZ  = /usr/lib/libz.a
#use this if you've built an -fPIC version of libz.a to link into the libhtdig.so
#LIBZ  = -lz-fpic




######################################################
#
#
# Individual targets
#
#
######################################################

libhtdig: $(OBJS) $(LXX_TARGETS) $(YXX_TARGETS) $(EXT_LIBS)
	$(CC) -shared $(OBJS) $(SSL_LIB) $(EXT_LIBS) $(LFLAGS) $(DBG) -o $(libhtdig)

libhtdigphp: $(ALL_OBJS) $(LXX_TARGETS) $(YXX_TARGETS) $(EXT_LIBS)
	$(CC) -shared $(ALL_OBJS) $(SSL_LIB) $(EXT_LIBS) $(LFLAGS) $(DBG) -o $(libhtdigphp)


clucene:
	@echo 
	@echo '---- Making CLucene ----'
	@echo '---- with RightNow modification ----'
	@perl -pi.bak -e 's/^postdeps="-lstdc\+\+ -lm -lgcc -lc -lgcc"/postdeps="-lm -lc"/g' $(CL_DIR)/libtool
	$(MAKE) -s -C $(CL_DIR)
	@perl -pi.bak -e 's/^postdeps="-lm -lc"/postdeps="-lstdc++ -lm -lgcc -lc -lgcc"/g' $(CL_DIR)/libtool
	@echo
	@echo '---- Making CLucene contributions----'
	@echo '---- with RightNow modification ----'
	@perl -pi.bak -e 's/^postdeps="-lstdc\+\+ -lm -lgcc -lc -lgcc"/postdeps="-lm -lc"/g' $(CL_CONTRIB_DIR)/libtool
	$(MAKE) -s -C $(CL_CONTRIB_DIR)
	@perl -pi.bak -e 's/^postdeps="-lm -lc"/postdeps="-lstdc++ -lm -lgcc -lc -lgcc"/g' $(CL_CONTRIB_DIR)/libtool
	@echo


tidy:
	@echo 
	@echo '---- Making HTMLTidy ----'
	$(MAKE) -s -C $(TIDY_DIR)
	@echo 



$(CLUCENE_LIB): $(CL_OBJS) $(CL_CONTRIB_OBJS)
	ar crus $(CLUCENE_LIB) $(CL_OBJS) $(CL_CONTRIB_OBJS)

$(TIDY_LIB): $(TIDY_OBJS)
	ar crus $(TIDY_LIB) $(TIDY_OBJS)

$(BDB_C_OBJS): 
	$(CC) -MM $(BDB_INC_DIR) $(BDB_DEFS) $(GCCOPTS) $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(BDB_DIR)/%.c) -MF $(@:$(OBJS_DIR)/%.o=$(DEPS_DIR)/%.d)
	$(CC) $(BDB_INC_DIR) $(BDB_DEFS) $(GCCOPTS) $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(BDB_DIR)/%.c) -o $@



$(COMMON_C_OBJS):
	$(CC) -MM $(INC_DIRS) $(DEFS) $(GCCOPTS) $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/common/%.c) -MF $(@:$(OBJS_DIR)/%.o=$(DEPS_DIR)/%.d)
	$(CC) $(INC_DIRS) $(DEFS) $(GCCOPTS) $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/common/%.c) -o $@

$(COMMON_CXX_OBJS): $(YXX_TARGETS) $(LXX_TARGETS)
	$(CXX) -MM $(INC_DIRS) $(DEFS) $(OPTS) $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/common/%.cc) -MF $(@:$(OBJS_DIR)/%.o=$(DEPS_DIR)/%.d)
	$(CXX) $(INC_DIRS) $(DEFS) $(OPTS) $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/common/%.cc) -o $@

$(NETWORK_CXX_OBJS):
	$(CXX) -MM $(INC_DIRS) $(DEFS) $(OPTS) $(SSL_INC) $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/network/%.cc) -MF $(@:$(OBJS_DIR)/%.o=$(DEPS_DIR)/%.d)
	$(CXX) $(INC_DIRS) $(DEFS) $(OPTS) $(SSL_INC) $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/network/%.cc) -o $@

$(CLAPI_CXX_OBJS):
	$(CXX) -MM -DHAVE_CONFIG_H $(INC_DIRS) $(DEFS) -fPIC -fno-rtti $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/CLuceneAPI/%.cc) -MF $(@:$(OBJS_DIR)/%.o=$(DEPS_DIR)/%.d)
	$(CXX) -DHAVE_CONFIG_H $(INC_DIRS) $(DEFS) -fPIC -fno-rtti $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/CLuceneAPI/%.cc) -o $@

$(SPIDER_CXX_OBJS):
	$(CXX) -MM $(INC_DIRS) $(DEFS) $(OPTS) $(SSL_INC) $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/spider/%.cc) -MF $(@:$(OBJS_DIR)/%.o=$(DEPS_DIR)/%.d)
	$(CXX) $(INC_DIRS) $(DEFS) $(OPTS) $(SSL_INC) $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/spider/%.cc) -o $@

$(LIBHTDIG_CXX_OBJS):
	$(CXX) -MM $(INC_DIRS) $(DEFS) $(SSL_INC) $(OPTS) -fexceptions $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/libhtdig/%.cc) -MF $(@:$(OBJS_DIR)/%.o=$(DEPS_DIR)/%.d)
	$(CXX) $(INC_DIRS) $(DEFS) $(SSL_INC) $(OPTS) -fexceptions $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/libhtdig/%.cc) -o $@

$(LIBHTDIGPHP_C_OBJS):
	$(CC) -MM $(PHP_INC_DIRS) $(PHP_DEFS) $(GCCOPTS) $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/libhtdig/%.c) -MF $(@:$(OBJS_DIR)/%.o=$(DEPS_DIR)/%.d)
	$(CC) $(PHP_INC_DIRS) $(PHP_DEFS) $(GCCOPTS) $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/libhtdig/%.c) -o $@

$(LXX_TARGETS): %.cc: %.lxx
	$(LEX) $(LEX_FLAGS) $< && mv $(LEX_OUTPUT_ROOT).c $@

$(YXX_TARGETS): %.cc: %.yxx
	$(YACC) $(YACC_FLAGS) $< && mv $(YACC_OUTPUT_ROOT).c $@
	if test -f y.tab.h; then mv -f y.tab.h $(SRC_DIR)/common/conf_parser.h ; fi

clean:
	@echo 'Cleaning dependencies'
	@rm -f $(patsubst $(OBJS_DIR)%.o, $(DEPS_DIR)%.d, $(ALL_OBJS))
	@echo 'Cleaning lexer targets'
	@rm -f $(LXX_TARGETS) $(YXX_TARGETS) $(SRC_DIR)/common/conf_parser.h
	@echo 'Cleaning objects'
	@rm -f $(ALL_OBJS)
	@echo 'Cleaning libraries'
	@rm -f $(libhtdig) $(libhtdigphp) $(CLUCENE_LIB) $(TIDY_LIB)
	
cleanall: clean
	@echo 
	@echo '---- Cleaning CLucene ----'
	@$(MAKE) -s -C $(CL_DIR) clean
	@echo 
	@echo '---- Cleaning CLucene contribs ----'
	@$(MAKE) -s -C $(CL_CONTRIB_DIR) clean
	@echo 
	@echo '---- Cleaning HTMLTidy ----'
	@$(MAKE) -s -C $(TIDY_DIR) clean
	@echo 

#depend stuff
-include $(patsubst $(OBJS_DIR)%.o, $(DEPS_DIR)%.d, $(ALL_OBJS))

#-include $(addprefix $(DEPDIR)/, $(subst ../,, $(SRC:.c=.d) $(CXXSRC:.cpp=.d)))
