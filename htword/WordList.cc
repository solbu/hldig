//
// WordList.cc
//
// WordList: Interface to the word database. Previously, this wrote to 
//           a temporary text file. Now it writes directly to the 
//           word database. 
//           NOTE: Some code previously attempted to directly read from 
//           the word db. This will no longer work, so it's preferred to 
//           use the access methods here.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordList.cc,v 1.2 1999/10/01 12:53:54 loic Exp $
//

#include "WordList.h"
#include "WordReference.h"
#include "WordRecord.h"
#include "WordType.h"
#include "Configuration.h"
#include "htString.h"
#include "db_cxx.h"
#include "HtPack.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <iostream.h>
#include <fstream.h>
#include <errno.h>

static inline const char* dberror(int errval) {
#define DB_MAX_ERROR	(-DB_TXN_CKP + 1)
  static const char* dbstr[DB_MAX_ERROR] = {
    "",
    "DB_INCOMPLETE",
    "DB_KEYEMPTY",
    "DB_KEYEXISTS",
    "DB_LOCK_DEADLOCK",
    "DB_LOCK_NOTGRANTED",
    "DB_LOCK_NOTHELD",
    "DB_NOTFOUND",
    "DB_RUNRECOVERY",
    "DB_DELETED",
    "DB_NEEDSPLIT",
    "DB_SWAPBYTES",
    "DB_TXN_CKP",
  };
  if(errval < 0 && -errval < DB_MAX_ERROR)
    return dbstr[-errval];
  else
    return strerror(errval);
}

//
// Interface to Dbc that uses String instead of Dbt
// Methods report errors on cerr and return OK/NOTOK status.
//
class WordCursor {
public:
  WordCursor() { cursor = 0; }
  ~WordCursor() {
    Close();
  }

  int Open(Db* db) {
    Close();
    if((errno = db->cursor(0, &cursor, 0)) != 0) {
      cerr << "WordCursor::Open failed " << dberror(errno) << "\n";
      return NOTOK;
    }
    return OK;
  }

  int Close() {
    if(cursor) cursor->close();
    cursor = 0;
    return OK;
  }

  int Get(String& key, String& data, int flags) {
    Dbt rkey;
    Dbt rdata;
    switch(flags & DB_OPFLAGS_MASK) {
    case DB_SET_RANGE:
    case DB_SET:
    case DB_GET_BOTH:
      rkey.set_data((void*)key.get());
      rkey.set_size((u_int32_t)key.length());
      break;
    }
    if((errno = cursor->get(&rkey, &rdata, (u_int32_t)flags)) != 0) {
      if(errno != DB_NOTFOUND) {
	cerr << "WordCursor::Get(" << flags << ") failed " << dberror(errno) << "\n";
      }
      return NOTOK;
    }
    key.set((const char*)rkey.get_data(), (int)rkey.get_size());
    data.set((const char*)rdata.get_data(), (int)rdata.get_size());
    return OK;
  }

  int Put(const String& key, const String& data, int flags) {
    Dbt rkey((void*)key.get(), (size_t)key.length());
    Dbt rdata((void*)data.get(), (size_t)data.length());
    if((errno = cursor->put(&rkey, &rdata, (u_int32_t)flags)) != 0) {
      cerr << "WordCursor::Put(" << key << ", " << data << ", " << flags << ") failed " << dberror(errno) << "\n";
      return NOTOK;
    }
    return OK;
  }

  int Del() {
    if((errno = cursor->del((u_int32_t)0)) != 0) {
      cerr << "WordCursor::Del() failed " << dberror(errno) << "\n";
      return NOTOK;
    }
    return OK;
  }
  
private:
    Dbc* cursor;
};

//*****************************************************************************
// WordList::~WordList()
//
WordList::~WordList()
{
    Close();
}

//*****************************************************************************
//
WordList::WordList(const Configuration& config_arg) :
  wtype(config_arg),
  config(config_arg)
{
    // The database itself hasn't been opened yet
    db = 0;
    isopen = 0;
    isread = 0;
}

//*****************************************************************************
//
int WordList::Open(const String& filename, int mode)
{
  Close();

  const char* progname = "WordList";

  //
  // Environment initialization
  //
  // Output errors to the application's log.
  //
  dbenv.set_error_stream(&cerr);
  dbenv.set_errpfx(progname);
  //
  // Do not trust C++ portability of exception handling. I may be
  // wrong about that but have no proof. 
  //
  dbenv.set_error_model(DbEnv::ErrorReturn);

  if ((errno = dbenv.appinit(0, 0, DB_CREATE)) != 0) {
    cerr << progname << ": DbEnv::appinit: " << dberror(errno) << "\n";
    return NOTOK;
  }
  //
  // Info initialization
  //
  dbinfo.set_bt_compare(word_db_cmp);

  isread = mode & O_RDONLY;
  mode = isread ? DB_RDONLY : DB_CREATE;

  if ((errno = Db::open(filename, DB_BTREE, (u_int32_t)mode, 0666, &dbenv, &dbinfo, &db)) != 0) {
    cerr << progname << ": Db::open: " << dberror(errno) << "\n";
    return NOTOK;
  }

  isopen = 1;

  return OK;
}


