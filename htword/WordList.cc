//
// WordList.cc
//
// WordList: Interface to the word database. Previously, this wrote to 
//           a temporary text file. Now it writes directly to the 
//           word database. 
//           NOTE: Some code previously attempted to directly read from 
//           the word db. This will no longer work, so it's preferred to 
//           use the access methods here.
//	     Configuration parameter used:
//           wordlist_extend
//           wordlist_verbose
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordList.cc,v 1.6.2.3 1999/10/25 13:11:21 bosc Exp $
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
    CleanupTrace();
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
    verbose =  config.Value("wordlist_verbose",0);
    traceOn=0;
    traceRes=NULL;
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
    WordReference prefix2(prefix);
    prefix2.Key().UndefinedWordSuffix();
    return Collect(prefix2, HTDIG_WORDLIST_COLLECT);
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
//      verbose=2;
    List        		*list = 0;
    WordKey			prefixKey;
    WordCursor			cursor;
    const WordReference&	last = WordStat::Last();
    
    if(cursor.Open(db.db) == NOTOK) return 0;
    
    String key;
    String data;

    if(verbose){cout << "Walk begin:action:" << action << ":SearchKey:"<< wordRef.Key()
		     << ": SuffixDeffined:" << wordRef.Key().IsDefinedWordSuffix() << "\n";}
    int nfields=word_key_info.nfields;

    // find first field that must be checked
    int first_skip_field=-2;
    for(int i=0;i<nfields;i++)
    {
	if(first_skip_field==-2 && !wordRef.Key().IsDefinedInSortOrder(i)){first_skip_field=-1;}
	if(first_skip_field==-1 &&  wordRef.Key().IsDefinedInSortOrder(i)){first_skip_field=i;break;}
    }
    if(first_skip_field<0){first_skip_field=nfields;}

    if(verbose){cout << "check skip speedup first field first_skip_field:" << first_skip_field << endl;}

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
	if(verbose>1){cout << "WORDLIST -> starting from begining" <<endl;}
      last.KeyPack(key);

    } else 
    {
	const WordKey& wordKey = wordRef.Key();

	prefixKey = wordKey;
	//
	// If the key is not a prefix, the start key is
	// the longest possible prefix contained in the key. If the
	// key does not contain any prefix, start from the beginning
	// of the file.
	//
    	if(!prefixKey.PrefixOnly()) 
	{
	    prefixKey.Clear();
	    //
	    // Move past the stat data
	    //
	    last.KeyPack(key);
	}
	else {prefixKey.Pack(key);}
    }
    if(cursor.Get(key, data, DB_SET_RANGE) != 0)
	return list;


    // **** Walk main loop
    int cursor_get_flags= DB_NEXT;

    do 
    {
	WordReference found(key, data);
	cursor_get_flags= DB_NEXT;

	if(traceOn)
	{
	    if(verbose>1)cout << "adding to trace:" << found << endl;
	    traceRes->Add(new WordReference(found));
	}

	if(verbose>1){cout << "*:  found:" <<  found << endl;}
	//
	// Don't bother to compare keys if we want to walk all the entries
	//
	if(!(action & HTDIG_WORDLIST)) 
	{
	    //
	    // Stop loop if we reach a record whose key does not
	    // match prefix key requirement, provided we have a valid
	    // prefix key.
	    //
	    if(!prefixKey.Empty() &&
	       !prefixKey.Equal(found.Key()))
		break;

	    //
	    // Skip entries that do not exactly match the specified key.
	    //
	    if(!wordRef.Key().Equal(found.Key()))
	    {
		SkipUselessSequentialWalking(wordRef.Key(),first_skip_field,found.Key(),key,cursor_get_flags);
		continue;
	    }
	}

	if(action & HTDIG_WORDLIST_COLLECTOR) 
	{
	    if(verbose>1){cout << "collecting:" <<  found << endl;}
	    list->Add(new WordReference(found));
	} else 
	if(action & HTDIG_WORDLIST_WALKER) 
	{
	    int ret = callback(this, cursor, &found, callback_data);
	    //
	    // The callback function tells us that something went wrong, might
	    // as well stop walking.
	    //
	    if(ret == NOTOK) break;
	} else 
	{
	    // Useless to continue since we're not doing anything
	    break;
	}
    } while(cursor.Get(key, data, cursor_get_flags) == 0);

    return list;
}

// SKIP SPEEDUP
// sequential searching can waste time by searching all keys, for example in:
// searching for Key: "argh" (unspecified) 10     ... in database:
// 1: "argh" 2 4
// 2: "argh" 2 11
// 3: "argh" 2 15
// 4: "argh" 2 20
// 5: "argh" 2 30
// 6: "argh" 5 1
// 7: "argh" 5 8
// 8: "argh" 8 6
// when sequential search reaches line 2 it will continue 
// searching lines 3 .. 5 when it could have skiped directly to line 6

int
WordList::SkipUselessSequentialWalking(const WordKey &wordRefKey,int first_skip_field,WordKey &foundKey,String &key,int &cursor_get_flags)
{
    int nfields=word_key_info.nfields;
    if(verbose>1){cout << "skipchk:" <<  foundKey << endl;}
    int i;
    // check if "found" key has a field that is bigger than 
    // the corresponding "wordRef" key field
    for(i=first_skip_field;i<nfields;i++)// (field 0 is not set (...it's the word))
    {
	if(wordRefKey.IsDefinedInSortOrder(i))
	{
	    if( (word_key_info.sort[i].direction == WORD_SORT_ASCENDING
		 && foundKey.GetInSortOrder(i) > wordRefKey.GetInSortOrder(i))   ||
		(word_key_info.sort[i].direction == WORD_SORT_DESCENDING
		 && foundKey.GetInSortOrder(i) < wordRefKey.GetInSortOrder(i))      )

	    { //  field 'i' is bigger in "found" than in "wordRef", we can skip
		if(verbose>1){cout << "found field:" << i 
				   << "is past wordref ... maybe we should skip" << endl;}
				// now find a key that's immediately bigger than "found"
		if(foundKey.SetToFollowingInSortOrder(i) == OK)
		{
		    // ok!, we can setup for skip (instead of next) now
		    foundKey.Pack(key);
		    if(verbose>1){cout << "SKIPING TO: " <<  foundKey << endl;}
		    cursor_get_flags=DB_SET_RANGE;
		    break;
		}
	    }
	}
    }
    return OK;
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


// streaming operators for ascii dumping and reading a list
class StreamOutData : public Object
{
public:
    ostream &o;
    StreamOutData(ostream &no):o(no){;}
};
int
wordlist_walk_callback_stream_out(WordList *words, WordCursor& cursor, const WordReference *word, Object &data)
{
    ((StreamOutData&)data).o << *word <<endl;
    return OK;
}

ostream &
operator << (ostream &o,  WordList &list)
{

    WordReference empty;
    StreamOutData data(o);
    list.Walk(empty,HTDIG_WORDLIST_WALK, wordlist_walk_callback_stream_out, (Object &)data);  
    return o;
}

istream &
operator >> (istream &is,  WordList &list)
{
  WordReference word;
  while(!is.eof())
  {
      if(!is.good())
      {
	  cerr << "WordList input from stream failed" << endl;
	  break;
      }
      is >> word;
      if(is.eof()){break;}
      if(!is.good())
      {
	  cerr << "WordList input from stream failed" << endl;
	  break;
      }
      if(list.verbose>1){cout << "WordList operator >> inserting word:" << word << endl;}
      list.Insert(word);
  }
  return is;
}


