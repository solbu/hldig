//
// WordCursor.h
//
// NAME
// 
// abstract class to search and retrieve entries in a WordList object.
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
// WordCursor *search = words->Cursor(WordKey("word <UNDEF> <UNDEF>"), HTDIG_WORDLIST_COLLECTOR);
//
// if(search->Walk() == NOTOK) bark;
// List* results = search->GetResults();
//
// WordCursor *search = words->Cursor(callback, data);
// WordCursor *search = words->Cursor(WordKey("word <UNDEF> <UNDEF>"));
// WordCursor *search = words->Cursor(WordKey("word <UNDEF> <UNDEF>"), callback, data);
// WordCursor *search = words->Cursor(WordKey());
//
// search->WalkInit();
// if(search->WalkNext() == OK)
//   dosomething(search->GetFound());
// search->WalkFinish();
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
// Ex1: <b>WordKey()</b> walk the entire list of occurences.
//
// Ex2: <b>WordKey("word <UNDEF> <UNDEF>")</b> find all occurrences
// of <i>word</i>.
//
// Ex3: <b>WordKey("meet <UNDEF> 1")</b> find all occurrences of
// <i>meet</i> that occur at LOCATION 1 in any DOCID. This can
// be inefficient since the search has to scan all occurrences
// of <i>meet</i> to find the ones that occur at LOCATION 1.
//
// Ex4: <b>WordKey("meet 2 <UNDEF>")</b> find all occurrences of
// <i>meet</i> that occur in DOCID 2, at any location.
//
// WordList is an abstract class and cannot be instanciated. 
// See the WordCursorOne manual page for an actual implementation of
// a WordCursor object.
//
// END
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordCursor.h,v 1.1.2.2 2000/09/14 03:13:26 ghutchis Exp $
//

#ifndef _WordCursor_h_
#define _WordCursor_h_

#ifndef SWIG
#include "htString.h"
#include "WordKey.h"
#include "WordDB.h"

class WordList;
class WordDBCursor;
//
// Possible values of the action argument of WordList::Walk
// check walk function in WordList.cc for info on these:
//
#define HTDIG_WORDLIST_COLLECTOR	0x0001
#define HTDIG_WORDLIST_WALKER		0x0002

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
  WordCursor(WordContext *context) :
    searchKey(context),
    found(context) {}

  virtual ~WordCursor() { }
#endif /* SWIG */
  //-
  // Clear all data in object, set <b>GetResult()</b> data to NULL but
  // do not delete it (the application is responsible for that).
  //
  virtual void Clear() = 0;
#ifndef SWIG
  virtual void ClearInternal() = 0;
  virtual void ClearResult() = 0;
#endif /* SWIG */

  //-
  // Returns the type of the object. May be overloaded by
  // derived classes to differentiate them at runtime.
  // Returns WORD_CURSOR.
  //
  virtual inline int IsA() const { return WORD_CURSOR; }

  //-
  // Optimize the cursor before starting a Walk.
  // Returns OK on success, NOTOK otherwise.
  //
  virtual inline int Optimize() { return OK; }

  //-
  // Save in <b>buffer</b> all the information necessary to resume
  // the walk at the point it left. The ASCII representation of the
  // last key found (GetFound()) is written in <b>buffer</b> using the
  // WordKey::Get method.
  //
  virtual int ContextSave(String& buffer) const = 0;
  //-
  // Restore from buffer all the information necessary to 
  // resume the walk at the point it left. The <b>buffer</b> is expected
  // to contain an ASCII representation of a WordKey (see WordKey::Set
  // method). A <b>Seek</b> is done on the key and the object is prepared
  // to jump to the next occurrence when <b>WalkNext</b> is called (the
  // cursor_get_flags is set to <i>DB_NEXT.</i>
  //
  virtual int ContextRestore(const String& buffer) = 0;

  //-
  // Walk and collect data from the index. 
  // Returns OK on success, NOTOK otherwise.
  //
  virtual int Walk() = 0;
  //-
  // Must be called before other Walk methods are used.
  // Fill internal state according to input parameters 
  // and move before the first matching entry.
  // Returns OK on success, NOTOK otherwise.
  //
  virtual int WalkInit() = 0;
  //-
  // Move before the first index matching entry.
  // Returns OK on success, NOTOK otherwise.
  //
  virtual int WalkRewind() = 0;
  //-
  // Move to the next matching entry.
  // At end of list, WORD_WALK_ATEND is returned.
  // Returns OK on success, NOTOK otherwise.
  //
  virtual int WalkNext() = 0;
