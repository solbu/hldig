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
// $Id: WordList.cc,v 1.8 2000/02/19 05:29:08 ghutchis Exp $
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
#include "HtTime.h"
#include "WordMonitor.h"
#include "WordDBCompress.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <iostream.h>
#include <fstream.h>
#include <errno.h>

// *****************************************************************************
//
WordList::~WordList()
{
    Close();
}

// *****************************************************************************
//
WordList::WordList(const Configuration& config_arg) :
  wtype(config_arg),
  config(config_arg)
{
    // The database itself hasn't been opened yet
    isopen = 0;
    isread = 0;
    extended = config.Boolean("wordlist_extend");
    compressor = 0;
    verbose =  config.Value("wordlist_verbose",0);

    bm_put_count = 0;
    bm_put_time = 0;
    bm_walk_count = 0;
    bm_walk_time=0;
    bm_walk_count_DB_SET_RANGE = 0;
    bm_walk_count_DB_NEXT = 0;
    monitor = 0;
}

// *****************************************************************************
//
int WordList::Open(const String& filename, int mode)
{
  int usecompress=0;

  db.dbinfo.set_bt_compare(word_db_cmp);

  if(config.Value("wordlist_page_size" ,0 )) {db.dbinfo.set_pagesize (config.Value("wordlist_page_size" ,0));}

  int cache_size=config.Value("wordlist_cache_size",10000000);
  if(cache_size){db.dbinfo.set_cachesize(cache_size);}
  else
  {
      cout << "WordList::Open WARNING no cachesize:: performance might be slow"  << endl;
  }

  if(config.Boolean("wordlist_compress") == 1)
  {
      usecompress=DB_COMPRESS;
      WordDBCompress* compressor = new WordDBCompress();
      compressor->debug = config.Value("wordlist_compress_debug");
      SetCompressor(compressor);
      db.CmprInfo(compressor->CmprInfo());
  }

  db.dbinfo.set_bt_compare(word_db_cmp);

  if(config.Boolean("wordlist_monitor") == 1) {
    monitor = new WordMonitor(config, GetCompressor(), this);
    if(GetCompressor()) { GetCompressor()->SetMonitor(monitor); }
  }

  int ret = db.Open(filename, DB_BTREE, (mode == O_RDONLY ? DB_RDONLY : DB_CREATE) | usecompress, 0666);

  isread = mode & O_RDONLY;
  isopen = 1;

  return ret;
}

// *****************************************************************************
//
int WordList::Close()
{
  if(isopen) {
    db.Close();
    isopen = 0;
    isread = 0;
  }

  if(monitor) {
    monitor = 0;
    if(GetCompressor()) { GetCompressor()->SetMonitor(0); }
    delete monitor;
  }

  {
    WordDBCompress* compressor = GetCompressor();
    if(compressor) {
      delete compressor;
      SetCompressor(0);
    }
  }

  return OK;
}

// *****************************************************************************
//
int WordList::Put(const WordReference& arg, int flags)
{
  if (arg.Key().GetWord().length() == 0) {
    cerr << "WordList::Put(" << arg << ") word is zero length\n";
    return NOTOK;
  }
  if (!arg.Key().Filled()) {
    cerr << "WordList::Put(" << arg << ") key is not fully defined\n";
  }

  WordReference	wordRef(arg);
  String 	word = wordRef.Key().GetWord();
  if(wtype.Normalize(word) & WORD_NORMALIZE_NOTOK)
    return NOTOK;
  wordRef.Key().SetWord(word);

  // DEBUGING / BENCHMARKING
  double start_time = 0.0;
  if(monitor) start_time = HtTime::DTime();

  int ret = db.Put(wordRef, flags);
  if(ret == OK)
    ret = Ref(wordRef);

  // DEBUGING / BENCHMARKING
  if(monitor) {
    bm_put_count++;
    bm_put_time += HtTime::DTime(start_time);
    (*monitor)();
  }

  return ret;
}


// *****************************************************************************
//
List *WordList::operator [] (const WordReference& wordRef)
{
  return Collect(wordRef);
}

// *****************************************************************************
//
List *WordList::Prefix (const WordReference& prefix)
{
    WordReference prefix2(prefix);
    prefix2.Key().UndefinedWordSuffix();
    return Collect(prefix2);
}

// *****************************************************************************
//
List *WordList::WordRefs()
{
  return Collect(WordReference());
}

// *****************************************************************************
//
List *
WordList::Collect(const WordSearchDescription &csearch)
{
    WordSearchDescription search = csearch;
    Walk(search);
    return search.collectRes;
}

