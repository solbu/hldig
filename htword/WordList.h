//
// WordList.h
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
// $Id: WordList.h,v 1.5.2.22 2000/01/28 22:59:14 loic Exp $
//

#ifndef _WordList_h_
#define _WordList_h_

#include <fcntl.h>

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
#endif /* SWIG */

class List;
class WordList;
class WordDBCursor;
class WordMonitor;
//
// Possible values of the action argument of WordList::Walk
// check walk function in WordList.cc for info on these:
//
#define HTDIG_WORDLIST_COLLECTOR	0x0001
#define HTDIG_WORDLIST_WALKER		0x0002

#ifndef SWIG
//
// Type of the callback argument in WordSearchDescription
//
typedef int (*wordlist_walk_callback_t)(WordList *, WordDBCursor& , const WordReference *, Object &);

//
// Possible values of the status member
//
//
// WordSearchDescription contains valid data
//
#define WORD_WALK_OK			0x0000
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
// WordSearchDescription contains undefined data
//
#define WORD_WALK_FAILED		0xffff

//
// Wordlist::Walk uses WordSearchDescription for :
// state information : cursor
// search term description
// debug/trace/benchmarking
// search result format description
//
class WordSearchDescription
{
    friend WordList;
 public:
    WordSearchDescription(wordlist_walk_callback_t ncallback, Object * ncallback_data);
    WordSearchDescription(const WordKey &nsearchKey, int naction = HTDIG_WORDLIST_WALKER);
    WordSearchDescription(const WordKey &nsearchKey, wordlist_walk_callback_t ncallback, Object * ncallback_data);

    void Clear();
    void ClearInternal();
    void ClearResult();

    //
    // Set the document number in key and prepare to
    // move to this document, if any.
    //
    int ModifyKey(const WordKey& patch);

    //
    // Input parameters
    //
    //
    // The key to be searched
    //
    WordKey searchKey;
    //
    // What do do when a WordReference is found
    // Can either be
    // HTDIG_WORDLIST_COLLECTOR  WordReference found stored in collectRes
    // HTDIG_WORDLIST_WALKER     callback is called for each WordReference found
    //
    int action;
    //
    // Callback function called for each WordReference found
    //
    wordlist_walk_callback_t callback;
    //
    // Argument given to callback, contains arbitrary caller defined data
    //
    Object *callback_data;

    //
    // Output parameters
    //
    //
    // List of WordReference found in the search
    //
    List *collectRes;
    //
    // Last match found.
    //
    WordReference found;
    //
    // Description of the last NOTOK condition
    //
    int status;

    //
    // Debugging section. Do not use unless you know exactly what you do.
    //
    //
    // Do not not skip entries
    //
    int noskip;
    //
    // Collect everything found while searching (not necessarily matching)
    //
    List *traceRes;

 private:
    //
    // Internal state
    //
    //
    // A pointer to the current location of the search within the index
    // Allows to resume search if necessary.
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
    //
    // Next leap is either DB_NEXT or DB_SET_RANGE
    //
    int cursor_get_flags;
    //
    // True if search key is a prefix key
    //
    int searchKeyIsSameAsPrefix;

    int first_skip_field;
};
#endif /* SWIG */

// 
// Inverted index interface
//
class WordList
{
public:
    //
    // Construction/Destruction
    //
    WordList(const Configuration& config_arg);
    virtual ~WordList();
    
    //
    // Insert
    //
    int			Insert(const WordReference& wordRef) { return Put(wordRef, DB_NOOVERWRITE); }
    int			Override(const WordReference& wordRef) { return Put(wordRef, 0); }
#ifndef SWIG
    int                 Put(const WordReference& wordRef, int flags);
#endif /* SWIG */

    //
    // Check for existence. Returns OK if exists, NOTOK otherwise.
    //
    int                 Exists(const WordReference& wordRef) { return db.Exists(wordRef); }
#ifndef SWIG
    int                 Exists(const String& word) { return Exists(WordReference(word)); }
#endif /* SWIG */

    //
    // Delete permanently
    //
    //
    // Returns the number of entries successfully deleted.
    //
    int                 WalkDelete(const WordReference& wordRef);
    //
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
    int                 Delete(WordDBCursor& cursor) { return cursor.Del() == OK ? 1 : 0; }

