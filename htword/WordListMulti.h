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
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordListMulti.h,v 1.2 2002/02/02 18:18:13 ghutchis Exp $
//

#ifndef _WordListMulti_h_
#define _WordListMulti_h_

#include <fcntl.h>
#include <stdio.h>

#ifndef SWIG
#include "WordList.h"
#include "WordCursorOne.h"
//#include "WordCursorMulti.h"
#endif /* SWIG */

class WordContext;

// 
// Inverted index interface
//
class WordListMulti : public WordList
{
 public:
    //-
    // Constructor. Build inverted index handling object using
    // run time configuration parameters listed in the <b>CONFIGURATION</b>
    // section.
    //
    WordListMulti(WordContext* ncontext);
    virtual ~WordListMulti();

#ifndef SWIG
    virtual int Override(const WordReference& wordRef);
#endif /* SWIG */

    //-
    // Returns OK if <b>wordRef</b> exists in the index, NOTOK otherwise.
    //
    virtual int Exists(const WordReference& wordRef);

    //
    // Delete permanently
    //
    //-
    // Delete all entries in the index whose key matches the 
    // <i>Key()</i> part of <b>wordRef</b>, using the <i>Walk</i>
    // method.
    // Returns the number of entries successfully deleted.
    //
    virtual int WalkDelete(const WordReference& wordRef);
    //-
    // Delete the entry in the index that exactly matches the
    // <i>Key()</i> part of <b>wordRef.</b>
    // Returns OK if deletion is successfull, NOTOK otherwise.
    //
    virtual int Delete(const WordReference& wordRef);

    //-
    // Open inverted index <b>filename.</b> <b>mode</b>
    // may be <i>O_RDONLY</i> or <i>O_RDWR.</i> If mode is 
    // <i>O_RDWR</i> it can be or'ed with <i>O_TRUNC</i> to reset
    // the content of an existing inverted index.
    // Return OK on success, NOTOK otherwise.
    //
    virtual int Open(const String& filename, int mode);
    //-
    // Close inverted index.
    // Return OK on success, NOTOK otherwise.
    // 
    virtual int Close();
    //-
    // Return the size of the index in pages.
    //
    virtual unsigned int Size() const;
    int AddIndex();
    int Merge();

    //-
    // Alias to the <b>Find</b> method.
    //
    virtual List *operator [] (const WordReference& wordRef);
    //-
    // Returns the list of word occurrences matching the <i>Key()</i>
    // part of <b>wordRef.</b> In the <i>Key()</i>, the string
    // (accessed with <i>GetWord()</i>) matches any string that begins
    // with it. The <i>List</i> returned contains pointers to
    // <i>WordReference</i> objects. It is the responsibility of the
    // caller to free the list.
    //
    virtual List *Prefix (const WordReference& prefix);

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
    virtual List *Words();
#endif /* SWIG */
    //- 
    // Returns a list of all entries contained in the
    // inverted index. The <i>List</i> returned contains pointers to
    // <i>WordReference</i> objects. It is the responsibility of
    // the caller to free the list. See List.h header for usage.
    //
    virtual List *WordRefs();

#ifndef SWIG
    //-
    // Create a cursor that searches all the occurrences in the
    // inverted index and call <b>ncallback</b> with
    // <b>ncallback_data</b> for every match.
    //
    virtual inline WordCursor *Cursor(wordlist_walk_callback_t callback, Object *callback_data) { return new WordCursorOne(this, callback, callback_data); }
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
    virtual inline WordCursor *Cursor(const WordKey &searchKey, int action = HTDIG_WORDLIST_WALKER) { return new WordCursorOne(this, searchKey, action); }
#ifndef SWIG
    //-
    // Create a cursor that searches all the occurrences in the
    // inverted index and that match <b>nsearchKey</b> and calls
    // <b>ncallback</b> with <b>ncallback_data</b> for every match.
    //
    virtual inline WordCursor *Cursor(const WordKey &searchKey, wordlist_walk_callback_t callback, Object * callback_data) { return new WordCursorOne(this, searchKey, callback, callback_data); }
#endif /* SWIG */

    //
    // Update/get global word statistics statistics
    //
    //-
    // Add one to the reference count for the string contained
    // in the <i>Key().GetWord()</i> part of <b>wordRef.</b>
    // Returns OK on success, NOTOK otherwise.
    //
    virtual int Ref(const WordReference& wordRef);
    //-
    // Substract one to the reference count for the string contained
    // in the <i>Key().GetWord()</i> part of <b>wordRef.</b>
    // Returns OK on success, NOTOK otherwise.
    //
    virtual int Unref(const WordReference& wordRef);
    virtual int AllRef();

#ifndef SWIG
    //-
    // Return in <b>noccurrence</b> the number of occurrences of the
    // string contained in the <i>GetWord()</i> part of <b>key.</b>
    // Returns OK on success, NOTOK otherwise.
    //
    virtual int Noccurrence(const String& key, unsigned int& noccurrence) const;
    virtual int Write(FILE* f) { return NOTOK; }
    virtual int Read(FILE* f) { return NOTOK; }

    virtual WordKey Key(const String& bufferin) { abort(); return WordKey(0); }

    virtual WordReference Word(const String& bufferin, int exists = 0) { abort(); return WordReference(0); }

#endif /* SWIG */
    //
    // Retrieve WordReferences from the database. 
    // Backend of WordRefs, operator[], Prefix...
    //
    virtual List *Collect(const WordReference& word);
#ifndef SWIG
    List*            	dbs;
    int			serial;
    int			file_max;
    int			file_min;
    unsigned int	put_max;
#endif /* SWIG */
};

#endif /* _WordListMulti_h_ */

