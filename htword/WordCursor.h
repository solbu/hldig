//
// WordList.h
//
// NAME
// 
// search specification and results for WordList.
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
// WordCursor *search = words->Cursor(WordKey("word <DEF> <UNDEF> <UNDEF>"));
// WordCursor *search = words->Cursor(WordKey("word <DEF> <UNDEF> <UNDEF>"), callback, data);
//
// ...
//
// if(search->Walk() == NOTOK) bark;
// List* results = search->GetResults();
//
// if(search->WalkNext() == OK)
//   dosomething(search->GetFound());
// 
// DESCRIPTION
// 
// WordCursor is an iterator on an inverted index. It is created by
// asking a <i>WordList</i> object with the <i>Cursor.</i> There is
// no other way to create a WordCursor object.
// When the <i>Walk*</i> methods return,
// the WordCursor object contains the result of the search and 
// status information that indicates if it reached the end of 
// the list (IsAtEnd() method).
//
// The <b>callback</b> function that is called each time a match is
// found takes the following arguments:
// <pre>
// WordList* words pointer to the inverted index handle.
// WordDBCursor& cursor to call Del() and delete the current match
// WordReference* wordRef is the match
// Object& data is the user data provided by the caller when
//              search began.
// </pre>
//
// The <i>WordKey</i> object that specifies the search criterion
// may be used as follows (assuming word is followed by DOCID and
// LOCATION):
// 
// Ex1: <b>WordKey("word <DEF> <UNDEF> <UNDEF>")</b> find all occurrences
// of <i>word</i>.
//
// Ex2: <b>WordKey("meet <UNDEF> <UNDEF> <UNDEF>")</b> find all occurrences
// starting with <i>meet</i>, including <i>meeting</i> etc.
//
// Ex3: <b>WordKey("meet <DEF> <UNDEF> 1")</b> find all occurrences of
// <i>meet</i> that occur at LOCATION 1 in any DOCID. This can
// be inefficient since the search has to scan all occurrences
// of <i>meet</i> to find the ones that occur at LOCATION 1.
//
// Ex4: <b>WordKey("meet <DEF> 2 <UNDEF>")</b> find all occurrences of
// <i>meet</i> that occur in DOCID 2, at any location.
//
// Interface functions are virtual so that a derivation of the 
// class is possible. Some functions are meant to be used by derived
// classes such as the <b>Initialize</b> function. All data members
// should be accessed using the corresponding accessor if possible.
//
// END
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordCursor.h,v 1.1.2.3 2000/10/10 03:15:43 ghutchis Exp $
//

#ifndef _WordCursor_h_
#define _WordCursor_h_

#ifndef SWIG
#include "htString.h"
#include "WordKey.h"
#include "WordDB.h"

class WordList;
class WordDBCursor;
#endif /* SWIG */
//
// Possible values of the action argument of WordList::Walk
// check walk function in WordList.cc for info on these:
//
#define HTDIG_WORDLIST_COLLECTOR	0x0001
#define HTDIG_WORDLIST_WALKER		0x0002

#ifndef SWIG
//
// Type of the callback argument in WordCursor
//
typedef int (*wordlist_walk_callback_t)(WordList *, WordDBCursor& , const WordReference *, Object &);
#endif /* SWIG */

//
// Possible values of the status member
//
//
// WalkNext reached the end of the matches
//
#define WORD_WALK_ATEND			0x0001
//
// Failed to acquire Berkeley DB cursor
//
#define WORD_WALK_CURSOR_FAILED		0x0002
//
// Berkeley DB Get operation failed
//
#define WORD_WALK_GET_FAILED		0x0004
//
// Callback function returned NOTOK
//
#define WORD_WALK_CALLBACK_FAILED	0x0008
//
// WalkNextStep hit an entry that does not match the
// searched key.
//
#define WORD_WALK_NOMATCH_FAILED	0x0010
//
// WordCursor contains undefined data
//
#define WORD_WALK_FAILED		0xffffffff

//
// Possible return values of the IsA() method
//
#define WORD_CURSOR			1
#define WORD_CURSORS			2