// *****************************************************************************
//
List *WordList::Collect(const WordReference& wordRef)
{
  WordSearchDescription search(wordRef.Key(), HTDIG_WORDLIST_COLLECTOR);
  Walk(search);
  return search.collectRes;
}

// *****************************************************************************
//
// Walk and collect data from the word database.
//
// If action bit HTDIG_WORDLIST_COLLECTOR is set WordReferences are
//    stored in a list and the list is returned.
// If action bit HTDIG_WORDLIST_WALKER is set the <callback> function
//    is called for each WordReference found. No list is built and the
//    function returns a null pointer.
//
// The <searchKey> argument may be a fully qualified key, containing precise values for each
// field of the key. It may also contain only some fields of the key. In both cases
// all the word occurences matching the fields set in the key are retrieved. It may
// be fast if key is a prefix (see WordKey::Prefix for a definition). It may
// be *slow* if key is not a prefix because it forces a complete walk of the
// index. 
//
int 
WordList::Walk(WordSearchDescription &search)
{
    // DEBUGING / BENCHMARKING
    double start_time = 0.0;
    if(monitor) start_time = HtTime::DTime();

    for(WalkInit(search); WalkNext(search) == OK; )
      ;
    WalkFinish(search);

    if(verbose > 2) { cerr << "WordList::Walk: FINSISHED "  << endl; }

    // DEBUGING / BENCHMARKING
    if(monitor) {
      bm_walk_time += HtTime::DTime(start_time);
      (*monitor)();
    }

    return search.status == WORD_WALK_ATEND ? OK : NOTOK;
}

int 
WordList::WalkInit(WordSearchDescription &search)
{
    WordDBCursor&		cursor                  = search.cursor;
    const WordKey&              searchKey               = search.searchKey;

    search.ClearResult();
    search.ClearInternal();

    WordReference wordRef;

    if(cursor.Open(db.db) == NOTOK) {
      search.status = WORD_WALK_CURSOR_FAILED;
      return NOTOK;
    }

    if(verbose > 2) {
      cerr << "WordList::WalkInit: Walk begin:action:" << search.action << ":SearchKey:"<< searchKey
	   << ": SuffixDeffined:" << searchKey.IsDefinedWordSuffix() << "\n";
    }

    search.first_skip_field=searchKey.FirstSkipField();
    if(verbose > 2) {
      cerr << "WordList::WalkInit: check skip speedup first field first_skip_field:" << search.first_skip_field << endl;
    }

    if(search.action & HTDIG_WORDLIST_COLLECTOR) {
	search.collectRes = new List;
    }

    return WalkRewind(search);
}

int 
WordList::WalkRewind(WordSearchDescription &search)
{
    const WordReference&	last                    = WordStat::Last();

    const WordKey&              searchKey               = search.searchKey;
    WordKey&			prefixKey               = search.prefixKey;
    String&                     key                     = search.key;
    int&			cursor_get_flags        = search.cursor_get_flags;
    int&                        searchKeyIsSameAsPrefix = search.searchKeyIsSameAsPrefix;

    //
    // Move the cursor to start walking and do some sanity checks.
    //
    if(searchKey.Empty()) 
    {
      //
      // Move past the stat data
      //
	if(verbose > 2) { cerr << "WordList::WalkInit: WORDLIST -> starting from begining" <<endl; }
	last.KeyPack(key);

    } else 
    {
	prefixKey = searchKey;
	//
	// If the key is a prefix, the start key is
	// the longest possible prefix contained in the key. If the
	// key does not contain any prefix, start from the beginning
	// of the file.
	//
    	if(prefixKey.PrefixOnly() == NOTOK) 
	{
	    if(verbose > 2) { cerr << "WordList::WalkInit: couldnt get prefix -> starting from begining" <<endl; }
	    prefixKey.Clear();
	    //
	    // Move past the stat data
	    //
	    last.KeyPack(key);
	}
	else 
	{
	    if(verbose > 2) { cerr << "WordList::WalkInit: actualy using prefix KEY!: " << prefixKey<< endl; }
	    prefixKey.Pack(key);
	}
    }

    search.status = WORD_WALK_OK;
    searchKeyIsSameAsPrefix = searchKey.ExactEqual(prefixKey);
    cursor_get_flags = DB_SET_RANGE;

    return OK;
}

