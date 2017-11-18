//
// WordList.h
//
// NAME
// 
// manage and use an inverted index file.
//
// SYNOPSIS
// 
// #include <mifluz.h>
// 
// Configuration* config;
// WordReference wordRef;
// ...
// WordList* words = new WordList(config)
// 
// delete words;
// 
// DESCRIPTION
// 
// WordList is the <i>mifluz</i> equivalent of a database handler. Each
// WordList object is bound to an inverted index file and implements the
// operations to create it, fill it with word occurrences and search 
// for an entry matching a given criterion.
// 
// CONFIGURATION
// 
// wordlist_extend {true|false} (default false)
//   If <b>true</b> maintain reference count of unique 
//   words. The <b>Noccurrence</b> method gives access to this count.
// 
// wordlist_verbose <number> (default 0)
//   Set the verbosity level of the WordList class. 
//   <br>
//   1 walk logic
//   <br>
//   2 walk logic details
//   <br>
//   3 walk logic lots of details
// 
// wordlist_page_size <bytes> (default 8192)
//   Berkeley DB page size (see Berkeley DB documentation)
// 
// wordlist_cache_size <bytes> (default 500K)
//   Berkeley DB cache size (see Berkeley DB documentation)
//   Cache makes a huge difference in performance. It must be at least 2%
//   of the expected total data size. Note that if compression is activated
//   the data size is eight times larger than the actual file size. In this
//   case the cache must be scaled to 2% of the data size, not 2% 
//   of the file size. See <b>Cache tuning</b> in the mifluz guide for
//   more hints.
// 
// wordlist_compress {true|false} (default false)
//   Activate compression of the index. The resulting index is eight times
//   smaller than the uncompressed index.
// 
//
// END
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: WordList.h,v 1.10 2004/05/28 13:15:28 lha Exp $
//

#ifndef _WordList_h_
#define _WordList_h_

#include <fcntl.h>
#include <stdio.h>

#ifndef SWIG
#include "Dictionary.h"
#include "List.h"
#include "htString.h"
#include "WordRecord.h"
#include "WordReference.h"
#include "WordType.h"
#include "WordDB.h"
#include "WordDBCompress.h"
#include "Configuration.h"
#include "WordCursor.h"
#endif /* SWIG */

class List;
class WordList;
class WordDBCursor;

// 
// Inverted index interface
//
class WordList
{
public:
    //-
    // Constructor. Build inverted index handling object using
    // run time configuration parameters listed in the <b>CONFIGURATION</b>
    // section.
    //
    WordList(const Configuration& config_arg);
    virtual ~WordList();
    
    //-
    // Insert <b>wordRef</b> in index. It is an error to insert
    // the same <b>wordRef</b> twice. This requires a lookup in the index 
    // prior to the insertion.
    // Returns OK on success, NOTOK on error.
    //
    int      Insert(const WordReference& wordRef) { return Put(wordRef, DB_NOOVERWRITE); }
    //-
    // Insert <b>wordRef</b> in index. If the <i>Key()</i> part of
    // the <b>wordRef</b> exists in the index, override it.
    // Returns OK on success, NOTOK on error.
    //
    int      Override(const WordReference& wordRef) { return Put(wordRef, 0); }
#ifndef SWIG
    int                 Put(const WordReference& wordRef, int flags);
#endif /* SWIG */

    //-
    // Returns OK if <b>wordRef</b> exists in the index, NOTOK otherwise.
    //
    int                 Exists(const WordReference& wordRef) { return db.Exists(wordRef) == 0 ? OK : NOTOK; }
#ifndef SWIG
    //-
    // Returns OK if <b>word</b> exists in the index, NOTOK otherwise.
    //
    int                 Exists(const String& word) { return Exists(WordReference(word)); }
#endif /* SWIG */

