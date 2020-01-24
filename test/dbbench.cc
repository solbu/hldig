//
// dbbench.cc
//
// dbbench: stress test the Berkeley DB database and WordList interface.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: dbbench.cc,v 1.13 2004/05/28 13:15:29 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include <hlconfig.h>
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
//#include <fcntl.h>  // included later, as non __STDC__ may #define open
#include <errno.h>

// If we have this, we probably want it.
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif /* HAVE_GETOPT_H */
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif /* HAVE_MALLOC_H */
#include <stdlib.h>

/* AIX requires this to be the first thing in the file.  */
//#ifndef __GNUC__  // Why not if g++?  Needed by g++ on Solaris 2.8
# if HAVE_ALLOCA_H
#  include <alloca.h>
# else
#  ifdef _AIX
#pragma alloca
#  else
#   ifndef alloca /* predefined by HP cc +Olibcalls */
char *alloca ();
#   endif
#  endif
# endif
//#endif

#include <htString.h>
#include <WordList.h>
#include <WordContext.h>
#include <db.h>

#include"HtTime.h"
#include"WordMonitor.h"

#define RAND() ((unsigned int) (1000.0*rand()/(RAND_MAX+1.0)))
/*
 * Store all options from the command line
 */
class params_t
{
public:
    char* wordsfile;
    char* dbfile;
    char* find;
    int nwords;
    int loop;
    DBTYPE type;
    int page_size;
    int cache_size;
    int multiply_keys;
    int wordlist;
    int compress;
    int pool;
    int compress_test;
    int npage;
    int uncompress;
    int remove;
    int count;
    int monitor;
    int random;
    void show()
    {
  printf("wordsfile:: %s\n", wordsfile);
  printf("dbfile:: %s\n", dbfile);
  printf("find:: %s\n", find);
  printf("nwords:: %d\n", nwords);
  printf("loop:: %d\n", loop);
  printf("page_size:: %d\n", page_size);
  printf("cache_size:: %d\n", cache_size);
  printf("multiply_keys:: %d\n", multiply_keys);
  printf("wordlist:: %d\n", wordlist);
  printf("compress:: %d\n", compress);
  printf("pool:: %d\n", pool);
  printf("compress_test:: %d\n", compress_test);
  printf("npage:: %d\n", npage);
  printf("uncompress:: %d\n", uncompress);
  printf("remove:: %d\n", remove);
  printf("count:: %d\n", count);
  printf("monitor:: %d\n", monitor);
   }
};

/*
 * Explain options
 */
static void usage();
/*
 * Verbosity level set with -v (++)
 */
static int verbose = 0;

// *****************************************************************
// Test framework
//
class Dbase {
public:
  Dbase(params_t* nparams) { params = nparams; }
  virtual ~Dbase() {}

  virtual void dbinit() = 0;
  void dobench();
  virtual void dbfinish() = 0;

  void fill();
  virtual void fill_one(String& line, int count) = 0;
  virtual void find() = 0;
  virtual void remove() = 0;

protected:
  params_t* params;
};

/*
 * Run function according to user specfied options.
 */
void Dbase::dobench()
{
  dbinit();

  if(params->find) {
    find();
  } else if(params->remove) {
    remove();
  } else {
    fill();
  }

  dbfinish();
}

/*
 * Generate a list of words from a file.
 * Call the fill_one function for each generated word.
 */
void Dbase::fill() {
#define FILL_BUFFER_SIZE (50*1024)
  char buffer[FILL_BUFFER_SIZE + 1];
  int count = params->count;
  int words_count;
  int i;

  fprintf(stderr, "Reading from %s ... ", params->wordsfile);

  for(i = 0; i < params->loop; i++) {

    FILE* in = fopen(params->wordsfile, "r");
    if(!in) {
      fprintf(stderr, "cannot open %s for reading : ", params->wordsfile);
      perror("");
      exit(1);
    }

    words_count = 0;

    while(fgets(buffer, FILL_BUFFER_SIZE, in)) {
      String line(buffer);
      line.chop("\r\n");

      for(int j = 0; j < params->multiply_keys; j++) {
  fill_one(line, count);
  count++;
      }
      words_count++;
      if(params->nwords > 0 && params->nwords <= words_count) break;
    }

    fclose(in);
  }

  fprintf(stderr, "pushed %d words\n", count);
}