int 
WordList::WalkNext(WordSearchDescription &search)
{
  if(search.status & WORD_WALK_FAILED) return NOTOK;

  int ret;
  while((ret = WalkNextStep(search)) == NOTOK &&
	(search.status & WORD_WALK_NOMATCH_FAILED))
    search.status = WORD_WALK_OK;
  return ret;
}

int 
WordList::WalkNextStep(WordSearchDescription &search)
{
  if(search.status & WORD_WALK_FAILED) return NOTOK;

  WordDBCursor&	     cursor                  = search.cursor;
  const WordKey&     searchKey               = search.searchKey;
  WordKey&	     prefixKey               = search.prefixKey;
  String&            key                     = search.key;
  String&            data                    = search.data;
  int&		     cursor_get_flags        = search.cursor_get_flags;
  int&               searchKeyIsSameAsPrefix = search.searchKeyIsSameAsPrefix;
  WordReference&     found                   = search.found;

  {
    int error;
    if((error = cursor.Get(key, data, cursor_get_flags)) != 0) {
      cursor.Close();
      search.status = error == DB_NOTFOUND ? WORD_WALK_ATEND : WORD_WALK_GET_FAILED;
      return NOTOK;
    }
  }

  //
  // Next step operation is always sequential walk
  //
  cursor_get_flags = DB_NEXT;

  found.Unpack(key, data);

  if(search.traceRes)
    {
      if(verbose > 2) cerr << "WordList::WalkNextStep: adding to trace:" << found << endl;
      search.traceRes->Add(new WordReference(found));
    }

  if(verbose > 2) { cerr << "WordList::WalkNextStep: *:  found:" <<  found << endl; }
  //
  // Don't bother to compare keys if we want to walk all the entries
  //
  if(!(searchKey.Empty())) 
    {
      // examples
      // searchKey:   aabc 1 ? ? ?
      // prefixKey: aabc 1 ? ? ?

      //
      // Stop loop if we reach a record whose key does not
      // match prefix key requirement, provided we have a valid
      // prefix key.
      // (ie. stop loop if we're past last possible match...)
      //
      if(!prefixKey.Empty() &&
	 !prefixKey.Equal(found.Key()))
	{
	  if(verbose) { cerr << "WordList::WalkNextStep: finished loop: no more possible matches:" << found  << endl; }
	  search.status = WORD_WALK_ATEND;
	  return NOTOK;
	}

      //
      // Skip entries that do not exactly match the specified key.
      // 
      if(!searchKeyIsSameAsPrefix && 
	 !searchKey.Equal(found.Key()))
	{
	  if(!search.noskip)
	    SkipUselessSequentialWalking(search);
	  search.status = WORD_WALK_NOMATCH_FAILED;
	  return NOTOK;
	}
    }

  if(search.collectRes) 
    {
      if(verbose > 2) { cerr << "WordList::WalkNextStep: collecting:" <<  found << endl; }
      search.collectRes->Add(new WordReference(found));
    }
  else if(search.callback)
    {
      if(verbose > 2) { cerr << "WordList::WalkNextStep: calling callback:" <<  found << endl; }
      int ret = (*search.callback)(this, cursor, &found, *(search.callback_data) );
      if(verbose > 2) { cerr << "WordList::WalkNextStep:  callback returned:" <<  ret << endl; }
      //
      // The callback function tells us that something went wrong, might
      // as well stop walking.
      //
      if(ret == NOTOK)
	{
	  if(verbose) { cerr << "WordList::WalkNextStep: finished loop: callback returned NOTOK:" << endl; }
	  search.status = WORD_WALK_CALLBACK_FAILED|WORD_WALK_ATEND;
	  return NOTOK;
	}
    }

  return OK;
}

int 
WordList::WalkFinish(WordSearchDescription &search)
{
  WordDBCursor&		cursor                  = search.cursor;

  return cursor.Close();
}