//
// Wordlist::Walk uses WordCursor for :
// state information : cursor
// search term description
// debug/trace/benchmarking
// search result format description
//
class WordCursor
{
 public:
#ifndef SWIG
  //
  // Private constructor. Creator of the object must then call Initialize()
  // prior to using any other methods.
  //
  WordCursor() { Clear(); }
  //-
  // Private constructor. See WordList::Cursor method with same prototype for
  // description.
  //
  WordCursor(WordList *words, wordlist_walk_callback_t callback, Object * callback_data) { Clear(); Initialize(words, WordKey(), callback, callback_data, HTDIG_WORDLIST_WALKER); }
  //-
  // Private constructor. See WordList::Cursor method with same prototype for
  // description.
  //
  WordCursor(WordList *words, const WordKey &searchKey, int action = HTDIG_WORDLIST_WALKER) { Clear(); Initialize(words, searchKey, 0, 0, action); }
  //-
  // Private constructor. See WordList::Cursor method with same prototype for
  // description.
  //
  WordCursor(WordList *words, const WordKey &searchKey, wordlist_walk_callback_t callback, Object * callback_data) { Clear(); Initialize(words, searchKey, callback, callback_data, HTDIG_WORDLIST_WALKER); }
#endif /* SWIG */
  virtual ~WordCursor() {}
  //-
  // Clear all data in object, set <b>GetResult()</b> data to NULL but
  // do not delete it (the application is responsible for that).
  //
  virtual void Clear();
  virtual void ClearInternal();
  virtual void ClearResult();

  //-
  // Returns the type of the object. May be overloaded by
  // derived classes to differentiate them at runtime.
  // Returns WORD_CURSOR.
  //
  virtual int IsA() const { return WORD_CURSOR; }

  //-
  // Returns true if WalkNext() step entries in strictly increasing 
  // order, false if it step entries in random order.
  //
  virtual int Ordered() const { return 1; }

  //-
  // Optimize the cursor before starting a Walk.
  // Returns OK on success, NOTOK otherwise.
  //
  virtual int Optimize() { return OK; }

  //-
  // Save in <b>buffer</b> all the information necessary to resume
  // the walk at the point it left. The ASCII representation of the
  // last key found (GetFound()) is written in <b>buffer</b> using the
  // WordKey::Get method.
  //
  virtual int ContextSave(String& buffer) const { found.Get(buffer); return OK; }
  //-
  // Restore from buffer all the information necessary to 
  // resume the walk at the point it left. The <b>buffer</b> is expected
  // to contain an ASCII representation of a WordKey (see WordKey::Set
  // method). A <b>Seek</b> is done on the key and the object is prepared
  // to jump to the next occurrence when <b>WalkNext</b> is called (the
  // cursor_get_flags is set to <i>DB_NEXT.</i>
  //
  virtual int ContextRestore(const String& buffer);

#ifndef SWIG
  //-
  // Walk and collect data from the index. 
  // Returns OK on success, NOTOK otherwise.
  //
  virtual int                 Walk();
#endif /* SWIG */
  //-
  // Must be called before other Walk methods are used.
  // Fill internal state according to input parameters 
  // and move before the first matching entry.
  // Returns OK on success, NOTOK otherwise.
  //
  virtual int                 WalkInit();
  //-
  // Move before the first index matching entry.
  // Returns OK on success, NOTOK otherwise.
  //
  virtual int                 WalkRewind();
  //-
  // Move to the next matching entry.
  // At end of list, WORD_WALK_ATEND is returned.
  // Returns OK on success, NOTOK otherwise.
  //
  virtual int                 WalkNext();
#ifndef SWIG
  //-
  // Advance the cursor one step. The entry pointed to by the cursor may
  // or may not match the requirements.  Returns OK if entry pointed
  // by cursor matches requirements.  Returns NOTOK on
  // failure. Returns WORD_WALK_NOMATCH_FAILED if the current entry
  // does not match requirements, it's safe to call WalkNextStep again
  // until either OK or NOTOK is returned.
  //
  virtual int                 WalkNextStep();
#endif /* SWIG */
  //-
  // Terminate Walk, free allocated resources.
  // Returns OK on success, NOTOK otherwise.
  //
  virtual int                 WalkFinish();
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

  //-
  // Move before the inverted index position specified in <b>patch.</b>
  // May only be called after a successfull call to the <i>WalkNext</i>
  // or <i>WalkNextStep</i>method.
  // Copy defined fields from <b>patch</b> into a copy of the 
  // <i>found</i> data member and 
  // initialize internal state so that <i>WalkNext</i> jumps to
  // this key next time it's called (cursor_get_flag set to DB_SET_RANGE).
  // Returns OK if successfull, NOTOK otherwise.
  //
  virtual int Seek(const WordKey& patch);

