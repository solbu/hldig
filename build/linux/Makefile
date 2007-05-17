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

LIBHTDIG_BUILD_VER = 4.1.1

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
ifndef PHP4
PHP_INC_DIRS = $(INC_DIRS) -I/nfs/local/linux/include/php-5.2.2-82/ -I/nfs/local/linux/include/php-5.2.2-82/main -I/nfs/local/linux/include/php-5.2.2-82/Zend -I/nfs/local/linux/include/php-5.2.2-82/TSRM
else
PHP_INC_DIRS = $(INC_DIRS) -I/nfs/local/linux/include/php-4.1.2-80/ -I/nfs/local/linux/include/php-4.1.2-80/main -I/nfs/local/linux/include/php-4.1.2-80/Zend -I/nfs/local/linux/include/php-4.1.2-80/TSRM
endif

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
BDB_C_SRC = $(addprefix BDB/, bt_compare.c bt_conv.c bt_curadj.c bt_cursor.c bt_delete.c bt_method.c bt_open.c bt_put.c bt_rec.c bt_reclaim.c bt_recno.c bt_rsearch.c bt_search.c bt_split.c bt_stat.c bt_upgrade.c btree_auto.c crdel_auto.c crdel_rec.c db.c db_am.c db_auto.c db_byteorder.c db_conv.c db_dispatch.c db_dup.c db_err.c db_getlong.c db_iface.c db_join.c db_log2.c db_meta.c db_method.c db_overflow.c db_pr.c db_rec.c db_reclaim.c db_ret.c db_salloc.c db_shash.c db_upgrade.c env_method.c env_open.c env_recover.c env_region.c hash.c hash_auto.c hash_conv.c hash_dup.c hash_func.c hash_meta.c hash_method.c hash_page.c hash_rec.c hash_reclaim.c hash_stat.c hash_upgrade.c lock.c lock_conflict.c lock_deadlock.c lock_region.c lock_util.c log.c log_archive.c log_auto.c log_compare.c log_findckp.c log_get.c log_method.c log_put.c log_rec.c log_register.c mp_alloc.c mp_bh.c mp_fget.c mp_fopen.c mp_fput.c mp_fset.c mp_method.c mp_region.c mp_register.c mp_stat.c mp_sync.c mp_trickle.c mut_fcntl.c mut_pthread.c mut_tas.c mutex.c os_abs.c os_alloc.c os_dir.c os_errno.c os_fid.c os_finit.c os_fsync.c os_handle.c os_map.c os_method.c os_oflags.c os_open.c os_region.c os_rename.c os_root.c os_rpath.c os_rw.c os_seek.c os_sleep.c os_spin.c os_stat.c os_tmpdir.c os_unlink.c qam.c qam_auto.c qam_conv.c qam_method.c qam_open.c qam_rec.c qam_stat.c txn.c txn_auto.c txn_rec.c txn_region.c xa.c xa_db.c xa_map.c)
# mp_cmpr.o
BDB_C_OBJS = $(addprefix $(OBJS_DIR)/, $(addsuffix .o, $(basename $(BDB_C_SRC) ) ) )

#htnet c++ files
NETWORK_CXX_SRC = $(addprefix network/, Connection.cc Transport.cc HtHTTP.cc HtFile.cc HtNNTP.cc HtCookie.cc HtCookieJar.cc HtCookieMemJar.cc HtHTTPBasic.cc HtHTTPSecure.cc SSLConnection.cc HtFTP.cc HtCookieInFileJar.cc)
NETWORK_CXX_OBJS = $(addprefix $(OBJS_DIR)/, $(addsuffix .o, $(basename $(NETWORK_CXX_SRC) ) ) )

#htdig spider c++ files
SPIDER_CXX_SRC = $(addprefix spider/, ExternalTransport.cc Spider.cc Document.cc URLRef.cc Server.cc TidyParser.cc SitemapParser.cc FacetCollection.cc ExternalParser.cc)
SPIDER_CXX_OBJS = $(addprefix $(OBJS_DIR)/, $(addsuffix .o, $(basename $(SPIDER_CXX_SRC) ) ) )

# CLuceneAPI files
CLAPI_CXX_SRC = $(addprefix CLuceneAPI/, CLuceneAPI.cc )
CLAPI_CXX_OBJS = $(addprefix $(OBJS_DIR)/, $(addsuffix .o, $(basename $(CLAPI_CXX_SRC) ) ) )

#htlib c files  $(REGEX)
COMMON_C_SRC = $(addprefix common/, getcwd.c mhash_md5.c regex.c vsnprintf.c memcmp.c mktime.c snprintf.c memcpy.c myqsort.c strerror.c memmove.c raise.c timegm.c)
COMMON_C_OBJS = $(addprefix $(OBJS_DIR)/, $(addsuffix .o, $(basename $(COMMON_C_SRC) ) ) )