// *****************************************************************************
//
// Find out if we should better jump to the next possible key (DB_SET_RANGE) instead of 
// sequential iterating (DB_NEXT). 
// If it is decided that jump is a better move :
//   search.cursor_set_flags = DB_SET_RANGE
//   search.key = calculated next possible key
// Else
//   do nothing
// Returns NOTOK if not skipping, OK if skipping.
// 
// Sequential searching can waste time by searching all keys, for example in:
// searching for Key: argh <DEF> <UNDEF> 10     ... in database:
// 1: argh 2 4
// 2: argh 2 11
// 3: argh 2 15
// 4: argh 2 20
// 5: argh 2 30
// 6: argh 5 1
// 7: argh 5 8
// 8: argh 8 6
// when sequential search reaches line 2 it will continue 
// searching lines 3 .. 5 when it could have skiped directly to line 6
//
int
WordList::SkipUselessSequentialWalking(WordSearchDescription &search)
{
  WordKey&      foundKey         = search.found.Key();
  String&       key              = search.key;
  int&          cursor_get_flags = search.cursor_get_flags;

  int nfields=WordKey::NFields();
  int status = NOTOK;
  if(verbose > 1) { cerr << "WordList::SkipUselessSequentialWalking: skipchk:" <<  foundKey << endl; }
  int i;
  //
  // check if "found" key has a field that is bigger than 
  // the corresponding "wordRef" key field
  //
  for(i = search.first_skip_field; i < nfields; i++)// (field 0 is not set (...it's the word))
    {
      if(search.searchKey.IsDefined(i))
	{
	  if( (WordKey::Info()->sort[i].direction == WORD_SORT_ASCENDING
	       && foundKey.Get(i) > search.searchKey.Get(i))   ||
	      (WordKey::Info()->sort[i].direction == WORD_SORT_DESCENDING
	       && foundKey.Get(i) < search.searchKey.Get(i))      )

	    { //  field 'i' is bigger in "found" than in "wordRef", we can skip
	      if(verbose > 1) {
		cerr << "WordList::SkipUselessSequentialWalking: found field:" << i 
		     << "is past wordref ... maybe we should skip" << endl;
	      }
				// now find a key that's immediately bigger than "found"
	      if(foundKey.SetToFollowing(i) == OK)
		{
		  // ok!, we can setup for skip (instead of next) now
		  foundKey.Pack(key);
		  if(verbose > 1) { cerr << "WordList::SkipUselessSequentialWalking: SKIPING TO: " <<  foundKey << endl; }
		  cursor_get_flags=DB_SET_RANGE;
		  status = OK;
		  break;
		}
	    }
	}
    }
  return status;
}

// *****************************************************************************
//
// Callback data dedicated to Dump and dump_word communication
//
class DeleteWordData : public Object
{
public:
  DeleteWordData() { count = 0; }

  int count;
};

// *****************************************************************************
//
//
static int delete_word(WordList *words, WordDBCursor &cursor, const WordReference *word, Object &data)
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

// *****************************************************************************
// 
// Delete all records matching wordRef, return the number of 
// deleted records.
//
int WordList::WalkDelete(const WordReference& wordRef)
{
  DeleteWordData data;
  WordSearchDescription description(wordRef.Key(), delete_word, &data);
  Walk(description);
  return data.count;
}

// *****************************************************************************
//
//
List *WordList::Words()
{
    List		*list = 0;
    String		key;
    String		record;
    WordReference	lastWord;
    WordDBCursor		cursor;

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
	if(lastWord.Key().GetWord().empty() ||
	   wordRef.Key().GetWord() != lastWord.Key().GetWord()) 
	{
	    list->Add(new String(wordRef.Key().GetWord()));
	    lastWord = wordRef;
	}
    } while (cursor.Get(key, record, DB_NEXT) == 0);
    
    return list;
}

// *****************************************************************************
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

// *****************************************************************************
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

// *****************************************************************************
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

  if(stat.Noccurence() > 0) {
    ret = db.Put(stat, 0);
  } else
    ret = db.Del(stat) == 0 ? OK : NOTOK;
  return ret;
}


// *****************************************************************************
//
// streaming operators for ascii dumping and reading a list
class StreamOutData : public Object
{
public:
    ostream &o;
    StreamOutData(ostream &no):o(no){;}
};

// *****************************************************************************
//
int
wordlist_walk_callback_stream_out(WordList *, WordDBCursor& , const WordReference *word, Object &data)
{
    ((StreamOutData&)data).o << *word <<endl;
    return OK;
}

// *****************************************************************************
//
ostream &
operator << (ostream &o,  WordList &list)
{

    WordKey empty;
    StreamOutData data(o);
    WordSearchDescription description(empty,wordlist_walk_callback_stream_out, (Object *)&data);
    list.Walk(description);
    return o;
}