    //
    // Delete permanently
    //
    //-
    // Delete all entries in the index whose key matches the 
    // <i>Key()</i> part of <b>wordRef</b>, using the <i>Walk</i>
    // method.
    // Returns the number of entries successfully deleted.
    //
    int                 WalkDelete(const WordReference& wordRef);
    //-
    // Delete the entry in the index that exactly matches the
    // <i>Key()</i> part of <b>wordRef.</b>
    // Returns OK if deletion is successfull, NOTOK otherwise.
    //
    int                 Delete(const WordReference& wordRef) {
      if(db.Del(wordRef) == 0)
  return Unref(wordRef);
      else
  return NOTOK;
    }
#ifdef SWIG
%name(DeleteCursor)
#endif /* SWIG */
    //-
    // Delete the inverted index entry currently pointed to by the
    // <b>cursor.</b> 
    // Returns 0 on success, Berkeley DB error code on error. This
    // is mainly useful when implementing a callback function for
    // a <b>WordCursor.</b> 
    //
    int                 Delete(WordDBCursor& cursor) { return cursor.Del(); }

    //-
    // Open inverted index <b>filename.</b> <b>mode</b>
    // may be <i>O_RDONLY</i> or <i>O_RDWR.</i> If mode is 
    // <i>O_RDWR</i> it can be or'ed with <i>O_TRUNC</i> to reset
    // the content of an existing inverted index.
    // If  word_only  is true, entries will compare equal if the "word" part
    // of the key is equal, even if the numeric fields aren't.  (What are the
    // numeric fields, anyway??)
    // Return OK on success, NOTOK otherwise.
    //
    int                 Open(const String& filename, int mode, int word_only=false);
    //-
    // Close inverted index.
    // 
    int      Close();

    //
    // These returns a list of all the WordReference * matching 
    // the constraint.
    //-
    // Returns the list of word occurrences exactly matching the
    // <i>Key()</i> part of <b>wordRef.</b> The <i>List</i> returned
    // contains pointers to <i>WordReference</i> objects. It is
    // the responsibility of the caller to free the list. See List.h
    // header for usage.
    //
    List    *Find(const WordReference& wordRef) { return (*this)[wordRef]; }
    //-
    // Returns the list of word occurrences exactly matching the
    // <b>word.</b> The <i>List</i> returned
    // contains pointers to <i>WordReference</i> objects. It is
    // the responsibility of the caller to free the list. See List.h
    // header for usage.
    //
    List    *FindWord(const String& word) { return (*this)[word]; }
#ifndef SWIG
    //-
    // Alias to the <b>Find</b> method.
    //
    List    *operator [] (const WordReference& wordRef);
    //-
    // Alias to the <b>FindWord</b> method.
    //
    List    *operator [] (const String& word)  { return (*this)[WordReference(word)]; }
#endif /* SWIG */
    //-
    // Returns the list of word occurrences matching the <i>Key()</i>
    // part of <b>wordRef.</b> In the <i>Key()</i>, the string
    // (accessed with <i>GetWord()</i>) matches any string that begins
    // with it. The <i>List</i> returned contains pointers to
    // <i>WordReference</i> objects. It is the responsibility of the
    // caller to free the list.
    //
    List    *Prefix (const WordReference& prefix);
#ifndef SWIG
    //-
    // Returns the list of word occurrences matching the
    // <b>word.</b> In the <i>Key()</i>, the string (accessed with
    // <i>GetWord()</i>) matches any string that begins with it. The
    // <i>List</i> returned contains pointers to <i>WordReference</i>
    // objects. It is the responsibility of the caller to free the
    // list.
    //
    List    *Prefix (const String& prefix) { return this->Prefix(WordReference(prefix)); }
#endif /* SWIG */

