//
// WordListMulti.cc
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: WordListMulti.cc,v 1.4 2003/06/24 19:57:27 nealr Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "WordListMulti.h"
#include "WordListOne.h"
#include "myqsort.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>

#ifndef _MSC_VER //_WIN32
#include <unistd.h>
#endif

class WordDBMulti : public Object 
{
public:
  WordDBMulti() { words = 0; size = 0; mode = 0; }

  WordListOne *words;
  String filename;
  int mode;
  unsigned int size;
};

// *****************************************************************************
//
WordListMulti::WordListMulti(WordContext* ncontext)
{
  dbs = new List;
  context = ncontext;
  // The database itself hasn't been opened yet
  isopen = 0;
  Configuration& config = context->GetConfiguration();
  extended = config.Boolean("wordlist_extend");
  verbose =  config.Value("wordlist_verbose");

  file_max =  config.Value("wordlist_multi_max", 50);
  if(file_max < 4) file_max = 4;

  file_min =  config.Value("wordlist_multi_min", 4);
  if(file_min < 2) file_min = 2;

  if(file_max < file_min) file_max = file_min * 2;

  put_max =  config.Value("wordlist_multi_put_max", 1000);
  if(put_max < 50) put_max = 50;

  compressor = 0;
  serial = 0;
}

// *****************************************************************************
//
WordListMulti::~WordListMulti()
{
  Close();
}

// *****************************************************************************
//
int WordListMulti::Open(const String& nfilename, int mode)
{
  filename = nfilename;

  char tmp[32];
  struct stat stat_buf;
  int i;
  //
  // Open existing indexes
  //
  for(i = 0; i < file_max; i++) {
    String filename_one(filename);
    sprintf(tmp, "%08d", i);
    filename_one << tmp;
    if(stat((char*)filename_one, &stat_buf) == 0) {
      WordDBMulti* db = new WordDBMulti();
      db->words = new WordListOne(context);
      db->filename = filename_one;
      db->mode = mode;
      dbs->Push(db);
    } else {
      break;
    }
  }
  serial = i;
  //
  // If no indexes exists and read-only, abort
  //
  if(i == 0 && (flags & DB_RDONLY)) {
    fprintf(stderr, "WordListMulti::Open(%s, O_RDONLY): no index found\n", (char*)filename);
    return NOTOK;
  }

  isopen = 1;

  //
  // If no indexes exists and read/write, create the first
  //
  if(i == 0)
    if(AddIndex() != OK) return NOTOK;

  WordDBMulti* db = (WordDBMulti*)dbs->Last();
  if(db->words->Open(db->filename, mode) != OK)
    return NOTOK;

  return OK;
}

// *****************************************************************************
//
int WordListMulti::Close()
{
  if(isopen) {
    WordDBMulti* db;
    ListCursor cursor;
    for(dbs->Start_Get(cursor); (db = (WordDBMulti*)dbs->Get_Next(cursor));) {
      delete db->words;
    }
    dbs->Destroy();
    isopen = 0;
    filename.trunc();
  }
  return OK;
}

// ****************************************************************************
//
unsigned int WordListMulti::Size() const 
{
  unsigned int size = 0;
  if(isopen) {
    WordDBMulti* db;
    ListCursor cursor;
    for(dbs->Start_Get(cursor); (db = (WordDBMulti*)dbs->Get_Next(cursor));) {
      if(!db->words->isopen) {
	if(db->words->Open(db->filename, O_RDONLY) != OK) return 0;
	size += db->words->Size();
	if(db->words->Close() != OK) return 0;
      } else {
	size += db->words->Size();
      }
    }
  }
  return size;
}

int WordListMulti::AddIndex()
{
  if(Flags() & O_RDONLY) return NOTOK;

  if(serial >= file_max)
    Merge();

  char tmp[32];

  String filename_one(filename);
  sprintf(tmp, "%08d", serial);
  filename_one << tmp;
  serial++;

  WordDBMulti* db = new WordDBMulti();
  db->words = new WordListOne(context);
  db->words->extended = extended;
  db->filename = filename_one;
  dbs->Push(db);

  return OK;
}

