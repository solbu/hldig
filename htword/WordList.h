//
// WordList.h
//
// NAME
// 
// abstract class to manage and use an inverted index file.
//
// SYNOPSIS
// 
// #include <mifluz.h>
// 
// WordContext context;
//
// WordList* words = context->List();
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
// WordList is an abstract class and cannot be instanciated. 
// The <b>List</b> method of the class WordContext will create 
// an instance using the appropriate derived class, either WordListOne
// or WordListMulti. Refer to the corresponding manual pages for
// more information on their specific semantic.
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
// wordlist_cache_max <bytes> (default 0)
//   Maximum size of the cumulated cache files generated when doing bulk
//   insertion with the <b>BatchStart()</b> function. When this limit is
//   reached, the cache files are all merged into the inverted index. 
//
// wordlist_cache_inserts {true|false} (default false)
//   If true all <b>Insert</b> calls are cached in memory. When the 
//   WordList object is closed or a different access method is called
//   the cached entries are flushed in the inverted index.
//
// wordlist_compress {true|false} (default false)
//   Activate compression of the index. The resulting index is eight times
//   smaller than the uncompressed index.
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
// $Id: WordList.h,v 1.5.2.26 2000/09/14 03:13:28 ghutchis Exp $
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
#include "WordDict.h"
#endif /* SWIG */

class List;
class WordList;
class WordDBCursor;
class WordContext;
class WordDBCaches;
class WordMeta;
class WordDead;

// 
// Inverted index interface
//
class WordList
{
 public:
    virtual ~WordList() {}

    //-
    // Return a pointer to the WordContext object used to create
    // this instance.
    //
    inline WordContext* GetContext() { return context; }
#ifndef SWIG
    //-
    // Return a pointer to the WordContext object used to create
    // this instance as a const.
    //
    inline const WordContext* GetContext() const { return context; }
#endif /* SWIG */

    //-
    // Insert <b>wordRef</b> in index. If the <i>Key()</i> part of
    // the <b>wordRef</b> exists in the index, override it.
    // Returns OK on success, NOTOK on error.
    //
    virtual inline int Override(const WordReference& wordRef) { NotImplemented(); return NOTOK; }

    //-
    // Returns OK if <b>wordRef</b> exists in the index, NOTOK otherwise.
    //
    virtual int Exists(const WordReference& wordRef) { NotImplemented(); return NOTOK; }
#ifndef SWIG
    //-
    // Returns OK if <b>word</b> exists in the index, NOTOK otherwise.
    //
    inline int Exists(const String& word) { return Dict()->Exists(word) ? OK : NOTOK; }
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
    virtual int WalkDelete(const WordReference& wordRef) { NotImplemented(); return NOTOK; }
    //-
    // Delete the entry in the index that exactly matches the
    // <i>Key()</i> part of <b>wordRef.</b>
    // Returns OK if deletion is successfull, NOTOK otherwise.
    //
    virtual int Delete(const WordReference& wordRef) { NotImplemented(); return NOTOK; }

    //-
    // Open inverted index <b>filename.</b> <b>mode</b>
    // may be <i>O_RDONLY</i> or <i>O_RDWR.</i> If mode is 
    // <i>O_RDWR</i> it can be or'ed with <i>O_TRUNC</i> to reset
    // the content of an existing inverted index.
    // Return OK on success, NOTOK otherwise.
    //
    virtual int Open(const String& filename, int mode) { NotImplemented(); return NOTOK; }
    //-
    // Close inverted index.
    // Return OK on success, NOTOK otherwise.
    // 
    virtual int Close() { NotImplemented(); return NOTOK; }
    //-
    // Return the size of the index in pages.
    //
    virtual unsigned int Size() const { NotImplemented(); return 0; }
    //-
    // Return the page size
    //
    virtual int Pagesize() const { NotImplemented(); return 0; }
    //-
    // Return a pointer to the inverted index dictionnary.
    //
    virtual WordDict *Dict() { NotImplemented(); return 0; }
    virtual WordMeta *Meta() { NotImplemented(); return 0; }
    virtual WordDead *Dead() { NotImplemented(); return 0; }
    //-
    // Return the filename given to the last call to Open.
    //
    const String& Filename() const { return filename; }
    //-
    // Return the mode given to the last call to Open.
    //
    int Flags() const { return flags; }

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
    inline List *Find(const WordReference& wordRef) { return (*this)[wordRef]; }
    //-
    // Returns the list of word occurrences exactly matching the
    // <b>word.</b> The <i>List</i> returned
    // contains pointers to <i>WordReference</i> objects. It is
    // the responsibility of the caller to free the list. See List.h
    // header for usage.
    //
    inline List *FindWord(const String& word) { return (*this)[word]; }
#ifndef SWIG
    //-
    // Alias to the <b>Find</b> method.
    //
    virtual List *operator [] (const WordReference& wordRef) { NotImplemented(); return 0; }
    //-
    // Alias to the <b>FindWord</b> method.
    //
    inline List *operator [] (const String& word)  {
      WordReference wordRef(context, word);
      unsigned int wordid;
      Dict()->SerialExists(word, wordid);
      if(wordid != WORD_DICT_SERIAL_INVALID) {
	wordRef.Key().Set(WORD_KEY_WORD, wordid);
	return (*this)[wordRef];
      } else {
	return new List;
      }
    }
#endif /* SWIG */
    //-
    // Returns the list of word occurrences matching the <i>Key()</i>
    // part of <b>wordRef.</b> In the <i>Key()</i>, the string
    // (accessed with <i>GetWord()</i>) matches any string that begins
    // with it. The <i>List</i> returned contains pointers to
    // <i>WordReference</i> objects. It is the responsibility of the
    // caller to free the list.
    //
    virtual List *Prefix (const WordReference& prefix) { NotImplemented(); return 0; }
#ifndef SWIG
    //-
    // Returns the list of word occurrences matching the
    // <b>word.</b> In the <i>Key()</i>, the string (accessed with
    // <i>GetWord()</i>) matches any string that begins with it. The
    // <i>List</i> returned contains pointers to <i>WordReference</i>
    // objects. It is the responsibility of the caller to free the
    // list.
    //
    inline List *Prefix (const String& prefix) { return this->Prefix(WordReference(context, prefix)); }
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
    virtual List *Words() { NotImplemented(); return 0; }
#endif /* SWIG */
    //- 
    // Returns a list of all entries contained in the
    // inverted index. The <i>List</i> returned contains pointers to
    // <i>WordReference</i> objects. It is the responsibility of
    // the caller to free the list. See List.h header for usage.
    //
    virtual List *WordRefs() { NotImplemented(); return 0; }

#ifndef SWIG
    //-
    // Create a cursor that searches all the occurrences in the
    // inverted index and call <b>ncallback</b> with
    // <b>ncallback_data</b> for every match.
    //
    virtual WordCursor *Cursor(wordlist_walk_callback_t callback, Object *callback_data) { NotImplemented(); return 0; }
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
    virtual WordCursor *Cursor(const WordKey &searchKey, int action = HTDIG_WORDLIST_WALKER) { NotImplemented(); return 0; }
    //-
    // Create a cursor that searches all the occurrences in the
    // inverted index and that match <b>nsearchKey</b> and calls
    // <b>ncallback</b> with <b>ncallback_data</b> for every match.
    //
    virtual WordCursor *Cursor(const WordKey &searchKey, wordlist_walk_callback_t callback, Object * callback_data) { NotImplemented(); return 0; }
#endif /* SWIG */

