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
// $Id: WordList.cc,v 1.36 1999/09/29 10:10:07 loic Exp $
//

#include "WordList.h"
#include "WordReference.h"
#include "WordRecord.h"
#include "HtWordType.h"
#include "Configuration.h"
#include "htString.h"
#include "DB2_db.h"
#include "HtPack.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fstream.h>
#include <errno.h>

extern Configuration	config;

//*****************************************************************************
// WordList::~WordList()
//
WordList::~WordList()
{
    delete words;
    Close();
}

//*****************************************************************************
//
WordList::WordList(const Configuration& config_arg) :
  wtype(config_arg),
  config(config_arg)
{
    words = new List;

    // The database itself hasn't been opened yet
    isopen = 0;
    isread = 0;
}

//*****************************************************************************
//
void WordList::Replace(const WordReference& arg)
{
  WordReference	wordRef(arg);
  String 	word = wordRef.Word();
  if(wtype.Normalize(word) & WORD_NORMALIZE_NOTOK)
    return;
  wordRef.Word(word);

  //
  // New word.  Create a new reference for it and cache it in the object.
  //
  words->Add(new WordReference(wordRef));
}

//*****************************************************************************
// void WordList::Flush()
//   Dump the current list of words to the database.  After
//   the words have been dumped, the list will be destroyed to make
//   room for the words of the next document.
//   
void WordList::Flush()
{
  WordReference	*wordRef;

    // Provided for backwards compatibility
  if (!isopen)
    Open(config["word_db"], O_RDWR);

  words->Start_Get();
  while ((wordRef = (WordReference *) words->Get_Next()))
    {
      if (wordRef->Word().length() == 0) {
	cerr << "WordList::Flush: unexpected empty word\n";
	continue;
      }

      Add(*wordRef);
    }	
    
  // Cleanup
  words->Destroy();
}

//*****************************************************************************
// void WordList::MarkGone()
//   The current document has disappeared or been modified. 
//   We do not need to store these words.
//
void WordList::MarkGone()
{
  words->Destroy();
}

//*****************************************************************************
//
int WordList::Open(const String& filename, int mode)
{
  Close();
  
  dbf = 0;

  dbf = Database::getDatabaseInstance(DB_BTREE);

  if(!dbf) return NOTOK;

  dbf->SetCompare(word_db_cmp);

  isread = mode & O_RDONLY;

  int ret;

  if(isread)
    ret = dbf->OpenRead(filename);
  else
    ret = dbf->OpenReadWrite(filename, 0777);
  
  if(ret != OK)
    return NOTOK;

  isopen = 1;

  return OK;
}


//*****************************************************************************
// int WordList::Close()
//
int WordList::Close()
{
  if(isopen) {
    dbf->Close();
    delete dbf;
    dbf = 0;
    isopen = 0;
    isread = 0;
  }
  return OK;
}


//*****************************************************************************
//
int WordList::Add(const WordReference& wordRef)
{
  if (wordRef.Word().length() == 0)
    return NOTOK;

  int ret;

  String	key;
  String	record;

  if((ret = wordRef.Pack(key, record)) == OK) {
    dbf->Put(key, record);
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
// static int dump_word(WordList* words, WordReference* word)
//
// Write the ascii representation of a word occurence. Helper
// of WordList::Dump
//
static int dump_word(WordList *, const WordReference *word, Object &data)
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

    if(action & HTDIG_WORDLIST_COLLECTOR) {
      list = new List;
    }

    if(action & HTDIG_WORDLIST) {
      dbf->Start_Get();
    } else {
      //
      // Find the best place to start walking and do some sanity checks.
      //
      const WordKey& key = wordRef.Key();
      //
      // If searching for words beginning with a prefix, ignore the
      // rest of the key even if set, otherwise it will fail to place
      // the cursor in position.
      //
      if(action & HTDIG_WORDLIST_PREFIX)
	prefixKey.SetWord(key.GetWord());
      else {
	prefixKey = key;
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
	dbf->Start_Get();
      } else {
	String packed;
	prefixKey.Pack(packed);
	dbf->Start_Seq(packed);
      }
    }

    String data;
    String key;
    while (dbf->Get_Next(data, key))
      {
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
	  int ret = callback(this, &found, callback_data);
	  //
	  // The callback function tells us that something went wrong, might
	  // as well stop walking.
	  //
	  if(ret == NOTOK) break;
	} else {
	  // Useless to continue since we're not doing anything
	  break;
	}
      }

    return list;
}

//*****************************************************************************
//
int WordList::Exists(const WordReference& wordRef)
{
    return dbf->Exists(wordRef.KeyPack());
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
static int delete_word(WordList *words, const WordReference *word, Object &data)
{
  if(words->Delete(*word) == 1) {
    ((DeleteWordData&)data).count++;
    return OK;
  } else {
    cerr << "WordList delete_word: deleting " << *word << " failed " << strerror(errno) << "\n";
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
// List *WordList::Words()
//
//
List *WordList::Words()
{
    List		*list = new List;
    String		key;
    String		record;
    String		word;
    WordReference	lastWord;

    dbf->Start_Get();
    while (dbf->Get_Next(record, key))
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