static int merge_cmp_size(WordListMulti*, WordDBMulti* a, WordDBMulti* b)
{
  return b->size - a->size;
}

static int merge_cmp_filename(WordListMulti*, WordDBMulti* a, WordDBMulti* b)
{
  return a->filename.compare(b->filename);
}

int WordListMulti::Merge()
{
  if(Flags() & DB_RDONLY) return NOTOK;

  Configuration& config = context->GetConfiguration();
  int use_compress = config.Boolean("wordlist_compress");

  WordDBMulti* db = (WordDBMulti*)dbs->Last();
  if(db->words->Close() != OK) return NOTOK;

  //
  // heap lists all the files in decreasing size order (biggest first)
  //
  WordDBMulti* heap = new WordDBMulti[serial];
  {
    int i;
    WordDBMulti* db;
    ListCursor cursor;
    for(i = 0, dbs->Start_Get(cursor); (db = (WordDBMulti*)dbs->Get_Next(cursor)); i++) {
      if(db->words->Open(db->filename, O_RDONLY) != OK) return NOTOK;
      db->size = db->words->Size();
      if(db->words->Close() != OK) return NOTOK;
      
      heap[i] = *db;
    }
    dbs->Destroy();
    myqsort((void*)heap, serial, sizeof(WordDBMulti), (myqsort_cmp)merge_cmp_size, (void*)this);
  }
  
  String tmpname = filename;
  tmpname << ".tmp";

  while(serial > file_min) {
    WordDBMulti* a = &heap[serial - 1];
    WordDBMulti* b = &heap[serial - 2];

    WordListOne tmp(context);
    tmp.extended = 0;

    if(a->words->Open(a->filename, O_RDONLY) != OK) return NOTOK;
    if(b->words->Open(b->filename, O_RDONLY) != OK) return NOTOK;
    if(tmp.Open(tmpname, O_RDWR) != OK) return NOTOK;
    if(tmp.db->CacheP() && tmp.db->CacheOff() != 0) return OK;

    WordDBCursor* cursora = a->words->db->Cursor();
    WordDBCursor* cursorb = b->words->db->Cursor();

    if(cursora->Open() != 0) return NOTOK;
    String keya;
    String dataa;

    if(cursorb->Open() != 0) return NOTOK;
    String keyb;
    String datab;

    int reta;
    int retb;

    reta = cursora->Get(keya, dataa, DB_NEXT);
    retb = cursorb->Get(keyb, datab, DB_NEXT);
      
      //
      // Merge while there are entries in both indexes
      //
    while(reta == 0 && retb == 0) {
      //
      // If keya lower than keyb
      //
      if(WordKey::Compare(context, keya, keyb) < 0) {
	if(tmp.db->Put(0, keya, dataa, 0) != 0) return NOTOK;
	reta = cursora->Get(keya, dataa, DB_NEXT);
      } else {
	if(tmp.db->Put(0, keyb, datab, 0) != 0) return NOTOK;
	retb = cursorb->Get(keyb, datab, DB_NEXT);
      }
    }

    //
    // Sanity check
    //
    if((reta != 0 && reta != DB_NOTFOUND) ||
       (retb != 0 && retb != DB_NOTFOUND))
      return NOTOK;

      //
      // Flush the remaining entries from the index that is
      // not yet empty.
      //
    if(reta != DB_NOTFOUND || retb != DB_NOTFOUND) {
      String key = reta == 0 ? keya : keyb;
      String data = reta == 0 ? data : datab;
      WordDBCursor* cursor = reta == 0 ? cursora : cursorb;
      int ret = 0;
      while(ret == 0) {
	if(tmp.db->Put(0, key, data, 0) != 0) return NOTOK;
	ret = cursor->Get(key, data, DB_NEXT);
      }
      if(ret != DB_NOTFOUND)
	return NOTOK;
    }
      
    delete cursora;
    delete cursorb;

    a->words->Close();
    b->words->Close();
    tmp.Close();

    //
    // Remove file a
    //
    if(unlink((char*)a->filename) != 0) {
      const String message = String("WordListMulti::Merge: unlink ") + a->filename;
      perror((const char*)message);
      return NOTOK;
    }
    if(use_compress) {
      if(unlink((char*)(a->filename + String("_weakcmpr"))) != 0) {
	const String message = String("WordListMulti::Merge: unlink ") + a->filename + String("_weakcmpr");
	perror((const char*)message);
	return NOTOK;
      }
    }

    //
    // Remove file b
    //
    if(unlink((char*)b->filename) != 0) {
      const String message = String("WordListMulti::Merge: unlink ") + b->filename;
      perror((const char*)message);
      return NOTOK;
    }
    if(use_compress) {
      if(unlink((char*)(b->filename + String("_weakcmpr"))) != 0) {
	const String message = String("WordListMulti::Merge: unlink ") + b->filename + String("_weakcmpr");
	perror((const char*)message);
	return NOTOK;
      }
    }

    //
    // Rename tmp file into file b
    //
    if(rename((char*)tmpname, (char*)b->filename) != 0) {
      const String message = String("WordListMulti::Merge: rename ") + tmpname + String(" ") + b->filename;
      perror((const char*)message);
      return NOTOK;
    }
    if(use_compress) {
      if(rename((char*)(tmpname + String("_weakcmpr")), (char*)(b->filename + String("_weakcmpr"))) != 0) {
	const String message = String("WordListMulti::Merge: rename ") + tmpname + String("_weakcmpr ") + b->filename + String("_weakcmpr");
	perror((const char*)message);
	return NOTOK;
      }
    }

    //
    // Update b file size. The size need not be accurate number as long
    // as it reflects the relative size of each file.
    //
    b->size += a->size;

    //
    // The 'a' index is no longer in use
    //
    delete a->words;
    
    serial--;
    //
    // update heap
    //
    myqsort((void*)heap, serial, sizeof(WordDBMulti), (myqsort_cmp)merge_cmp_size, (void*)this);
  }

  //
  // Rename the indexes so that they are in increasing order
  // and push them in the list of active indexes.
  //
  myqsort((void*)heap, serial, sizeof(WordDBMulti), (myqsort_cmp)merge_cmp_filename, (void*)this);
  int i;
  for(i = 0; i < serial; i++) {
    WordDBMulti* db = new WordDBMulti();
    *db = heap[i];

    String newname(filename);
    char tmp[32];
    sprintf(tmp, "%08d", i);
    newname << tmp;

    //
    // Rename if not equal
    //
    if(db->filename.compare(newname)) {
      //
      // Rename db index into newname
      //
      if(rename((char*)db->filename, (char*)newname) != 0) {
	const String message = String("WordListMulti::Merge: rename ") + db->filename + String(" ") + newname;
	perror((const char*)message);
	return NOTOK;
      }
      if(use_compress) {
	if(rename((char*)(db->filename + String("_weakcmpr")), (char*)(newname + String("_weakcmpr"))) != 0) {
	  const String message = String("WordListMulti::Merge: rename ") + db->filename + String("_weakcmpr ") + newname + String("_weakcmpr");
	  perror((const char*)message);
	  return NOTOK;
	}
      }

      db->filename = newname;
    }

    dbs->Push(db);
  }

  return OK;
}

