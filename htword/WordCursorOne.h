//
// WordCursorOne.h
//
// NAME
// 
// search and retrieve entries in a WordListOne object.
//
// SYNOPSIS
// 
// #include <WordList.h>
//
// int callback(WordList *, WordDBCursor& , const WordReference *, Object &)
// {
//    ...
// }
//
// Object* data = ...
//
// WordList *words = ...;
//
// WordCursor *search = words->Cursor(callback, data);
// WordCursor *search = words->Cursor(WordKey("word <UNDEF> <UNDEF>"));
// WordCursor *search = words->Cursor(WordKey("word <UNDEF> <UNDEF>"), callback, data);
// WordCursor *search = words->Cursor(WordKey());
//
// ...
//
// if(search->Walk() == NOTOK) bark;
// List* results = search->GetResults();
//
// search->WalkInit();
// if(search->WalkNext() == OK)
//   dosomething(search->GetFound());
// search->WalkFinish();
// 
// DESCRIPTION
// 
// WordCursorOne is a WordCursor derived class that implements search
// in a WordListOne object. It currently is the only derived class of
// the WordCursor object. Most of its behaviour is described in the
// WordCursor manual page, only the behaviour specific to WordCursorOne
// is documented here.
//
//
// END
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordCursorOne.h,v 1.1.2.1 2000/09/14 03:13:27 ghutchis Exp $
//

#ifndef _WordCursorOne_h_
#define _WordCursorOne_h_

#ifndef SWIG
#include "htString.h"
#include "WordKey.h"
#include "WordDB.h"
#include "WordCursor.h"

class WordList;
class WordDBCursor;
#endif /* SWIG */

class WordCursorOne : public WordCursor
{
 public:
#ifndef SWIG
  //-
  // Private constructor. Creator of the object must then call Initialize()
  // prior to using any other methods.
  //
  WordCursorOne(WordList *words);
  //-
  // Private constructor. See WordList::Cursor method with same prototype for
  // description.
  //
  WordCursorOne(WordList *words, wordlist_walk_callback_t callback, Object * callback_data);
  //-
  // Private constructor. See WordList::Cursor method with same prototype for
  // description.
  //
  WordCursorOne(WordList *words, const WordKey &searchKey, int action = HTDIG_WORDLIST_WALKER);
  //-
  // Private constructor. See WordList::Cursor method with same prototype for
  // description.
  //
  WordCursorOne(WordList *words, const WordKey &searchKey, wordlist_walk_callback_t callback, Object * callback_data);
#endif /* SWIG */
  virtual ~WordCursorOne() {
    if(cursor) delete cursor;
  }
  virtual void Clear();
  virtual void ClearInternal();
  virtual void ClearResult();

  virtual inline int ContextSave(String& buffer) const { found.Get(buffer); return OK; }
  virtual int ContextRestore(const String& buffer);

#ifndef SWIG
  virtual int Walk();
#endif /* SWIG */
  virtual int WalkInit();
  virtual int WalkRewind();
  virtual int WalkNext();
#ifndef SWIG
  virtual int WalkNextStep();
#endif /* SWIG */
  virtual int WalkFinish();
  //
  // Find out if cursor should better jump to the next possible key
  // (DB_SET_RANGE) instead of sequential iterating (DB_NEXT).  If it
  // is decided that jump is a better move : cursor_set_flags =
  // DB_SET_RANGE key = calculated next possible key Else do nothing
  // Return OK if skipping successfull.  Returns WORD_WALK_ATEND if no
  // more possible match, reached the maximum. Returns
  // WORD_WALK_FAILED on general failure, occurs if called and no
  // skipping necessary.
  // 
  int SkipUselessSequentialWalking();

  virtual int Seek(const WordKey& patch);

#ifndef SWIG
  virtual int Get(String& bufferout) const;
  inline String Get() const { String tmp; Get(tmp); return tmp; }

 protected:

  int Initialize(WordList *nwords, const WordKey &nsearchKey, wordlist_walk_callback_t ncallback, Object * ncallback_data, int naction);

  //
  // Internal state
  //
  //
  // The actual Berkeley DB cursor.
  //
  WordDBCursor* cursor;
  //
  // The latest retrieved key and data
  //
  String key;
  String data;
  //
  // The shorted prefix key computed from searchKey
  //
  WordKey prefixKey;
  //
  // WalkNext leap is either DB_NEXT or DB_SET_RANGE.
  //
  int cursor_get_flags;
  //
  // True if search key is a prefix key
  //
  int searchKeyIsSameAsPrefix;
#endif /* SWIG */
};

#endif /* _WordCursorOne_h_ */
