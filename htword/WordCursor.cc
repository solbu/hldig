//
// WordCursor.cc
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: WordCursor.cc,v 1.4 2004/05/28 13:15:26 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>

#include "WordCursor.h"
#include "WordStat.h"
#include "WordList.h"

#include <stdio.h>

//
// WordCursor implementation
// 

// *****************************************************************************
//
int WordCursor::Initialize(WordList *nwords, const WordKey &nsearchKey, wordlist_walk_callback_t ncallback, Object *ncallback_data, int naction)
{
    action = naction;
    searchKey = nsearchKey;
    callback = ncallback;
    callback_data = ncallback_data;
    words = nwords;
    return OK;
}

// *****************************************************************************
//
void 
WordCursor::Clear()
{
  searchKey.Clear();
  action = 0;
  callback = 0;
  callback_data = 0;
  ClearResult();
  ClearInternal();
  words = 0;

  //
  // Debugging section. 
  //
  traceRes = 0;
}

// *****************************************************************************
//
void
WordCursor::ClearInternal()
{
  cursor.Close();
  key.trunc();
  data.trunc();
  prefixKey.Clear();
  cursor_get_flags = DB_SET_RANGE;
  searchKeyIsSameAsPrefix = 0;
}

// *****************************************************************************
//
void
WordCursor::ClearResult()
{
  collectRes = 0;
  found.Clear();
  status = OK;
}