#htcommon c++ files
COMMON_CXX_SRC = $(addprefix common/, HtStdHeader.cc IndexDBRef.cc IndexDB.cc DocumentRef.cc defaults.cc HtURLCodec.cc URL.cc URLTrans.cc cgi.cc HtSGMLCodec.cc HtConfiguration.cc HtURLRewriter.cc HtDebug.cc Configuration.cc Database.cc Dictionary.cc DB2_db.cc IntObject.cc List.cc Object.cc ParsedString.cc Queue.cc QuotedStringList.cc Stack.cc String.cc StringList.cc StringMatch.cc String_fmt.cc good_strtok.cc strcasecmp.cc strptime.cc HtCodec.cc HtWordCodec.cc HtVector.cc HtHeap.cc HtPack.cc HtDateTime.cc HtRegex.cc HtRegexList.cc HtRegexReplace.cc HtRegexReplaceList.cc HtVectorGeneric.cc HtMaxMin.cc HtWordType.cc md5.cc filecopy.cc WordType.cc)
COMMON_CXX_OBJS = $(addprefix $(OBJS_DIR)/, $(addsuffix .o, $(basename $(COMMON_CXX_SRC) ) ) )

#libhtdig c++ files
LIBHTDIG_CXX_SRC = $(addprefix libhtdig/, libhtdig_htsearch.cc libhtdig_htdig.cc)
LIBHTDIG_CXX_OBJS = $(addprefix $(OBJS_DIR)/, $(addsuffix .o, $(basename $(LIBHTDIG_CXX_SRC) ) ) )

#htdig php wrapper
LIBHTDIGPHP_C_SRC = $(addprefix libhtdig/, htdigphp.c)
LIBHTDIGPHP_C_OBJS = $(addprefix $(OBJS_DIR)/, $(addsuffix .o, $(basename $(LIBHTDIGPHP_C_SRC) ) ) )


#
# htcommon lex & yacc targets (not compiled in these targets, just created
# from lxx and yxx files. COMMON_CXX_OBJS has the actual objects)
#
LXX_SRC = $(addprefix common/, conf_lexer.lxx)
LXX_OBJS = $(addprefix $(OBJS_DIR)/, $(addsuffix .o, $(basename $(LXX_SRC) ) ) )
YXX_SRC = $(addprefix common/, conf_parser.yxx)
YXX_OBJS = $(addprefix $(OBJS_DIR)/, $(addsuffix .o, $(basename $(YXX_SRC) ) ) )


OBJS += $(BDB_C_OBJS) $(COMMON_C_OBJS) $(COMMON_CXX_OBJS)
OBJS += $(NETWORK_CXX_OBJS) $(SPIDER_CXX_OBJS)
OBJS += $(LIBHTDIG_CXX_OBJS) $(CLAPI_CXX_OBJS)
OBJS += $(YXX_OBJS) $(LXX_OBJS)

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

libhtdig: tidy clucene $(OBJS) $(LXX_TARGETS) $(YXX_TARGETS) $(EXT_LIBS)
	$(CC) -shared $(OBJS) $(SSL_LIB) $(EXT_LIBS) $(LFLAGS) $(DBG) -o $(libhtdig)

libhtdigphp: tidy clucene $(ALL_OBJS) $(LXX_TARGETS) $(YXX_TARGETS) $(EXT_LIBS)
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

$(BDB_C_OBJS): $(OBJS_DIR)/%.o: $(ROOT_DIR)/%.c
	$(CC) -MM $(BDB_INC_DIR) $(BDB_DEFS) $(GCCOPTS) $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(ROOT_DIR)/%.c) -MF $(@:$(OBJS_DIR)/%.o=$(DEPS_DIR)/%.d)
	$(CC) $(BDB_INC_DIR) $(BDB_DEFS) $(GCCOPTS) $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(ROOT_DIR)/%.c) -o $@

$(COMMON_C_OBJS): $(OBJS_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -MM $(INC_DIRS) $(DEFS) $(GCCOPTS) $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/%.c) -MF $(@:$(OBJS_DIR)/%.o=$(DEPS_DIR)/%.d)
	$(CC) $(INC_DIRS) $(DEFS) $(GCCOPTS) $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/%.c) -o $@

$(COMMON_CXX_OBJS): $(OBJS_DIR)/%.o: $(SRC_DIR)/%.cc
	$(CXX) -MM $(INC_DIRS) $(DEFS) $(OPTS) $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/%.cc) -MF $(@:$(OBJS_DIR)/%.o=$(DEPS_DIR)/%.d)
	$(CXX) $(INC_DIRS) $(DEFS) $(OPTS) $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/%.cc) -o $@

$(NETWORK_CXX_OBJS):  $(OBJS_DIR)/%.o: $(SRC_DIR)/%.cc
	$(CXX) -MM $(INC_DIRS) $(DEFS) $(OPTS) $(SSL_INC) $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/%.cc) -MF $(@:$(OBJS_DIR)/%.o=$(DEPS_DIR)/%.d)
	$(CXX) $(INC_DIRS) $(DEFS) $(OPTS) $(SSL_INC) $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/%.cc) -o $@