// *****************************************************************
// Test Berkeley DB alone
//
class Dsimple : public Dbase {
public:
  Dsimple(params_t* nparams) : Dbase(nparams) { pad = 0; }
  virtual ~Dsimple() { if(pad) free(pad); }

  virtual void dbinit();
  void dbinit_env();
  void dbopen();
  virtual void dbfinish();

  virtual void fill_one(String& line, int count);
  virtual void find();
  virtual void remove();

  void dbput(const String& key, const String& data);

protected:
  DB_ENV* dbenv;
  DB* db;
  char* pad;
};

/*
 * Comparison routine for the <int>string keys.
 */
static int
int_cmp(const DBT *a, const DBT *b)
{
  // First compare word
  size_t len = (a->size > b->size ? b->size : a->size) - (sizeof(unsigned short) + sizeof(int));
  uint8_t *p1, *p2;

  for (p1 = (uint8_t*)a->data + sizeof(unsigned short) + sizeof(int), p2 = (uint8_t*)b->data + sizeof(unsigned short) + sizeof(int); len--; ++p1, ++p2)
    if (*p1 != *p2)
      return ((long)*p1 - (long)*p2);

  //
  // If words compare equal, compare numbers
  //
  if(a->size == b->size) {
    int ai, bi;
    memcpy((char*)&ai, ((char*)a->data + sizeof(unsigned short)), sizeof(int));
    memcpy((char*)&bi, ((char*)b->data + sizeof(unsigned short)), sizeof(int));

    if(ai - bi)
      return ai - bi;

    unsigned short as, bs;
    memcpy((char*)&as, ((char*)a->data), sizeof(unsigned short));
    memcpy((char*)&bs, ((char*)b->data), sizeof(unsigned short));

    return as - bs;
  }

  return ((long)a->size - (long)b->size);
}

/*
 * Init and Open the database
 */
void Dsimple::dbinit()
{
  dbinit_env();
  dbopen();
}

/*
 * Prepare the ground for testing.
 */
void Dsimple::dbinit_env()
{
  const char *progname = "dbbench problem...";

  Configuration* config = WordContext::Initialize();
  config->Add("wordlist_env_skip", "true");
  if(params->monitor)
    config->Add("wordlist_monitor", "true");
  WordContext::Initialize(*config);
  //
  // Make sure the size of a record used with raw Berkeley DB is equal to the
  // size of a record used with Word classes.
  //
  {
    pad = strdup("0123456789012345678900123456789012345678901234567890");
    //
    // Dsimple uses an int (unique count) and short (docid) in addition to the word
    //
    int pad_length = WordKeyInfo::Instance()->num_length - sizeof(unsigned short) - sizeof(int);
    if(pad_length > 0) {
      if(pad_length > (int)(strlen(pad) - 1)) {
  fprintf(stderr, "Not enough padding\n");
  exit(1);
      }
    } else {
      fprintf(stderr, "WordKey is always bigger than simulated key\n");
      exit(1);
    }
    pad[pad_length] = '\0';
  }

  int error;
  if((error = CDB_db_env_create(&dbenv, 0)) != 0) {
    fprintf(stderr, "%s: %s\n", progname, CDB_db_strerror(error));
    exit (1);
  }
  dbenv->set_errfile(dbenv, stderr);
  dbenv->set_errpfx(dbenv, progname);
  if(params->cache_size > 500 * 1024)
    dbenv->set_cachesize(dbenv, 0, params->cache_size, 0);
  int flags = DB_CREATE | DB_INIT_MPOOL | DB_INIT_LOCK | DB_NOMMAP;
  if(!params->pool)
    flags |= DB_PRIVATE;

  dbenv->open(dbenv, NULL, NULL, flags, 0666);
}

/*
 * Open of database after dbinit_env
 */
void Dsimple::dbopen()
{
  if(CDB_db_create(&db, dbenv, 0) != 0)
    exit(1);

  // Note that prefix is disabled because bt_compare is set and
  // bt_prefix is not.
  if(params->type == DB_BTREE) db->set_bt_compare(db, int_cmp);

  if(params->page_size) db->set_pagesize(db, params->page_size);

  int flags = DB_CREATE | DB_NOMMAP;
  if(params->compress)
    flags |= DB_COMPRESS;
  if(params->find)
    flags |= DB_RDONLY;

  if(db->open(db, params->dbfile, NULL, params->type, flags, 0666) != 0)
    exit(1);
}

/*
 * Close the database and free objects
 */