// *****************************************************************************
//
istream &
operator >> (istream &in,  WordList &list)
{
  WordReference word;
#define WORD_BUFFER_SIZE	1024
  char buffer[WORD_BUFFER_SIZE];
  String line;
  int line_number = 0;

  while(!in.eof())
    {
      line_number++;

      in.get(buffer, WORD_BUFFER_SIZE);
      line.append(buffer);
      //
      // Get the terminator. I love iostream :-(
      //
      if(!in.eof()) {
	char c;
	in.get(c);
	if(c == '\n') 
	  line.append(c);
	in.putback(c);
      }
      //
      // Join big lines
      //
      if(line.last() != '\n' && line.last() != '\r' && !in.eof())
	continue;
      //
      // Eat the terminator
      //
      if(!in.eof()) in.get();
      //
      // Strip line terminators from line
      //
      line.chop("\r\n");
      //
      // If line ends with a \ continue
      //
      if(line.last() == '\\') {
	line.chop(1);
	if(!in.eof())
	  continue;
      }
      
      if(!line.empty()) {
	if(!in.good())
	  {
	    cerr << "WordList::operator >>: line " << line_number << " : " << line << endl
		 << " input from stream failed (A)" << endl;
	    break;
	  }

	if(word.Set(line) != OK)
	  cerr << "WordList::operator >>: line " << line_number << " : " << line << endl
	       << " failed (ignored)" << endl;
	else
	  list.Insert(word);
      
	if(in.eof()){break;}
	if(!in.good())
	  {
	    cerr << "WordList::operator >>: line " << line_number << " : " << line << endl
		 << " input from stream failed (B)" << endl;
	    break;
	  }
	if(list.verbose > 1) { cerr << "WordList operator >> inserting word:" << word << endl; }
      }

      line.trunc();
    }
  return in;
}

// *****************************************************************************
//
// WordSearchDescription implementation
// 

// *****************************************************************************
//
WordSearchDescription::WordSearchDescription(const WordKey &nsearchKey, int naction /* = HTDIG_WORDLIST_WALKER */)
{
    Clear();
    searchKey = nsearchKey;
    action = naction;
}

// *****************************************************************************
//
WordSearchDescription::WordSearchDescription(const WordKey &nsearchKey,wordlist_walk_callback_t ncallback,Object * ncallback_data)
{
    Clear();
    searchKey = nsearchKey;
    action = HTDIG_WORDLIST_WALKER;
    callback = ncallback;
    callback_data = ncallback_data;
}

// *****************************************************************************
//
WordSearchDescription::WordSearchDescription(wordlist_walk_callback_t ncallback,Object * ncallback_data)
{
    Clear();
    action = HTDIG_WORDLIST_WALKER;
    callback = ncallback;
    callback_data = ncallback_data;
}

// *****************************************************************************
//
void 
WordSearchDescription::Clear()
{
  searchKey.Clear();
  action = 0;
  callback = 0;
  callback_data = 0;
  ClearResult();
  ClearInternal();

  //
  // Debugging section. 
  //
  noskip = 0;
  traceRes = 0;
}

// *****************************************************************************
//
void
WordSearchDescription::ClearInternal()
{
  cursor.Close();
  key.trunc();
  data.trunc();
  prefixKey.Clear();
  first_skip_field = -3;
  cursor_get_flags = DB_SET_RANGE;
  searchKeyIsSameAsPrefix = 0;
}

// *****************************************************************************
//
void
WordSearchDescription::ClearResult()
{
  collectRes = 0;
  found.Clear();
  status = WORD_WALK_OK;
}

// *****************************************************************************
//
// Intuitive meaning is : set the document number in key and prepare to
// move to this document, if any.
//
// Technical meaning is : Override latests key found (found field) with patch fields
// values, starting from the first field set in patch up to the
// last. 
// Pack the result in the key field and set
// cursor_get_flags to DB_SET_RANGE.
//
int
WordSearchDescription::ModifyKey(const WordKey& patch)
{
  int nfields = WordKey::NFields();
  WordKey& foundKey = found.Key();

  if(!foundKey.Filled()) {
    cerr << "WordSearchDescription::ModifyKey: only make sense on a fully defined key" << endl;
    return NOTOK;
  }

  if(patch.Empty()) {
    cerr << "WordSearchDescription::ModifyKey: empty patch is useless" << endl;
    return NOTOK;
  }
  
  int i;
  //
  // Leave the most significant fields untouched
  //
  for(i = 0; i < nfields; i++)
    if(patch.IsDefined(i))
      break;
  //
  // From the first value set in the patch to the end
  // override.
  //
  for(; i < nfields; i++) {
    if(patch.IsDefined(i))
      foundKey.Set(i, patch.Get(i));
    else
      foundKey.Set(i, 0);
  }

  //
  // Next move will jump to the patched key
  //
  foundKey.Pack(key);
  cursor_get_flags = DB_SET_RANGE;
  
  return OK;
}