// ****************************************************************************
//
int WordListMulti::Override(const WordReference& arg)
{
  WordDBMulti* db = (WordDBMulti*)dbs->Last();

  if(db->words->Size() > put_max) {
    if(db->words->Close() != OK) return NOTOK;
    if(AddIndex() != OK) return NOTOK;
    db = (WordDBMulti*)dbs->Last();
    if(db->words->Open(db->filename, db->mode) != OK) return NOTOK;
  }

  return db->words->Override(arg);
}

// *****************************************************************************
int WordListMulti::Exists(const WordReference& )
{
  return 0;
}

// *****************************************************************************
//
List *WordListMulti::operator [] (const WordReference& )
{
  return 0;
#if 0
  return Collect(wordRef);
#endif
}

// *****************************************************************************
//
List *WordListMulti::Prefix (const WordReference& )
{
  return 0;
#if 0
  WordReference prefix2(prefix);
  prefix2.Key().UndefinedWordSuffix();
  return Collect(prefix2);
#endif
}

// *****************************************************************************
//
List *WordListMulti::WordRefs()
{
  return 0;
#if 0
  return Collect(WordReference(context));
#endif
}

// *****************************************************************************
//
List *WordListMulti::Collect(const WordReference&)
{
  return 0;
#if 0
  WordCursor *search = Cursor(wordRef.Key(), HTDIG_WORDLIST_COLLECTOR);
  if(search->Walk() != OK) return 0;
  List* result = search->GetResults();
  delete search;
  return result;
#endif
}