void Dsimple::dbfinish()
{
  (void)db->close(db, 0);
  (void)dbenv->close(dbenv, 0);
  WordContext::Finish();
}

/*
 * Create a key from the word in <line> and the unique count in <count>
 */
void Dsimple::fill_one(String& line, int count)
{
  unsigned short docid = params->random ? RAND() : ((count >> 16) & 0xff);
  String key((char*)&docid, sizeof(unsigned short));
  key.append((char*)&count, sizeof(int));
  key.append(line.get(), line.length());
  key.append(pad);
  dbput(key, "");
}

/*
 * Search for words.
 */
void Dsimple::find()
{
  int seqrc;
  DBC* cursor;
  DBT key;
  DBT data;

  if((seqrc = db->cursor(db, NULL, &cursor, 0)) != 0)
    abort();

  memset(&key, '\0', sizeof(DBT));
  memset(&data, '\0', sizeof(DBT));

  String word("\0\0\0\0", sizeof(int));
  int next;

  if(strlen(params->find) > 0) {
    word.append(params->find, strlen(params->find));
    key.data = word.get();
    key.size = word.length();
    cursor->c_get(cursor, &key, &data, DB_SET_RANGE);
    next = DB_NEXT_DUP;
  } else {
    cursor->c_get(cursor, &key, &data, DB_FIRST);
    next = DB_NEXT;
  }

  do {
    if(verbose == 1) {
      int docid;
      memcpy(&docid, key.data, sizeof(int));
      String word(((char*)key.data) + sizeof(int), key.size - sizeof(int));

      fprintf(stderr, "key: docid = %d word = %s\n", docid, (char*)word);
    }
    //
    // Straight dump of the entry
    //
    if(verbose > 1) {
      String k((const char*)key.data, (int)key.size);
      String d((const char*)data.data, (int)data.size);

      fprintf(stderr, "key: %s data: %s\n", (char*)k, (char*)d);
    }

    key.flags = 0;
  } while(cursor->c_get(cursor, &key, &data, next) == 0);

  cursor->c_close(cursor);
}

/*
 * Delete keys
 */
void Dsimple::remove()
{
  int seqrc;
  DBC* cursor;
  DBT key;
  DBT data;
  int removed = 0;

  if((seqrc = db->cursor(db, NULL, &cursor, 0)) != 0)
    abort();

  memset(&key, '\0', sizeof(DBT));
  memset(&data, '\0', sizeof(DBT));

  String word("\0\0\0\0", sizeof(int));

  cursor->c_get(cursor, &key, &data, DB_FIRST);

  do {
    if(verbose) {
      int docid;
      memcpy(&docid, key.data, sizeof(int));
      String word(((char*)key.data) + sizeof(int), key.size - sizeof(int));

      fprintf(stderr, "key: docid = %d word = %s\n", docid, (char*)word);
    }

    cursor->c_del(cursor, 0);
    removed++;
    if(params->remove < removed) break;
  } while(cursor->c_get(cursor, &key, &data, DB_NEXT) == 0);

  cursor->c_close(cursor);
}

/*
 * Wrap a key + data insertion from String to DBT
 */
void Dsimple::dbput(const String& key, const String& data)
{
    DBT  k, d;

    memset(&k, 0, sizeof(DBT));
    memset(&d, 0, sizeof(DBT));

    char* key_string = (char*)alloca(key.length());
    memcpy(key_string, key.get(),key.length());
    k.data = key_string;
    k.size = key.length();

    char* data_string = (char*)alloca(data.length());
    memcpy(data_string, data.get(),data.length());
    d.data = data_string;
    d.size = data.length();

    if((db->put)(db, NULL, &k, &d, 0) != 0)
      abort();
}

// *****************************************************************
// Test WordList
//
class Dwordlist : public Dbase {
public:
  Dwordlist(params_t* nparams) : Dbase(nparams) {}

  virtual void dbinit();
  virtual void dbfinish();

  virtual void fill_one(String& line, int count);
  virtual void find();
  virtual void remove();

  void dbput(const String& key, const String& data);

protected:
  WordList* words;
};

static Configuration*  config = 0;

/*
 * Init and Open the database
 */