//*****************************************************************************
// int WordList::Close()
//
int WordList::Close()
{
  if(isopen) {
    db->close(0);
    db = 0;
    isopen = 0;
    isread = 0;
  }
  return OK;
}


//*****************************************************************************
//
int WordList::Put(const WordReference& arg, int flags)
{
  if (arg.Word().length() == 0)
    return NOTOK;

  WordReference	wordRef(arg);
  String 	word = wordRef.Word();
  if(wtype.Normalize(word) & WORD_NORMALIZE_NOTOK)
    return NOTOK;
  wordRef.Word(word);

  int ret;

  String	key;
  String	record;

  if((ret = wordRef.Pack(key, record)) == OK) {
    Dbt rkey(key.get(), key.length());
    Dbt rrecord(record.get(), record.length());
    if((errno = db->put(0, &rkey, &rrecord, flags)) != 0) {
      cerr << "WordList::Put(" << wordRef << ") failed " << dberror(errno) << "\n";
      return NOTOK;
    }
  }

  return ret;
}


//*****************************************************************************
// List *WordList::operator [] (const WordReference& wordRef)
//
List *WordList::operator [] (const WordReference& wordRef)
{
  return Collect(wordRef, HTDIG_WORDLIST_COLLECT_WORD);
}


//*****************************************************************************
// List *WordList::Prefix (const WordReference& prefix)
//
List *WordList::Prefix (const WordReference& prefix)
{
  return Collect(prefix, HTDIG_WORDLIST_COLLECT_PREFIX);
}

//*****************************************************************************
// List *WordList::WordRefs()
//
List *WordList::WordRefs()
{
  return Collect(WordReference(), HTDIG_WORDLIST_COLLECT);
}

//
// Callback data dedicated to Dump and dump_word communication
//
class DumpWordData : public Object
{
public:
  DumpWordData(FILE* fl_arg) { fl = fl_arg; }

  FILE* fl;
};

//*****************************************************************************
//
// Write the ascii representation of a word occurence. Helper
// of WordList::Dump
//
static int dump_word(WordList *, WordCursor &, const WordReference *word, Object &data)
{
  DumpWordData &info = (DumpWordData &)data;

  word->Dump(info.fl);

  return OK;
}

//*****************************************************************************
// int WordList::Dump(char* filename)
//
// Write an ascii version of the word database in <filename>
//
int WordList::Dump(const String& filename)
{
  FILE		*fl;

  if (!isopen) {
    cerr << "WordList::Dump: database must be opened first\n";
    return NOTOK;
  }

  if((fl = fopen(filename, "w")) == 0) {
    perror(form("WordList::Dump: opening %s for writing", (const char*)filename));
    return NOTOK;
  }

  WordReference::DumpHeader(fl);
  DumpWordData data(fl);
  (void)Walk(WordReference(), HTDIG_WORDLIST_WALK, dump_word, data);
  
  fclose(fl);

  return OK;
}

//*****************************************************************************
//
List *WordList::Collect (const WordReference& wordRef, int action)
{
  Object o;
  return Walk(wordRef, action, (wordlist_walk_callback_t)0, o);
}

