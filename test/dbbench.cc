#ifdef HAVE_CONFIG_H
#include <htconfig.h>
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <fstream.h>
#include <fcntl.h>
#include <errno.h>

// If we have this, we probably want it.
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#include <malloc.h>
#include <stdlib.h>
#include <errno.h>

/* AIX requires this to be the first thing in the file.  */
#ifndef __GNUC__
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
#endif

#include <zlib.h>
#include <htString.h>
#include <db.h>

typedef struct {
  char* wordsfile;
  char* dbfile;
  char* find;
  int nwords;
  int loop;
  int page_size;
  int cache_size;
  int multiply_keys;
  int compress;
  int pool;
  int compress_test;
  int npage;
  int uncompress;
  int remove;
  int count;
} params_t;

static void dobench(params_t* params);
static void docompress(params_t* params);
static int verbose;

void usage();

//*****************************************************************************
// int main(int ac, char **av)
//
int main(int ac, char **av)
{
  int			c;
  extern char		*optarg;
  params_t		params;

  params.wordsfile = strdup("words.uniq");
  params.dbfile = strdup("test");
  params.nwords = -1;
  params.loop = 1;
  params.page_size = 4096;
  params.cache_size = 0;
  params.multiply_keys = 1;
  params.compress = 0;
  params.compress_test = 0;
  params.pool = 0;
  params.find = 0;
  params.npage = 0;
  params.remove = 0;
  params.count = 0;

  while ((c = getopt(ac, av, "vB:C:S:MZf:l:w:k:n:zp:ur:c:")) != -1)
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
	case 'C':
	  params.cache_size = atoi(optarg);
	  break;
	case 'S':
	  params.page_size = atoi(optarg);
	  break;
	case 'M':
	  params.pool = 1;
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
	case '?':
	  usage();
	  break;
	}
    }

  if(params.compress_test) {
    docompress(&params);
  } else {
    dobench(&params);
  }

  free(params.wordsfile);
  free(params.dbfile);
  if(params.find) free(params.find);

  return 0;
}

void dbput(DB* db, params_t* params, const String& key, const String& data)
{
    DBT	k, d;

    memset(&k, 0, sizeof(DBT));
    memset(&d, 0, sizeof(DBT));

    char* key_string = (char*)alloca(key.length());
    strcpy(key_string, key.get());
    k.data = key_string;
    k.size = key.length();

    char* data_string = (char*)alloca(data.length());
    strcpy(data_string, data.get());
    d.data = data_string;
    d.size = data.length();

    if((db->put)(db, NULL, &k, &d, 0) != 0)
      abort();
}

//*****************************************************************************
// void usage()
//   Display program usage information
//
void usage()
{
    cout << "usage: benchit [options]\n";
    cout << "Options:\n";
    cout << "\t-v\t\tIncreases the verbosity\n";
    cout << "\t-B dbfile\tuse <dbfile> as a db file name (default test).\n";
    cout << "\t-C <size>\tset cache size to <size>.\n";
    cout << "\t-S <size>\tset page size to <size>.\n";
    cout << "\t-M\t\tuse shared memory pool (default do not use).\n";
    cout << "\t-z\t\tSet DB_COMPRESS flag\n";
    cout << "\n";
    cout << "\t-f word\t\tfind word and display entries. If empty string show all.\n";
    cout << "\n";
    cout << "\t-r n\t\tRemove <n> first entries.\n";
    cout << "\n";
    cout << "\t-Z\t\tcompress blocks of existing dbfile.\n";
    cout << "\t-p n\t\ttest compress on first <n> pages (default all pages).\n";
    cout << "\t-u\t\tuncompress each page & check with original (default don't uncompress).\n";
    cout << "\n";
    cout << "\t-l loop\t\tread the word file loop times (default 1).\n";
    cout << "\t-w file\t\tRead words list from file (default words.uniq).\n";
    cout << "\t-k n\t\tcreate <n> entries for each word (default 1).\n";
    cout << "\t-n limit\tRead at most <limit> words (default read all).\n";
    cout << "\t-c count\tStart serial count at <count> (default 0).\n";
    exit(0);
}

/*
 * Comparison routine for the <int>string keys.
 */
static int
int_cmp(const DBT *a, const DBT *b)
{
  // First compare word
  size_t len = (a->size > b->size ? b->size : a->size) - sizeof(int);
  u_int8_t *p1, *p2;

  for (p1 = (u_int8_t*)a->data + sizeof(int), p2 = (u_int8_t*)b->data + sizeof(int); len--; ++p1, ++p2)
    if (*p1 != *p2)
      return ((long)*p1 - (long)*p2);

  // If words compare equal, compare document number
  if(a->size == b->size) {
    int ai, bi;
    memcpy((char*)&ai, ((char*)a->data), sizeof(int));
    memcpy((char*)&bi, ((char*)b->data), sizeof(int));

    return ai - bi;
  }

  return ((long)a->size - (long)b->size);
}