    //
    // Iterate over the complete database.
    //
#ifndef SWIG
    //- 
    // Returns a list of all unique words contained in the inverted
    // index. The <i>List</i> returned contains pointers to
    // <i>String</i> objects. It is the responsibility of the caller
    // to free the list. See List.h header for usage.
    //
    List                *Words();
#endif /* SWIG */
    //- 
    // Returns a list of all entries contained in the
    // inverted index. The <i>List</i> returned contains pointers to
    // <i>WordReference</i> objects. It is the responsibility of
    // the caller to free the list. See List.h header for usage.
    //
    List    *WordRefs();

#ifndef SWIG
    //-
    // Create a cursor that searches all the occurrences in the
    // inverted index and call <b>ncallback</b> with
    // <b>ncallback_data</b> for every match.
    //
    WordCursor *Cursor(wordlist_walk_callback_t callback, Object *callback_data) { return new WordCursor(this, callback, callback_data); }
#endif /* SWIG */
    //- 
    // Create a cursor that searches all the occurrences in the
    // inverted index and that match <b>nsearchKey.</b> If
    // <b>naction</b> is set to HTDIG_WORDLIST_WALKER calls
    // <b>searchKey.callback</b> with <b>searchKey.callback_data</b>
    // for every match. If <b>naction</b> is set to
    // HTDIG_WORDLIST_COLLECT push each match in <b>searchKey.collectRes</b>
    // data member as a <b>WordReference</b> object. It is the responsibility
    // of the caller to free the <b>searchKey.collectRes</b> list.
    //
    WordCursor *Cursor(const WordKey &searchKey, int action = HTDIG_WORDLIST_WALKER) { return new WordCursor(this, searchKey, action); }
#ifndef SWIG
    //-
    // Create a cursor that searches all the occurrences in the
    // inverted index and that match <b>nsearchKey</b> and calls
    // <b>ncallback</b> with <b>ncallback_data</b> for every match.
    //
    WordCursor *Cursor(const WordKey &searchKey, wordlist_walk_callback_t callback, Object * callback_data) { return new WordCursor(this, searchKey, callback, callback_data); }
#endif /* SWIG */

    //
    // Update/get global word statistics statistics
    //
    //-
    // Add one to the reference count for the string contained
    // in the <i>Key().GetWord()</i> part of <b>wordRef.</b>
    // Returns OK on success, NOTOK otherwise.
    //
    int Ref(const WordReference& wordRef);
    //-
    // Substract one to the reference count for the string contained
    // in the <i>Key().GetWord()</i> part of <b>wordRef.</b>
    // Returns OK on success, NOTOK otherwise.
    //
    int Unref(const WordReference& wordRef);
#ifndef SWIG
    //-
    // Return in <b>noccurrence</b> the number of occurrences of the
    // string contained in the <i>GetWord()</i> part of <b>key.</b>
    // Returns OK on success, NOTOK otherwise.
    //
    int Noccurrence(const WordKey& key, unsigned int& noccurrence) const;

    //
    // Accessors
    //
    //
    // Get the Berkeley DB object
    //
    const WordType&      GetWordType() const { return wtype; }
#endif /* SWIG */
    //-
    // Return the <i>Configuration</i> object used to initialize
    // the <i>WordList</i> object. 
    //
    const Configuration& GetConfiguration() const { return config; }

#ifndef SWIG
    //
    // Input/Output
    //
    //-
    // Write on file descriptor <b>f</b> an ASCII description of the
    // index. Each line of the file contains a <i>WordReference</i>
    // ASCII description.
    // Returns 0 on success, not 0 otherwise.
    //
    int Write(FILE* f);
    //
    //-
    // Read <i>WordReference</i> ASCII descriptions from <b>f</b>,
    // returns the number of inserted WordReference or < 0 if an error
    // occurs. Invalid descriptions are ignored as well as empty
    // lines.
    //
    int Read(FILE* f);

#endif /* SWIG */
    //
    // Retrieve WordReferences from the database. 
    // Backend of WordRefs, operator[], Prefix...
    //
    List    *Collect(const WordReference& word);
#ifndef SWIG
    //
    // Compressor object accessors
    //
    WordDBCompress *GetCompressor() { return compressor; }
    void SetCompressor(WordDBCompress* compressor_arg) { compressor = compressor_arg; }

    const WordType    wtype;
    const Configuration&  config;

    int        isopen;
    int        isread;

    //
    // If true enable extended functionalities of WordList such
    // as per-word statistics. Read from wordlist_extended configuration
    // parameter.
    //
    int        extended;


    WordDB                db;
    WordDBCompress         *compressor;
    int                         verbose;
#endif /* SWIG */
};

#endif /* _WordList_h_ */