// *****************************************************************************
// 
// Delete all records matching wordRef, return the number of 
// deleted records.
//
int WordListMulti::WalkDelete(const WordReference& )
{
  return 0;
#if 0
  DeleteWordData data;
  WordCursor *description = Cursor(wordRef.Key(), delete_word, &data);
  description->Walk();
  delete description;
  return data.count;
#endif
}

int WordListMulti::Delete(const WordReference& )
{
  return NOTOK;
}

// *****************************************************************************
//
//
List *WordListMulti::Words()
{
  return 0;
#if 0
  List		*list = 0;
  String		key;
  String		record;
  WordReference	lastWord(context);
  WordDBCursor*		cursor = db.Cursor();

  if(!cursor) return 0;

  //
  // Move past the first word count record
  //
  const WordReference& last = WordStat::Last(context);
  last.Pack(key, record);
  if(cursor->Get(key, record, DB_SET_RANGE) != 0)
    return 0;
  list = new List;
  do {
    WordReference	wordRef(context, key, record);
    if(lastWord.Key().GetWord().empty() ||
       wordRef.Key().GetWord() != lastWord.Key().GetWord()) 
      {
	list->Add(new String(wordRef.Key().GetWord()));
	lastWord = wordRef;
      }
  } while (cursor->Get(key, record, DB_NEXT) == 0);
    
  return list;
#endif
}

// *****************************************************************************
//
// Returns the reference count for word in <count> arg
//
int WordListMulti::Noccurrence(const String& , unsigned int& ) const
{
  return 0;
#if 0
  noccurrence = 0;
  WordStat stat(context, key.GetWord());
  int ret;
  if((ret = db.Get(stat)) != 0) {
    if(ret != DB_NOTFOUND)
      return NOTOK;
  } else {
    noccurrence = stat.Noccurrence();
  }

  return OK;
#endif
}

// *****************************************************************************
//
// Increment reference count for wordRef
//
int WordListMulti::Ref(const WordReference& )
{
  return NOTOK;
}

// *****************************************************************************
//
// Decrement reference count for wordRef
//
int WordListMulti::Unref(const WordReference& )
{
  return NOTOK;
}

// *****************************************************************************
//
int WordListMulti::AllRef() {
  if(!extended) return OK;

  Merge();

  WordDBMulti* db;
  ListCursor cursor;
  for(dbs->Start_Get(cursor); (db = (WordDBMulti*)dbs->Get_Next(cursor));) {
    if(!db->words->isopen) {
      if(db->words->Open(db->filename, O_RDWR) != OK) return NOTOK;
      if(db->words->Close() != OK) return NOTOK;
    }
  }

  return OK;
}