$(CLAPI_CXX_OBJS):  $(OBJS_DIR)/%.o: $(SRC_DIR)/%.cc
	$(CXX) -MM -DHAVE_CONFIG_H $(INC_DIRS) $(DEFS) -fPIC -fno-rtti $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/%.cc) -MF $(@:$(OBJS_DIR)/%.o=$(DEPS_DIR)/%.d)
	$(CXX) -DHAVE_CONFIG_H $(INC_DIRS) $(DEFS) -fPIC -fno-rtti $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/%.cc) -o $@

$(SPIDER_CXX_OBJS): $(OBJS_DIR)/%.o: $(SRC_DIR)/%.cc
	$(CXX) -MM $(INC_DIRS) $(DEFS) $(OPTS) $(SSL_INC) $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/%.cc) -MF $(@:$(OBJS_DIR)/%.o=$(DEPS_DIR)/%.d)
	$(CXX) $(INC_DIRS) $(DEFS) $(OPTS) $(SSL_INC) $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/%.cc) -o $@

$(LIBHTDIG_CXX_OBJS):  $(OBJS_DIR)/%.o: $(SRC_DIR)/%.cc
	$(CXX) -MM $(INC_DIRS) $(DEFS) $(SSL_INC) $(OPTS) -fexceptions $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/%.cc) -MF $(@:$(OBJS_DIR)/%.o=$(DEPS_DIR)/%.d)
	$(CXX) $(INC_DIRS) $(DEFS) $(SSL_INC) $(OPTS) -fexceptions $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)//%.cc) -o $@

$(LIBHTDIGPHP_C_OBJS): $(OBJS_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -MM $(PHP_INC_DIRS) $(PHP_DEFS) $(GCCOPTS) $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/%.c) -MF $(@:$(OBJS_DIR)/%.o=$(DEPS_DIR)/%.d)
	$(CC) $(PHP_INC_DIRS) $(PHP_DEFS) $(GCCOPTS) $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/%.c) -o $@

$(LXX_OBJS): $(OBJS_DIR)/%.o: $(SRC_DIR)/%.lxx
	$(LEX) $(LEX_FLAGS) $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/%.lxx) && mv $(LEX_OUTPUT_ROOT).c $(@:.o=.cc)
	$(CXX) -MM $(INC_DIRS) $(DEFS) $(OPTS) $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/%.cc) -MF $(@:$(OBJS_DIR)/%.o=$(DEPS_DIR)/%.d)
	$(CXX) $(INC_DIRS) $(DEFS) $(OPTS) $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/%.cc) -o $@

$(YXX_OBJS): $(OBJS_DIR)/%.o: $(SRC_DIR)/%.yxx
	$(YACC) $(YACC_FLAGS) $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/%.yxx) && mv $(YACC_OUTPUT_ROOT).c $(@:.o=.cc)
	if test -f y.tab.h; then mv -f y.tab.h $(SRC_DIR)/common/conf_parser.h ; fi
	$(CXX) -MM $(INC_DIRS) $(DEFS) $(OPTS) $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/%.cc) -MF $(@:$(OBJS_DIR)/%.o=$(DEPS_DIR)/%.d)
	$(CXX) $(INC_DIRS) $(DEFS) $(OPTS) $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/%.cc) -o $@

#$(LEXYACC_CXX_OBJS): $(YXX_TARGETS) $(LXX_TARGETS)
#	$(CXX) -MM $(INC_DIRS) $(DEFS) $(OPTS) $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/common/%.cc) -MF $(@:$(OBJS_DIR)/%.o=$(DEPS_DIR)/%.d)
#	$(CXX) $(INC_DIRS) $(DEFS) $(OPTS) $(DBG) -c $(@:$(OBJS_DIR)/%.o=$(SRC_DIR)/common/%.cc) -o $@

clean:
	@echo 'Cleaning dependencies'
	@rm -f $(patsubst $(OBJS_DIR)%.o, $(DEPS_DIR)%.d, $(ALL_OBJS))
	@echo 'Cleaning lexer targets'
	@rm -f $(LXX_TARGETS) $(YXX_TARGETS) $(SRC_DIR)/common/conf_parser.h
	@echo 'Cleaning objects'
	@rm -f $(ALL_OBJS)
	@echo 'Cleaning libraries'
	@rm -f $(libhtdig) $(libhtdigphp) $(CLUCENE_LIB) $(TIDY_LIB)
	rm -rf $(DEPS_DIR) $(OBJS_DIR)
	mkdir  $(DEPS_DIR) $(OBJS_DIR)
	mkdir  $(DEPS_DIR)/CLuceneAPI $(DEPS_DIR)/spider $(DEPS_DIR)/libhtdig $(DEPS_DIR)/network $(DEPS_DIR)/common $(DEPS_DIR)/BDB
	mkdir  $(OBJS_DIR)/CLuceneAPI $(OBJS_DIR)/spider $(OBJS_DIR)/libhtdig $(OBJS_DIR)/network $(OBJS_DIR)/common $(OBJS_DIR)/BDB
	
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
