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
// $Id: WordList.cc,v 1.6 1999/10/05 16:48:03 loic Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "WordList.h"
#include "WordReference.h"
#include "WordRecord.h"
#include "WordType.h"
#include "WordStat.h"
#include "Configuration.h"
#include "htString.h"
#include "HtPack.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <iostream.h>
#include <fstream.h>
#include <errno.h>

//*****************************************************************************
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
    isopen = 0;
    isread = 0;
    extended = config.Boolean("wordlist_extend");
}

//*****************************************************************************
//
int WordList::Open(const String& filename, int mode)
{
  //
  // Info initialization
  //
  db.dbinfo.set_bt_compare(word_db_cmp);

  int ret = db.Open(filename, DB_BTREE, mode == O_RDONLY ? DB_RDONLY : DB_CREATE, 0666);

  isread = mode & O_RDONLY;
  isopen = 1;

  return ret;
}


//*****************************************************************************
//
int WordList::Close()
{
  if(isopen) {
    db.Close();
    isopen = 0;
    isread = 0;
  }
  return OK;
}


//*****************************************************************************
//
int WordList::Put(const WordReference& arg, int flags)
{
  if (arg.Key().GetWord().length() == 0) {
    cerr << "WordList::Put(" << arg << ") word is zero length\n";
    return NOTOK;
  }

  WordReference	wordRef(arg);
  String 	word = wordRef.Key().GetWord();
  if(wtype.Normalize(word) & WORD_NORMALIZE_NOTOK)
    return NOTOK;
  wordRef.Key().SetWord(word);

  
  int ret = db.Put(wordRef, flags);
  if(ret == OK)
    ret = Ref(wordRef);
  return ret;
}


//*****************************************************************************
//
List *WordList::operator [] (const WordReference& wordRef)
{
  return Collect(wordRef, HTDIG_WORDLIST_COLLECT_WORD);
}


//*****************************************************************************
//
List *WordList::Prefix (const WordReference& prefix)
{
  return Collect(prefix, HTDIG_WORDLIST_COLLECT_PREFIX);
}

//*****************************************************************************
//
List *WordList::WordRefs()
{
  return Collect(WordReference(), HTDIG_WORDLIST_COLLECT);
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
    List        		*list = 0;
    int				prefixLength = wordRef.Key().GetWord().length();
    WordKey			prefixKey;
    WordCursor			cursor;
    const WordReference&	last = WordStat::Last();
    
    if(cursor.Open(db.db) == NOTOK) return 0;
    
    String key;
    String data;


    if(action & HTDIG_WORDLIST_COLLECTOR) {
      list = new List;
    }

    //
    // Move the cursor to start walking and do some sanity checks.
    //
    if(action & HTDIG_WORDLIST) {
      //
      // Move past the stat data
      //
      last.KeyPack(key);
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
	//
	// Move past the stat data
	//
	last.KeyPack(key);
      } else {
	prefixKey.Pack(key);
      }
      if(cursor.Get(key, data, DB_SET_RANGE) != 0)
	return list;
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
    } while(cursor.Get(key, data, DB_NEXT) == 0);

    return list;
}

//*****************************************************************************
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
    words->Unref(*word);
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
//
List *WordList::Words()
{
    List		*list = 0;
    String		key;
    String		record;
    WordReference	lastWord;
    WordCursor		cursor;

    if(cursor.Open(db.db) != OK) return 0;

    //
    // Move past the first word count record
    //
    const WordReference& last = WordStat::Last();
    last.Pack(key, record);
    if(cursor.Get(key, record, DB_SET_RANGE) != 0)
      return 0;

    list = new List;
    do {
	WordReference	wordRef(key, record);

	if(!lastWord.Key().GetWord().empty() ||
	   wordRef.Key().GetWord() != lastWord.Key().GetWord()) {
	  list->Add(new String(wordRef.Key().GetWord()));
	  lastWord = wordRef;
	}
    } while (cursor.Get(key, record, DB_NEXT) == 0);
    
    return list;
}

//*****************************************************************************
//
// Returns the reference count for word in <count> arg
//
int WordList::Noccurence(const WordKey& key, unsigned int& noccurence) const
{
  noccurence = 0;
  WordStat stat(key.GetWord());
  int ret;
  if((ret = db.Get(stat)) != 0) {
    if(ret != DB_NOTFOUND)
      return NOTOK;
  } else {
    noccurence = stat.Noccurence();
  }

  return OK;
}

//*****************************************************************************
//
// Increment reference count for wordRef
//
int WordList::Ref(const WordReference& wordRef)
{
  if(!extended) return OK;

  WordStat stat(wordRef.Key().GetWord());
  int ret;
  if((ret = db.Get(stat)) != 0 && ret != DB_NOTFOUND)
    return NOTOK;

  stat.Noccurence()++;

  return db.Put(stat, 0);
}

//*****************************************************************************
//
// Decrement reference count for wordRef
//
int WordList::Unref(const WordReference& wordRef)
{
  if(!extended) return OK;

  WordStat stat(wordRef.Key().GetWord());
  int ret;
  if((ret = db.Get(stat)) != 0) {
    if(ret == DB_NOTFOUND)
      cerr << "WordList::Unref(" << wordRef << ") Unref on non existing word occurence\n";
    return NOTOK;
  }

  if(stat.Noccurence() == 0) {
    cerr << "WordList::Unref(" << wordRef << ") Unref on 0 occurences word\n";
    return NOTOK;
  }
  stat.Noccurence()--;

  if(stat.Noccurence() > 0)
    ret = db.Put(stat, 0);
  else
    ret = db.Del(stat) == 0 ? OK : NOTOK;
  return ret;
}