    //-
    // Create a WordKey object and return it. The <b>bufferin</b> argument
    // is used to initialize the key, as in the WordKey::Set method. 
    // The first component of <b>bufferin</b> must be a word that is translated
    // to the corresponding numerical id using the WordDict::Serial
    // method.
    //
    virtual WordKey Key(const String& bufferin) { NotImplemented(); return WordKey(0); }
    //-
    // Create a WordReference object and return it. The
    // <b>bufferin</b> argument is used to initialize the structure,
    // as in the WordReference::Set method.  The first component of
    // <b>bufferin</b> must be a word that is translated to the
    // corresponding numerical id using the WordDict::Serial method.
    // If the <b>exists</b> argument is set to 1, the method 
    // WordDict::SerialExists is used instead, that is no serial is
    // assigned to the word if it does not already have one.
    // Before translation the word is normalized using the
    // WordType::Normalize method. The word is saved using the
    // WordReference::SetWord method.
    //
    virtual WordReference Word(const String& bufferin, int exists = 0) { NotImplemented(); return WordReference(0); }
    //-
    // Alias for Word(bufferin, 1).
    //
    virtual WordReference WordExists(const String& bufferin) { return Word(bufferin, 1); }
    
    //-
    // Accelerate bulk insertions in the inverted index. All 
    // insertion done with the <b>Override</b> method are batched
    // instead of being updating the inverted index immediately.
    // No update of the inverted index file is done before the
    // <b>BatchEnd</b> method is called.
    // 
    virtual void BatchStart();
    //- 
    // Terminate a bulk insertion started with a call to the
    // <b>BatchStart</b> method. When all insertions are done
    // the <b>AllRef</b> method is called to restore statistics.
    //
    virtual void BatchEnd();

#ifndef SWIG
    //-
    // Return in <b>noccurrence</b> the number of occurrences of the
    // string contained in the <i>GetWord()</i> part of <b>key.</b>
    // Returns OK on success, NOTOK otherwise.
    //
    virtual int Noccurrence(const String& key, unsigned int& noccurrence) const { NotImplemented(); return NOTOK; }

    //
    // Input/Output
    //
    //-
    // Write on file descriptor <b>f</b> an ASCII description of the
    // index. Each line of the file contains a <i>WordReference</i>
    // ASCII description.
    // Return OK on success, NOTOK otherwise.
    //
    virtual int Write(FILE* f) { NotImplemented(); return NOTOK; }
    //-
    // Write on file descriptor <b>f</b> the complete dictionnary 
    // with statistics.
    // Return OK on success, NOTOK otherwise.
    //
    virtual int WriteDict(FILE* f) { NotImplemented(); return NOTOK; }
    //
    //-
    // Read <i>WordReference</i> ASCII descriptions from <b>f</b>,
    // returns the number of inserted WordReference or < 0 if an error
    // occurs. Invalid descriptions are ignored as well as empty
    // lines.
    //
    virtual int Read(FILE* f) { NotImplemented(); return NOTOK; }

#endif /* SWIG */
    //
    // Retrieve WordReferences from the database. 
    // Backend of WordRefs, operator[], Prefix...
    //
    virtual List *Collect(const WordReference& word) { NotImplemented(); return 0; }
#ifndef SWIG
    //
    // Compressor object accessors
    //
    inline WordDBCompress *GetCompressor() { return compressor; }
    inline void SetCompressor(WordDBCompress* compressor_arg) { compressor = compressor_arg; }

    inline void NotImplemented() const {
      fprintf(stderr, "WordList::NotImplemented\n");
      abort();
    }

    WordContext*		context;

    int				isopen;
    int				flags;
    String			filename;

    //
    // If true enable extended functionalities of WordList such
    // as per-word statistics. Read from wordlist_extended configuration
    // parameter.
    //
    int				extended;


    WordDBCompress	       *compressor;
    int                         verbose;

    WordDBCaches*		caches;
#endif /* SWIG */
};

#endif /* _WordList_h_ */