    //
    // Open underlying db file
    // mode may be O_RDONLY or O_RDWR
    //
    int                 Open(const String& filename, int mode);
    //
    // Close underlying db file
    // 
    int			Close();

    //
    // This returns a list of all the WordReference * matching 
    // the constraint.
    //
    // Return the list of word occurences exactly matching the wordRef
    //
    List		*Find(const WordReference& wordRef) { return (*this)[wordRef]; }
    List		*FindWord(const String& word) { return (*this)[word]; }
#ifndef SWIG
    List		*operator [] (const WordReference& wordRef);
    List		*operator [] (const String& word)  { return (*this)[WordReference(word)]; }
#endif /* SWIG */
    //
    // Return the list of word occurences matching the beginning of wordRef
    //
    List		*Prefix (const WordReference& prefix);
#ifndef SWIG
    List		*Prefix (const String& prefix) { return this->Prefix(WordReference(prefix)); }

    //
    // Iterate over the complete database.
    //
    // This returns a list of all the Words, as String *
    List                *Words();
    // This returns a list of all the Words, as WordReference *
    List		*WordRefs();

    //
    // Walk and collect data from the word database.
    // Backend of Collect, Dump, Delete...
    //
    int                 Walk(WordSearchDescription &search);
    //
    // Fill internal state according to input parameters.
    // Move to the first matching entry.
    // Returns OK if successfull, NOTOK if it fails.
    //
    int                 WalkInit(WordSearchDescription& search);
    //
    // Move to the first matching entry.
    // Returns OK if successfull, NOTOK if it fails.
    //
    int                 WalkRewind(WordSearchDescription& search);
    //
    // Move to the next match
    // Returns OK if successfull, NOTOK if it fails.
    //
    int                 WalkNext(WordSearchDescription& search);
    //
    // Advance the cursor one step, be it a match or not
    // Returns OK if successfull, NOTOK if it fails.
    // If NOTOK and WORD_WALK_NOMATCH_FAILED is in status, it's safe to
    // call WalkNextStep again.
    //
    int                 WalkNextStep(WordSearchDescription& search);
    //
    // Terminate walk, free allocated resources.
    // Returns OK if successfull, NOTOK if it fails.
    //
    int                 WalkFinish(WordSearchDescription& search);
    List               *Collect(const WordSearchDescription &search);

    //
    // Update/get global word statistics statistics
    //
    int Ref(const WordReference& wordRef);
    int Unref(const WordReference& wordRef);
    int Noccurence(const WordKey& key, unsigned int& noccurence) const;

    //
    // Accessors
    //
    const WordType&      GetWordType() const { return wtype; }
    const Configuration& GetConfiguration() const { return config; }

    //
    // Input/Output
    //
    friend ostream &operator << (ostream &o, WordList &list); 
    friend istream &operator >> (istream &o, WordList &list); 

 protected:
    //
    // Retrieve WordReferences from the database. 
    // Backend of WordRefs, operator[], Prefix...
    //
    List		*Collect (const WordReference& word);
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
    int SkipUselessSequentialWalking(WordSearchDescription &search);
    //
    // Compressor object accessors
    //
    WordDBCompress *GetCompressor() { return compressor; }
    void SetCompressor(WordDBCompress* compressor_arg) { compressor = compressor_arg; }

    const WordType		wtype;
    const Configuration&	config;

    int				isopen;
    int				isread;

    //
    // If true enable extended functionalities of WordList such
    // as per-word statistics. Read from wordlist_extended configuration
    // parameter.
    //
    int				extended;

 private:

    WordDB	            	db;
    WordDBCompress	       *compressor;
    int                         verbose;

 protected:
    //
    // Debugging section. Do not use unless you know exactly what you do.
    //
 public:
    int    bm_put_count;
    double bm_put_time;
    int    bm_walk_count;
    double bm_walk_time;
    int    bm_walk_count_DB_SET_RANGE;
    int    bm_walk_count_DB_NEXT;
    WordMonitor *monitor;
#endif /* SWIG */
};

#endif