int
WordCursor::ContextRestore(const String& buffer)
{
  int ret = OK;
  if(!buffer.empty()) {
    WordKey key(buffer);
    if((ret = Seek(key)) != OK)
      return ret;
    //
    // Move to restored position so that next call to
    // WalkNext will go above the restored position.
    //
    if((ret = WalkNext()) != OK)
      return ret;
  }
  return ret;
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
// all the word occurrences matching the fields set in the key are retrieved. It may
// be fast if key is a prefix (see WordKey::Prefix for a definition). It may
// be *slow* if key is not a prefix because it forces a complete walk of the
// index. 
//
int 
WordCursor::Walk()
{
  int ret;
  if((ret = WalkInit()) != OK) return ret;
  while((ret = WalkNext()) == OK)
    ;
  int ret1;
  if((ret1 = WalkFinish()) != OK) return ret1;

  return ret == WORD_WALK_ATEND ? OK : NOTOK;
}

int 
WordCursor::WalkInit()
{
  int ret = OK;

  ClearResult();
  ClearInternal();

  WordReference wordRef;

  if((ret = cursor.Open(words->db.db)) != 0)
    return ret;

  if(words->verbose) fprintf(stderr, "WordCursor::WalkInit: action = %d, SearchKey = %s\n", action, (char*)searchKey.Get());

  if(action & HTDIG_WORDLIST_COLLECTOR) {
    collectRes = new List;
  }

  const WordReference&  last                    = WordStat::Last();

  WordKey first_key;
  //
  // Move the cursor to start walking and do some sanity checks.
  //
  if(searchKey.Empty()) {
    //
    // Move past the stat data
    //
    if(words->verbose) fprintf(stderr, "WordCursor::WalkInit: at start of keys because search key is empty\n");
    first_key = last.Key();

  } else {
    prefixKey = searchKey;
    //
    // If the key is a prefix, the start key is
    // the longest possible prefix contained in the key. If the
    // key does not contain any prefix, start from the beginning
    // of the file.
    //
    if(prefixKey.PrefixOnly() == NOTOK) {
      if(words->verbose) fprintf(stderr, "WordCursor::WalkInit: at start of keys because search key is not a prefix\n");
      prefixKey.Clear();
      //
      // Move past the stat data
      //
      first_key = last.Key();
    } else {
      if(words->verbose) fprintf(stderr, "WordCursor::WalkInit: go to %s \n", (char*)prefixKey.Get());
      first_key = prefixKey;
    }
  }

  first_key.Pack(key);
  //
  // Allow Seek immediately after Init
  //
  found.Key().CopyFrom(first_key);

  status = OK;
  searchKeyIsSameAsPrefix = searchKey.ExactEqual(prefixKey);
  cursor_get_flags = DB_SET_RANGE;

  return OK;
}

int 
WordCursor::WalkRewind()
{
  const WordReference&  last                    = WordStat::Last();

  WordKey first_key;
  //
  // Move the cursor to start walking and do some sanity checks.
  //
  if(searchKey.Empty()) {
    first_key = last.Key();
  } else {
    prefixKey = searchKey;
    //
    // If the key is a prefix, the start key is
    // the longest possible prefix contained in the key. If the
    // key does not contain any prefix, start from the beginning
    // of the file.
    //
    if(prefixKey.PrefixOnly() == NOTOK) {
      prefixKey.Clear();
      //
      // Move past the stat data
      //
      first_key = last.Key();
    } else {
      first_key = prefixKey;
    }
  }

  first_key.Pack(key);
  //
  // Allow Seek immediately after Rewind
  //
  found.Key().CopyFrom(first_key);

  status = OK;
  searchKeyIsSameAsPrefix = searchKey.ExactEqual(prefixKey);
  cursor_get_flags = DB_SET_RANGE;

  return OK;
}

int 
WordCursor::WalkNext()
{
  int ret;
  while((ret = WalkNextStep()) == WORD_WALK_NOMATCH_FAILED)
    if(words->verbose > 1) fprintf(stderr, "WordCursor::WalkNext: got false match, retry\n");

  return ret;
}

int 
WordCursor::WalkNextStep()
{
  status = OK;

  {
    int error;
    if((error = cursor.Get(key, data, cursor_get_flags)) != 0) {
      if(error == DB_NOTFOUND) {
	if(words->verbose) fprintf(stderr, "WordCursor::WalkNextStep: looking for %s, no more matches\n", (char*)searchKey.Get());
	return (status = WORD_WALK_ATEND);
      } else {
	return WORD_WALK_GET_FAILED;
      }
    }
  }

  //
  // Next step operation is always sequential walk
  //
  cursor_get_flags = DB_NEXT;

  found.Unpack(key, data);

  if(traceRes) traceRes->Add(new WordReference(found));

  if(words->verbose > 1) fprintf(stderr, "WordCursor::WalkNextStep: looking for %s, candidate is %s\n", (char*)searchKey.Get(), (char*)found.Get());

  //
  // Don't bother to compare keys if we want to walk all the entries
  //
  if(!(searchKey.Empty()))     {
    // examples
    // searchKey: aabc 1 ? ? ?
    // prefixKey: aabc 1 ? ? ?

    //
    // Stop loop if we reach a record whose key does not
    // match prefix key requirement, provided we have a valid
    // prefix key.
    // (ie. stop loop if we're past last possible match...)
    //
    if(!prefixKey.Empty() &&
       !prefixKey.Equal(found.Key()))	{
      if(words->verbose) fprintf(stderr, "WordCursor::WalkNextStep: looking for %s, no more matches because found a key that is greater than searchKey\n", (char*)searchKey.Get());
      return (status = WORD_WALK_ATEND);
    }

    //
    // Skip entries that do not exactly match the specified key.
    // 
    if(!searchKeyIsSameAsPrefix && 
       !searchKey.Equal(found.Key()))	{
      int ret;
      switch((ret = SkipUselessSequentialWalking())) {
      case OK:
	if(words->verbose > 1) fprintf(stderr, "WordCursor::WalkNextStep: looking for %s, false match jump to %s\n", (char*)searchKey.Get(), (char*)found.Get());
	return WORD_WALK_NOMATCH_FAILED;
	break;
      case WORD_WALK_ATEND:
	if(words->verbose) fprintf(stderr, "WordCursor::WalkNextStep: looking for %s, no more matches according to SkipUselessSequentialWalking\n", (char*)searchKey.Get());
	return (status = WORD_WALK_ATEND);
	break;
      default:
	fprintf(stderr, "WordCursor::WalkNextStep: SkipUselessSequentialWalking failed %d\n", ret);
	return NOTOK;
	break;
      }
    }
  }

  if(words->verbose) fprintf(stderr, "WordCursor::WalkNextStep: looking for %s, found %s\n", (char*)searchKey.Get(), (char*)found.Get());

  if(collectRes) {
    if(words->verbose > 2) fprintf(stderr, "WordCursor::WalkNextStep: collect\n");
    collectRes->Add(new WordReference(found));
  } else if(callback) {
    if(words->verbose > 2) fprintf(stderr, "WordCursor::WalkNextStep: calling callback\n");
    int ret = (*callback)(words, cursor, &found, *(callback_data) );
    //
    // The callback function tells us that something went wrong, might
    // as well stop walking.
    //
    if(ret != OK) {
      if(words->verbose) fprintf(stderr, "WordCursor::WalkNextStep: callback returned NOTOK");
      return WORD_WALK_CALLBACK_FAILED|(status = WORD_WALK_ATEND);
    }
  }

  return OK;
}

int 
WordCursor::WalkFinish()
{
  if(words->verbose) fprintf(stderr, "WordCursor::WalkFinish\n");

  return cursor.Close() == 0 ? OK : NOTOK;
}

// *****************************************************************************
//
// Helper for SkipUselessSequentialWalking. 
// Undefine in foundKey all fields defined in searchKey
// so that they are not considered by SetToFollowing.
// It could become a method of WordKey but lacks generalisation and
// from what I see it is a rather specific operation.
//
static inline void complement(WordKey& key, const WordKey& mask)
{
  int nfields = WordKey::NFields();
  int i;
  //
  // Undefine in 'key' all fields defined in 'mask'
  //
  for(i = 0; i < nfields; i++) {
    if(mask.IsDefined(i))
      key.Undefined(i);
    else
      key.SetDefined(i);
  }
  //
  // If searching for a prefix, we must allow the word in 
  // key to increment.
  //
  if(mask.IsDefinedWordSuffix()) {
    key.UndefinedWordSuffix();
  } else {
    key.SetDefinedWordSuffix();
    key.SetDefined(0);
  }
}

// *****************************************************************************
//
// Find out if we should better jump to the next possible key (DB_SET_RANGE) instead of 
// sequential iterating (DB_NEXT). 
// If it is decided that jump is a better move :
//   cursor_set_flags = DB_SET_RANGE
//   key = calculated next possible key
// Else
//   do nothing
// Return values
// OK: skipping successfull.
// WORD_WALK_ATEND : no more possible match, reached the maximum
// WORD_WALK_FAILED: general failure, occurs if called and no skipping
//                   necessary.
// 
// Sequential searching can waste time by searching all keys, for example:
// If searching for Key: argh <DEF> <UNDEF> 10
// Under normal circonstances we would do the following
// 
//    DATA            STATUS   ACTION
// 1: argh 1 10       match    DB_NEXT
// 2: argh 2 11       nomatch  DB_NEXT
// 3: argh 2 15       nomatch  DB_NEXT
// 4: argh 2 20       nomatch  DB_NEXT
// 5: argh 2 30       nomatch  DB_NEXT
// 6: argh 5 1        nomatch  DB_NEXT
// 7: argh 5 8        nomatch  DB_NEXT
// 8: argh 8 6        nomatch  DB_NEXT
//
// But the optimal would be
//
//    DATA            STATUS   ACTION
// 1: argh 1 10       match    DB_NEXT
// 2: argh 2 11       nomatch  DB_SET_RANGE argh 3 10
// 3: argh 2 15       
// 4: argh 2 20       
// 5: argh 2 30
// 6: argh 5 1        nomatch  DB_SET_RANGE argh 5 10
// 7: argh 5 8
// 8: argh 8 6        nomatch  DB_SET_RANGE argh 8 10
//
// That saves a lot of unecessary hit. The underlying logic is a bit 
// more complex but you have the idea.
//
int
WordCursor::SkipUselessSequentialWalking()
{
  WordKey&      foundKey         = found.Key();

  int nfields = WordKey::NFields();
  int i;

  //
  // Find out how the searchKey and the foundKey differ.
  //
  int diff_field = 0;
  int lower = 0;
  if(!foundKey.Diff(searchKey, diff_field, lower)) {
    //
    // foundKey matches searchKey (no difference), don't
    // skip, everything is fine. The caller of SkipUselessSequentialWalking
    // is expected to avoid this case for efficiency.
    //
    return WORD_WALK_FAILED;
  }

  if(words->verbose > 2) fprintf(stderr, "WordCursor::SkipUselessSequentialWalking: looking for %s, candidate is %s\n", (char*)searchKey.Get(), (char*)foundKey.Get());

  //
  // Undefine in foundKey all fields defined in searchKey
  // so that they are not considered by SetToFollowing.
  //
  complement(foundKey, searchKey);

  //
  // If the key found is lower than the searched key when
  // considering only the fields defined in the search key,
  // we only need to enforce the key to get the match.
  // Otherwise we need to increment the found key to jump
  // properly.
  //
  if(lower) {
    if(words->verbose > 1) fprintf(stderr, "WordCursor::SkipUselessSequentialWalking: enforcing the search constraint is enough to jump forward\n");
    for(i = diff_field + 1; i < nfields; i++)
      if(foundKey.IsDefined(i)) foundKey.Set(i, 0);
  } else {
    if(words->verbose > 1) fprintf(stderr, "WordCursor::SkipUselessSequentialWalking: increment the key to jump forward\n");
    //
    // diff_field - 1 is not really necessary because diff_field is undefined
    // in foundKey and would therefore be ignored by SetToFollowing. We write
    // diff_field - 1 to clearly state that incrementing begins just before the
    // field for which a difference was found.
    //
    int ret;
    if((ret = foundKey.SetToFollowing(diff_field - 1)) != OK)
      return ret;
  }

  //
  // Copy all fields defined in searchKey into foundKey. This will copy
  // searchKey in foundKey because all these fields have been
  // previously undefined in foundKey.
  //
  foundKey.Merge(searchKey);

  if(words->verbose > 2) fprintf(stderr, "WordCursor::SkipUselessSequentialWalking: looking for %s, jump to %s\n", (char*)searchKey.Get(), (char*)foundKey.Get());

  //
  // Instruct Next function to jump to the calculated key
  //
  if(foundKey.Pack(key) == NOTOK) {
    return WORD_WALK_FAILED;
  }
  cursor_get_flags = DB_SET_RANGE;

  return OK;
}

// *****************************************************************************
//
// Copy defined fields in patch into foundKey and 
// initialize internal state so that WalkNext jumps to
// this key next time it's called.
//
// Technically this means : Override latest key found (found data member)
// with patch fields values, starting from the first field set in
// patch up to the last.  Pack the result in the key field and set
// cursor_get_flags to DB_SET_RANGE.
//
int
WordCursor::Seek(const WordKey& patch)
{
  int nfields = WordKey::NFields();
  WordKey pos = searchKey;

  if(patch.Empty()) {
    fprintf(stderr, "WordCursor::Seek: empty patch is useless\n");
    return NOTOK;
  }
  
  int i;
  //
  // Leave the most significant fields untouched
  //
  for(i = WORD_FIRSTFIELD; i < nfields; i++)
    if(patch.IsDefined(i))
      break;
  //
  // From the first value set in the patch to the end
  // override.
  //
  for(; i < nfields; i++) {
    if(patch.IsDefined(i))
      pos.Set(i, patch.Get(i));
    else
      pos.Set(i, 0);
  }

  if(!pos.Filled()) {
    fprintf(stderr, "WordCursor::Seek: only make sense if the resulting key is fully defined\n");
    return NOTOK;
  }

  if(words->verbose > 2) fprintf(stderr, "WordCursor::Seek: seek to %s\n", (char*)pos.Get());

  //
  // Next move will jump to the patched key
  //
  pos.Pack(key);
  cursor_get_flags = DB_SET_RANGE;
  
  return OK;
}

int WordCursor::Noccurrence(unsigned int& noccurrence) const
{
  if(!words) {
    fprintf(stderr, "WordCursor::Noccurrence: words not set (call Prepare first)\n");
    return NOTOK;
  }
  return words->Noccurrence(searchKey, noccurrence);
}

//
// Convert the whole structure to an ascii string description
//
int WordCursor::Get(String& bufferout) const
{
  String tmp;
  bufferout.trunc();

  searchKey.Get(tmp);
  bufferout << "Input: searchKey = " << tmp << ", action = " <<  action << "; Output: collectRes " << (collectRes ? "set" : "not set");
  found.Get(tmp);
  bufferout << ", found = " << tmp << ", status = " << status;
  prefixKey.Get(tmp);
  bufferout << "; Internal State: prefixKey = " << tmp << ", cursor_get_flags = " << cursor_get_flags;

  return OK;
}