void Dwordlist::dbinit()
{
    if(verbose) {
      fprintf(stderr, "Dwordlist::dbinit\n");
      params->show();
    }

    config = WordContext::Initialize();
    if(params->cache_size > 500 * 1024) {
      String str;
      str << params->cache_size;
      config->Add("wordlist_cache_size", str);
      if(verbose)
  fprintf(stderr, "setting cache size to: %s\n", (char*)str);
    }
    if(params->page_size) {
      String str;
      str << params->page_size;
      config->Add("wordlist_page_size", str);
      if(verbose)
  fprintf(stderr, "setting page size to: %s\n", (char*)str);
    }
    if(params->compress)
  config->Add("wordlist_compress", "true");
    if(params->monitor)
      config->Add("wordlist_monitor", "true");

    WordContext::Initialize(*config);

    words = new WordList(*config);

    if(verbose) WordKeyInfo::Instance()->Show();

    if(words->Open(params->dbfile, (params->find ? O_RDONLY : O_RDWR)) != OK)
      exit(1);
}

/*
 * Close the database and free objects
 */
void Dwordlist::dbfinish()
{
  delete words;
  WordContext::Finish();
}

/*
 * Create a key from the word in <line> and the unique count in <count>
 */
void Dwordlist::fill_one(String& line, int count)
{
    WordReference wordRef;
    WordKey& key = wordRef.Key();

    if(params->random) count = RAND();
    key.SetWord(line);
    key.Set(WORD_FIRSTFIELD, count >> 16);
    key.Set(WORD_FIRSTFIELD + 1, 0);
    key.Set(WORD_FIRSTFIELD + 2, count & 0xffff);

    words->Override(wordRef);
}

static int
wordlist_walk_callback_file_out(WordList *, WordDBCursor&, const WordReference *word, Object &)
{
  printf("%s\n", (char*)word->Get());
  return OK;
}

/*
 * Search for words.
 */
void Dwordlist::find()
{
  if(strlen(params->find) > 0) {
    Object data;
    WordKey key;
    key.SetWord(params->find);
    WordCursor *cursor = words->Cursor(key,
               wordlist_walk_callback_file_out,
               &data);
    cursor->Walk();
    delete cursor;
  } else {
    words->Write(stdout);
  }
}

/*
 * Delete keys
 */
void Dwordlist::remove()
{
}

#ifdef HAVE_LIBZ
static void docompress(params_t* params);
#endif /* HAVE_LIBZ */

// *****************************************************************************
// Entry point
//
int main(int ac, char **av)
{
  int      c;
  extern char    *optarg;
  params_t    params;

  params.wordsfile = strdup("words.uniq");
  params.dbfile = strdup("test");
  params.nwords = -1;
  params.loop = 1;
  params.type = DB_BTREE;
  params.page_size = 4096;
  params.cache_size = 0;
  params.multiply_keys = 1;
  params.compress = 0;
  params.wordlist = 0;
  params.compress_test = 0;
  params.pool = 0;
  params.find = 0;
  params.npage = 0;
  params.remove = 0;
  params.count = 0;
  params.monitor = 0;
  params.random = 0;

  while ((c = getopt(ac, av, "vB:T:C:S:MZf:l:w:k:n:zWp:ur:c:mR")) != -1)
    {
      switch (c)
  {
  case 'v':
    verbose++;
    break;
  case 'B':
    free(params.dbfile);
    params.dbfile = strdup(optarg);
    break;
  case 'T':
    if(!strcmp(optarg, "hash")) {
      params.type = DB_HASH;
    } else {
      params.type = DB_BTREE;
    }
    break;
  case 'C':
    params.cache_size = atoi(optarg);
    break;
  case 'S':
    params.page_size = atoi(optarg);
    break;
  case 'M':
    params.pool = 1;
    break;
  case 'W':
    params.wordlist = 1;
    break;
  case 'z':
    params.compress = 1;
    break;
  case 'f':
    params.find = strdup(optarg);
    break;
  case 'l':
    params.loop = atoi(optarg);
    break;
  case 'w':
    free(params.wordsfile);
    params.wordsfile = strdup(optarg);
    break;
  case 'k':
    params.multiply_keys = atoi(optarg);
    break;
  case 'n':
    params.nwords = atoi(optarg);
    break;
    break;
  case 'Z':
    params.compress_test = 1;
    break;
  case 'p':
    params.npage = atoi(optarg);
    break;
  case 'u':
    params.uncompress = 1;
    break;
  case 'r':
    params.remove = atoi(optarg);
    break;
  case 'c':
    params.count = atoi(optarg);
    break;
  case 'm':
    params.monitor = 1;
    break;
  case 'R':
    params.random = 1;
    break;
  case '?':
    usage();
    break;
  }
    }

  if(params.compress_test) {
#ifdef HAVE_LIBZ
    docompress(&params);
#else /* HAVE_LIBZ */
    fprintf(stderr, "compiled without zlib, compression test not available\n");
    exit(1);
#endif /* HAVE_LIBZ */
  } else {
    if(params.wordlist) {
      Dwordlist bench(&params);
      bench.dobench();
    } else {
      Dsimple bench(&params);
      bench.dobench();
    }
  }

  free(params.wordsfile);
  free(params.dbfile);
  if(params.find) free(params.find);

  return 0;
}