#ifndef SWIG
  //-
  // Advance the cursor one step. The entry pointed to by the cursor may
  // or may not match the requirements.  Returns OK if entry pointed
  // by cursor matches requirements.  Returns NOTOK on
  // failure. Returns WORD_WALK_NOMATCH_FAILED if the current entry
  // does not match requirements, it's safe to call WalkNextStep again
  // until either OK or NOTOK is returned.
  //
  virtual int WalkNextStep() = 0;
#endif /* SWIG */
  //-
  // Terminate Walk, free allocated resources.
  // Returns OK on success, NOTOK otherwise.
  //
  virtual int WalkFinish() = 0;

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
  virtual int Seek(const WordKey& patch) = 0;

  //-
  // Returns true if cursor is positioned after the last possible
  // match, false otherwise.
  //
  virtual inline int IsAtEnd() const { return status == WORD_WALK_ATEND; }

  //
  // Accessors for input parameters
  //
  //-
  // Returns the search criterion.
  //
  inline WordKey& GetSearch() { return searchKey; }
#ifndef SWIG
  inline const WordKey& GetSearch() const { return searchKey; }
#endif /* SWIG */
  //-
  // Returns the type of action when a matching entry
  // is found.
  //
  inline int GetAction() const { return action; }
  //
  // Accessors for output parameters
  //
  //-
  // Returns the list of WordReference found. The application
  // is responsible for deallocation of the list. If the <b>action</b>
  // input flag bit HTDIG_WORDLIST_COLLECTOR is not set, return a NULL
  // pointer.
  //
  inline List *GetResults() { return collectRes; }
#ifndef SWIG
  //-
  // For debugging purposes. Returns the list of WordReference hit 
  // during the search
  // process. Some of them match the searched key, some don't.
  // The application is responsible for deallocation of the list.
  //
  inline List *GetTraces() { return traceRes; }
  //-
  // For debugging purposes. Set the list of WordReference hit
  // during the search process. 
  //
  inline void SetTraces(List* traceRes_arg) { traceRes = traceRes_arg; }
#endif /* SWIG */
  //-
  // Returns the last entry hit by the search. Only contains
  // a valid value if the last <i>WalkNext</i> or <i>WalkNextStep</i>
  // call was successfull (i.e. returned OK).
  //
  inline const WordReference& GetFound() { return found; }
  //-
  // Returns the status of the cursor which may be 
  // OK or WORD_WALK_ATEND.
  //
  inline int GetStatus() const { return status; }

#ifndef SWIG
  //-
  // Convert the whole structure to an ASCII string description.
  // Returns OK if successfull, NOTOK otherwise.
  //
  virtual int Get(String& bufferout) const = 0;
#endif /* SWIG */
  //-
  // Convert the whole structure to an ASCII string description
  // and return it.
  //
  inline String Get() const { String tmp; Get(tmp); return tmp; }

#ifndef SWIG
 protected:

  //-
  // Protected method. Derived classes should use this function to initialize
  // the object if they do not call a WordCursor constructor in their own
  // constructutor. Initialization may occur after the object is created
  // and must occur before a <b>Walk*</b> method is called. See the 
  // DESCRIPTION section for the semantics of the arguments.
  // Return OK on success, NOTOK on error.
  //
  virtual int Initialize(WordList *nwords, const WordKey &nsearchKey, wordlist_walk_callback_t ncallback, Object * ncallback_data, int naction) = 0;


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
  //-
  // The inverted index used by this cursor.
  //
  WordList *words;
#endif /* SWIG */
};

#endif /* _WordCursor_h_ */