  //-
  // Returns true if cursor is positioned after the last possible
  // match, false otherwise.
  //
  virtual int IsAtEnd() const { return status == WORD_WALK_ATEND; }

  //
  // Accessors for input parameters
  //
  //-
  // Returns the search criterion.
  //
  WordKey& GetSearch() { return searchKey; }
#ifndef SWIG
  const WordKey& GetSearch() const { return searchKey; }
#endif /* SWIG */
  //-
  // Returns the type of action when a matching entry
  // is found.
  //
  int GetAction() const { return action; }
  //
  // Accessors for output parameters
  //
  //-
  // Returns the list of WordReference found. The application
  // is responsible for deallocation of the list.
  //
  List *GetResults() { return collectRes; }
  //-
  // For debugging purposes. Returns the list of WordReference hit 
  // during the search
  // process. Some of them match the searched key, some don't.
  // The application is responsible for deallocation of the list.
  //
  List *GetTraces() { return traceRes; }
  //-
  // For debugging purposes. Set the list of WordReference hit
  // during the search process. 
  //
  void SetTraces(List* traceRes_arg) { traceRes = traceRes_arg; }
  //-
  // Returns the last entry hit by the search. Only contains
  // a valid value if the last <i>WalkNext</i> or <i>WalkNextStep</i>
  // call was successfull (i.e. returned OK).
  //
  const WordReference& GetFound() { return found; }
  //-
  // Returns the number of occurrences of the searched word
  // in the inverted index in the <b>noccurrence</b> parameter.
  // Returns OK on success, NOTOK on failure.
  //
  virtual int Noccurrence(unsigned int& noccurrence) const;

#ifndef SWIG
  //-
  // Convert the whole structure to an ASCII string description
  // Returns OK if successfull, NOTOK otherwise.
  //
  virtual int Get(String& bufferout) const;
  String Get() const { String tmp; Get(tmp); return tmp; }

 protected:

  //-
  // Protected method. Derived classes should use this function to initialize
  // the object if they do not call a WordCursor constructor in their own
  // constructutor. Initialization may occur after the object is created
  // and must occur before a <b>Walk*</b> method is called. See the 
  // DESCRIPTION section for the semantics of the arguments.
  // Return OK on success, NOTOK on error.
  //
  int Initialize(WordList *nwords, const WordKey &nsearchKey, wordlist_walk_callback_t ncallback, Object * ncallback_data, int naction);

  //
  // Input parameters
  //
  //-
  // Input data. The key to be searched, see DESCRIPTION for more information.
  //
  WordKey searchKey;
  //
  // Input data. What do do when a WordReference is found.
  // Can either be
  // HTDIG_WORDLIST_COLLECTOR  WordReference found stored in collectRes
  // HTDIG_WORDLIST_WALKER     callback is called for each WordReference found
  //
  int action;

  //
  // Input data. Callback function called for each match found.
  //
  wordlist_walk_callback_t callback;
  //
  // Input data. Argument given to callback, contains arbitrary 
  // caller defined data.
  //
  Object *callback_data;

  //
  // Output parameters
  //
  //
  // Output data. List of WordReference found in the search.
  //
  List *collectRes;

  //-
  // Output data. Last match found. Use GetFound() to retrieve it.
  //
  WordReference found;
  //-
  // Output data. WORD_WALK_ATEND if cursor is past last match, 
  // OK otherwise. Use GetStatus() to retrieve it.
  //
  int status;

  //
  // Debugging section. Do not use unless you know exactly what you do.
  //
  //
  // Collect everything found while searching (not necessarily matching)
  //
  List *traceRes;

  //
  // Internal state
  //
  //
  // The actual Berkeley DB cursor.
  //
  WordDBCursor cursor;
  //
  // The latest retrieved key and data
  //
  String key;
  String data;
  //
  // The shorted prefix key computed from searchKey
  //
  WordKey prefixKey;
  //-
  // WalkNext leap is either DB_NEXT or DB_SET_RANGE.
  //
  int cursor_get_flags;
  //
  // True if search key is a prefix key
  //
  int searchKeyIsSameAsPrefix;
  //-
  // The inverted index used by this cursor.
  //
  WordList *words;
#endif /* SWIG */
};

#endif /* _WordCursor_h_ */