// *****************************************************************************
// void usage()
//   Display program usage information
//
static void usage()
{
    printf("usage: dbbench [options]\n");
    printf("Options:\n");
    printf("\t-v\t\tIncreases the verbosity\n");
    printf("\t-B dbfile\tuse <dbfile> as a db file name (default test).\n");
    printf("\t-T {hash|btree}\tfile structure (default btree).\n");
    printf("\t-C <size>\tset cache size to <size>.\n");
    printf("\t-S <size>\tset page size to <size>.\n");
    printf("\t-M\t\tuse shared memory pool (default do not use).\n");
    printf("\t-z\t\tSet DB_COMPRESS flag\n");
    printf("\t-R\t\tUse random number for numerical values\n");

    printf("\n");
    printf("\t-W\t\tuse WordList instead of raw Berkeley DB\n");
    printf("\n");
    printf("\t-f word\t\tfind word and display entries. If empty string show all.\n");
    printf("\t-m\t\tMonitor Word classes activity\n");
    printf("\n");
    printf("\t-r n\t\tRemove <n> first entries.\n");
    printf("\n");
    printf("\t-Z\t\tcompress blocks of existing dbfile.\n");
    printf("\t-p n\t\ttest compress on first <n> pages (default all pages).\n");
    printf("\t-u\t\tuncompress each page & check with original (default don't uncompress).\n");
    printf("\n");
    printf("\t-l loop\t\tread the word file loop times (default 1).\n");
    printf("\t-w file\t\tRead words list from file (default words.uniq).\n");
    printf("\t-k n\t\tcreate <n> entries for each word (default 1).\n");
    printf("\t-n limit\tRead at most <limit> words (default read all).\n");
    printf("\t-c count\tStart serial count at <count> (default 0).\n");
    exit(0);
}

#ifdef HAVE_LIBZ
/*
 * Compress file one block after the other. Intended for mesuring the
 * compression overhead.
 */

extern "C"
{
    extern int CDB___memp_cmpr_inflate(const uint8_t *, int, uint8_t * , int  , void *);
    extern int CDB___memp_cmpr_deflate(const uint8_t *, int, uint8_t **, int *, void *);
}

int compressone(params_t* params, unsigned char* buffin, int buffin_length) {
  uint8_t *buffout = 0;
  int buffout_length = 0;

  if(CDB___memp_cmpr_deflate(buffin, buffin_length, &buffout, &buffout_length,NULL) != 0) {
    printf("compressone: deflate failed\n");
    abort();
  }

  if(verbose) fprintf(stderr, "compressone: %d\n", buffout_length);

  if(params->uncompress) {
    uint8_t *bufftmp = (uint8_t*)malloc(buffin_length);
    int bufftmp_length = buffin_length;

    if(CDB___memp_cmpr_inflate(buffout, buffout_length, bufftmp, bufftmp_length,NULL) != 0) {
      fprintf(stderr, "compressone: inflate failed\n");
      abort();
    }
    if(bufftmp_length != buffin_length) abort();
    if(memcmp(bufftmp, buffin, bufftmp_length)) abort();

    free(bufftmp);
  }

  free(buffout);

  return buffout_length > (params->page_size / 2) ? 1 : 0;
}

#include <fcntl.h>  // if included at top, db->open may have caused problems

static void docompress(params_t* params)
{
  if(params->page_size == 0) params->page_size = 4096;
  int in = open(params->dbfile, O_RDONLY);
  unsigned char* buffin = (unsigned char*)malloc(params->page_size);
  int read_count;
  int overflow = 0;
  int count = 0;

  while((read_count = read(in, buffin, params->page_size)) == params->page_size) {
    overflow += compressone(params, buffin, params->page_size);
    if(params->npage > 1 && params->npage <= count) break;
    count++;
  }
  printf("overflow: %d out of %d\n", overflow, count);

  close(in);
}
#endif /* HAVE_LIBZ */