//*****************************************************************************
//
// Walk and collect data from the word database.
//
// If action bit HTDIG_WORDLIST is set all the words are retrieved.
// If action bit HTDIG_WORDLIST_WORD is set all the occurences of
//    the <wordRef> argument are retrieved.
// If action bit HTDIG_WORDLIST_PREFIX is set all the occurences of
//    the words starting with <wordRef.Word()> are retrieved.
//
// If action bit HTDIG_WORDLIST_COLLECTOR is set WordReferences are
//    stored in a list and the list is returned.
// If action bit HTDIG_WORDLIST_WALKER is set the <callback> function
//    is called for each WordReference found. No list is built and the
//    function returns a null pointer.
//
//
// The <wordRef> argument may be a fully qualified key, containing precise values for each
// field of the key. It may also contain only some fields of the key. In both cases
// all the word occurences matching the fields set in the key are retrieved. It may
// be fast if <wordRef.Key()> is a prefix (see WordKey::Prefix for a definition). It may
// be *slow* if <wordRef.Key()> is not a prefix because it forces a complete walk of the
// index. 
//
// The idea of a prefix key is not to be confused with the idea of a prefix word. A prefix
// key is a partially specified key, a prefix word is the start of a word. A prefix key may
// contain a prefix word if HTDIG_WORDLIST_PREFIX is set and a fully qualified key may 
// contain a prefix word if HTDIG_WORDLIST_PREFIX is set. These are two unrelated things, 
// although similar in concept.
//
List *WordList::Walk(const WordReference& wordRef, int action, wordlist_walk_callback_t callback, Object &callback_data)
{
    List        	*list = 0;
    int			prefixLength = wordRef.Word().length();
    WordKey		prefixKey;
    WordCursor		cursor;
    
    if(cursor.Open(db) == NOTOK) return 0;
    
    String key;
    String data;

    //
    // Move the cursor to start walking and do some sanity checks.
    //
    if(action & HTDIG_WORDLIST) {
      if(cursor.Get(key, data, DB_FIRST) != OK) return 0;
    } else {
      const WordKey& wordKey = wordRef.Key();
      //
      // If searching for words beginning with a prefix, ignore the
      // rest of the key even if set, otherwise it will fail to place
      // the cursor in position.
      //
      if(action & HTDIG_WORDLIST_PREFIX)
	prefixKey.SetWord(wordKey.GetWord());
      else {
	prefixKey = wordKey;
	//
	// If the key is not a prefix, the start key is
	// the longest possible prefix contained in the key. If the
	// key does not contain any prefix, start from the beginning
	// of the file.
	//
	if(!prefixKey.PrefixOnly())
	  prefixKey.Clear();
      }

      //
      // No heuristics possible, we have to start from the beginning. *sigh*
      //
      if(prefixKey.Empty()) {
	if(cursor.Get(key, data, DB_FIRST) != OK) return 0;
      } else {
	prefixKey.Pack(key);
	if(cursor.Get(key, data, DB_SET_RANGE) != OK) return 0;
      }
    }

    if(action & HTDIG_WORDLIST_COLLECTOR) {
      list = new List;
    }

    do {
      WordReference found(key, data);
      //
      // Don't bother to compare keys if we want to walk all the entries
      //
      if(!(action & HTDIG_WORDLIST)) {
	//
	// Stop loop if we reach a record whose key does not
	// match prefix key requirement, provided we have a valid
	// prefix key.
	//
	if(!prefixKey.Empty() &&
	   !prefixKey.Equal(found.Key(), action & HTDIG_WORDLIST_PREFIX ? prefixLength : 0))
	  break;
	//
	// Skip entries that do not exactly match the specified key.
	//
	if(!wordRef.Key().Equal(found.Key(), action & HTDIG_WORDLIST_PREFIX ? prefixLength : 0))
	  continue;
      }

      if(action & HTDIG_WORDLIST_COLLECTOR) {
	list->Add(new WordReference(found));
      } else if(action & HTDIG_WORDLIST_WALKER) {
	int ret = callback(this, cursor, &found, callback_data);
	//
	// The callback function tells us that something went wrong, might
	// as well stop walking.
	//
	if(ret == NOTOK) break;
      } else {
	// Useless to continue since we're not doing anything
	break;
      }
    } while(cursor.Get(key, data, DB_NEXT) == OK);

    return list;
}

//*****************************************************************************
//
int WordList::Exists(const WordReference& wordRef)
{
  String key;

  wordRef.Key().Pack(key);

  Dbt rkey((void*)key.get(), (u_int32_t)key.length());
  Dbt rdata;

  if((errno = db->get(0, &rkey, &rdata, 0)) != 0) {
    if(errno != DB_NOTFOUND)
      cerr << "WordList::Exists(" << wordRef << ") failed " << dberror(errno) << "\n";
    return NOTOK;
  }
  
  return OK;
}

//
// Callback data dedicated to Dump and dump_word communication
//
class DeleteWordData : public Object
{
public:
  DeleteWordData() { count = 0; }

  int count;
};

//*****************************************************************************
//
//
static int delete_word(WordList *words, WordCursor &cursor, const WordReference *word, Object &data)
{
  if(words->Delete(cursor) == 1) {
    ((DeleteWordData&)data).count++;
    return OK;
  } else {
    cerr << "WordList delete_word: deleting " << *word << " failed \n";
    return NOTOK;
  }
}

//*****************************************************************************
// 
// Delete all records matching wordRef, return the number of 
// deleted records.
//
int WordList::WalkDelete(const WordReference& wordRef)
{
  DeleteWordData data;
  (void)Walk(wordRef, HTDIG_WORDLIST_WALK_WORD, delete_word, data);
  return data.count;
}

//*****************************************************************************
// 
// Delete record matching wordRef exactly.
//
int WordList::Delete(const WordReference& wordRef)
{
  String key;

  wordRef.Key().Pack(key);

  Dbt rkey((void*)key.get(), (u_int32_t)key.length());
  Dbt rdata;

  if((errno = db->del(0, &rkey, 0)) != 0) {
    if(errno != DB_NOTFOUND) {
      cerr << "WordList::Delete(" << wordRef << ") failed " << dberror(errno) << "\n";
      return -1;
    }
    return 0;
  }
  
  return 1;
}

//*****************************************************************************
// 
// Delete record at cursor position
//
int WordList::Delete(WordCursor& cursor)
{
  return cursor.Del() == OK ? 1 : 0;
}

//*****************************************************************************
// List *WordList::Words()
//
//
List *WordList::Words()
{
    List		*list = new List;
    String		key;
    String		record;
    WordReference	lastWord;
    WordCursor		cursor;

    if(cursor.Open(db) != OK) return 0;

    while (cursor.Get(key, record, DB_NEXT) == OK)
      {
	WordReference	wordRef(key, record);

	if ( (lastWord.Word() == String("")) || (wordRef.Word() != lastWord.Word()) )
	  {
            list->Add(new String(wordRef.Word()));
	    lastWord = wordRef;
	  }
      }
    return list;
}