static void fill(DB* db, params_t* params)
{
  char buffer[50000];
  int count = params->count;
  int words_count;
  int i;

  cout << "Reading from " << params->wordsfile << " ... ";

  for(i = 0; i < params->loop; i++) {
  
    ifstream in(params->wordsfile);
    words_count = 0;

    while (!in.bad()) {
      in.getline(buffer, sizeof(buffer));
      if (in.eof())
	break;

      String line;
      line << buffer;
      line.chop("\r\n");

      for(int j = 0; j < params->multiply_keys; j++) {
	String key((char*)&count, sizeof(int));
	key.append(line.get(), line.length());
	dbput(db, params, key, "");
	count++;
      }
      words_count++;
      if(params->nwords > 0 && params->nwords <= words_count) break;
    }
  }

  cout << "pushed " << count << " words\n";
}

/*
 * Search for words.
 */
static void find(DB* db, params_t* params)
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
    if(verbose) {
      int docid;
      memcpy(&docid, key.data, sizeof(int));
      String word(((char*)key.data) + sizeof(int), key.size - sizeof(int));

      cout << "key: docid = " << docid << " word = " << word.get() << "\n";
    }

    key.flags = 0;
  } while(cursor->c_get(cursor, &key, &data, next) == 0);
  
  cursor->c_close(cursor);
}

/*
 * Delete keys
 */
static void remove(DB* db, params_t* params)
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

      cout << "key: docid = " << docid << " word = " << word.get() << "\n";
    }

    cursor->c_del(cursor, 0);
    removed++;
    if(params->remove < removed) break;
  } while(cursor->c_get(cursor, &key, &data, DB_NEXT) == 0);
  
  cursor->c_close(cursor);
}

/*
 * Prepare the ground for testing.
 */

static DB_ENV* db_init(params_t* params)
{
  DB_ENV *dbenv;
  char *progname = "benchit problem...";

  //
  // Rely on calloc to initialize the structure.
  //
  if ((dbenv = (DB_ENV *)calloc(sizeof(DB_ENV), 1)) == NULL)
    {
      fprintf(stderr, "%s: %s\n", progname, strerror(ENOMEM));
      exit (1);
    }
  dbenv->db_errfile = stderr;
  dbenv->db_errpfx = progname;
  int flags = DB_CREATE | DB_NOMMAP;
  if(params->pool) {
    dbenv->mp_size = params->cache_size;
    flags |= DB_INIT_MPOOL | DB_INIT_LOCK;
  }
     
  if ((errno = db_appinit(0, NULL, dbenv, flags)) != 0)
    {
      fprintf(stderr, "%s: db_appinit: %s\n", progname, strerror(errno));
      exit (1);
    }
  return (dbenv);
}

static void dobench(params_t* params)
{
  DB_ENV* dbenv = db_init(params);
  DB_INFO dbinfo;
  DB* db;

  memset(&dbinfo, 0, sizeof(dbinfo));

  // Note that prefix is disabled because bt_compare is set and
  // bt_prefix is not.
  dbinfo.bt_compare = int_cmp;

  if(params->page_size) dbinfo.db_pagesize = params->page_size;
  if(!params->pool && params->cache_size) dbinfo.db_cachesize = params->cache_size;

  int flags = DB_CREATE | DB_NOMMAP;
  if(params->compress) flags |= DB_COMPRESS;

  if(db_open(params->dbfile, DB_BTREE, flags, 0664, dbenv, &dbinfo, &db))
    abort();

  if(params->find) {
    find(db, params);
  } else if(params->remove) {
    remove(db, params);
  } else {
    fill(db, params);
  }

  (void)(db->close)(db, 0);
  (void)db_appexit(dbenv);

}

/*
 * Compress file one block after the other. Intended for mesuring the
 * compression overhead.
 */

extern "C" {
int __memp_cmpr_inflate(u_int8_t *, int, u_int8_t *, int);
int __memp_cmpr_deflate(u_int8_t *, int, u_int8_t **, int*);
}

int compressone(params_t* params, unsigned char* buffin, int buffin_length) {
  u_int8_t *buffout = 0;
  int buffout_length = 0;

  if(__memp_cmpr_deflate(buffin, buffin_length, &buffout, &buffout_length) != 0) {
    cout << "compressone: deflate failed\n";
    abort();
  }

  if(verbose) cout << "compressone: " << buffout_length << "\n";

  if(params->uncompress) {
    u_int8_t *bufftmp = (u_int8_t*)malloc(buffin_length);
    int bufftmp_length = buffin_length;
    
    if(__memp_cmpr_inflate(buffout, buffout_length, bufftmp, bufftmp_length) != 0) {
      cout << "compressone: inflate failed\n";
      abort();
    }
    if(bufftmp_length != buffin_length) abort();
    if(memcmp(bufftmp, buffin, bufftmp_length)) abort();

    free(bufftmp);
  }

  free(buffout);

  return buffout_length > (params->page_size / 2) ? 1 : 0;
}

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
  cout << "overflow: " << overflow << " out of " << count << "\n";
  
  close(in);
}
