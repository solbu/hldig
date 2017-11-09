//
// WordDBCache.h
//
// NAME
// intermediate cache for WordList objects. 
//
// SYNOPSIS
//
// Internal helper for the WordListOne object.
//
// DESCRIPTION
//
// To speed up bulk insertions, the WordDBCache allows them to remain in
// memory as long as a given limit is not reached. The inserted entries
// are them sorted and dumped into a file. When a given number of files
// have been produced, they are merged into one. Eventually the resulting
// list of entries is inserted into the WordList index.
//
// 
// END
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: WordDBCache.h,v 1.4 2004/05/28 13:15:26 lha Exp $
//

#ifndef _WordDBCache_h_
#define _WordDBCache_h_

#include <stdlib.h>
#include <errno.h>

#include "htString.h"
#include "List.h"
#include "db.h"
#include "lib.h"
#include "myqsort.h"
#include "WordList.h"

class WordDB;
class WordLock;

//
// Minimum size of the pulsing cache
//
#define WORD_DB_CACHE_MINIMUM	(500 * 1024)

//
// We could use DBT instead but it's more than two times bigger and
// time saving by the most efficient way of memory space is the whole
// point of the cache.
//
class WordDBCacheEntry {
public:
  char* key;
  unsigned int key_size;
  char* data;
  unsigned int data_size;
};

class WordDBCache {
public:
  inline WordDBCache(WordContext* ncontext) {
    context = ncontext;

    entries = (WordDBCacheEntry*)malloc(1000 * sizeof(WordDBCacheEntry));
    entries_length = 0;
    entries_size = 1000;

    pool = (char*)malloc(WORD_DB_CACHE_MINIMUM);
    pool_length = 0;
    pool_size = pool_max = WORD_DB_CACHE_MINIMUM;
  }

  inline ~WordDBCache() {
    if(pool_length > 0) {
      fprintf(stderr, "WordDBCache::~WordDBCache: destructor called and cache not empty\n");
    }
    free(entries);
    free(pool);
  }

  inline int ResizeEntries() {
    entries_size *= 2;
    entries = (WordDBCacheEntry*)realloc(entries, entries_size * sizeof(WordDBCacheEntry));
    return entries ? 0 : DB_RUNRECOVERY;
  }

  inline int ResizePool(int wanted) {
    if(pool_size * 2 > pool_max) {
      if(pool_max > pool_size && pool_max > wanted)
	pool_size = pool_max;
      else
	return ENOMEM;
    } else {
      pool_size *= 2;
    }
    pool = (char*)realloc(pool, pool_size);
    return pool ? 0 : DB_RUNRECOVERY;
  }

  inline int Allocate(int size) {
    int ret;
    if(entries_length >= entries_size)
      if((ret = ResizeEntries()) != 0)
	return ret;
    if(pool_length + size >= pool_size) {
      if((ret = ResizePool(pool_length + size)) != 0)
	return ret;
    }
    return 0;
  }

  inline int GetMax() const { return pool_max; }

  inline int SetMax(int max) {
    if(max > pool_max)
      pool_max = max;
    return 0;
  }

  inline int SetCompare(int (*ncompare)(WordContext *, const WordDBCacheEntry *, const WordDBCacheEntry *)) {
    compare = ncompare;
    return 0;
  }

  inline int Sort() {
    if(Absolute() != OK) return NOTOK;
    //
    // Reorder entries in increasing order
    //
    myqsort((void*)entries, entries_length, sizeof(WordDBCacheEntry), (myqsort_cmp)compare, (void*)context);
    return 0;
  }

  inline int Relative() {
    int i;
    for(i = 0; i < entries_length; i++) {
      entries[i].key = (char*)(entries[i].key - pool);
      entries[i].data = (char*)(entries[i].data - pool);
    }
    return OK;
  }
  
  inline int Absolute() {
    int i;
    for(i = 0; i < entries_length; i++) {
      entries[i].key = pool + (int)(entries[i].key);
      entries[i].data = pool + (int)(entries[i].data);
    }
    return OK;
  }

  inline int Entries(WordDBCacheEntry*& nentries, int& nentries_length) {
    nentries = entries;
    nentries_length = entries_length;
    return 0;
  }

  inline int Pool(char*& npool, int& npool_length) {
    npool = pool;
    npool_length = pool_length;
    return OK;
  }
  
  inline int Add(char* key, int key_size, char* data, int data_size) {
    int ret;
    if((ret = Allocate(key_size + data_size)) != 0)
      return ret;

    entries[entries_length].key = (char*)pool_length;
    entries[entries_length].key_size = key_size;
    entries[entries_length].data = (char*)(pool_length + key_size);
    entries[entries_length].data_size = data_size;
    entries_length++;
    memcpy(pool + pool_length, key, key_size);
    memcpy(pool + pool_length + key_size, data, data_size);
    pool_length += key_size + data_size;

    return 0;
  }

  inline int Flush() {
    entries_length = 0;
    pool_length = 0;
    return 0;
  }

  inline int Empty() {
    return entries_length <= 0;
  }
  
private:
  WordDBCacheEntry* entries;
  int entries_length;
  int entries_size;

  char* pool;
  int pool_length;
  int pool_size;
  int pool_max;

  int (*compare)(WordContext *, const WordDBCacheEntry *, const WordDBCacheEntry *);
  WordContext *context;
};

class WordDBCacheFile : public Object 
{
public:
  WordDBCacheFile() { size = 0; }

  String filename;
  unsigned int size;
};

class WordDBCaches {
 public:
  inline WordDBCaches(WordList* nwords, int nfile_max, int size_hint, int nsize_max) : cache(nwords->GetContext()) {
    words = nwords;

    files = new WordDB(words->GetContext()->GetDBInfo());
    files->Open(words->Filename(), "tmp", DB_BTREE, words->Flags(), 0666, WORD_DB_FILES);
    file_max = nfile_max;
    size_max = nsize_max;
    lock = 0;

    cache.SetMax(size_hint / 2);
  }

  ~WordDBCaches() {
    delete files;
  }

  int Full() const { return size_max > 0 ? size >= size_max : 0; }

  int Add(char* key, int key_size, char* data, int data_size);
  int AddFile(String& filename);

  int CacheFlush();

  int Merge();
  int Merge(const String& filea, const String& fileb, const String& tmpname);
  int Merge(WordDB& db);

  int CacheWrite(const String& filename);
  int CacheCompare(int (*compare)(WordContext *, const WordDBCacheEntry *, const WordDBCacheEntry *)) { cache.SetCompare(compare); return OK; }

  int WriteEntry(FILE* fp, WordDBCacheEntry& entry, unsigned char*& buffer, unsigned int& buffer_size);
  int ReadEntry(FILE* fp, WordDBCacheEntry& entry, unsigned char*& buffer, unsigned int& buffer_size);

 private:
  WordList*		words;

  WordDB*            	files;
  int			file_max;
  int			size_max;
  int			size;
  
  WordLock*		lock;
  WordDBCache		cache;
};

#endif /* _WordDBCache_h */
