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
// $Id: WordList.h,v 1.5.2.11 2000/01/06 13:58:29 bosc Exp $
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
#include "Configuration.h"
#endif /* SWIG */

class List;
class WordList;
class WordCursor;
class WordMonitor;
//
// Possible values of the action argument of WordList::Walk
// check walk function in WordList.cc for info on these:
//
#define HTDIG_WORDLIST_COLLECTOR	0x0001
#define HTDIG_WORDLIST_WALKER		0x0002

#ifndef SWIG
//
// Type of the callback argument of WordList::Walk
//
typedef int (*wordlist_walk_callback_t)(WordList *words, WordCursor& cursor, const WordReference *word, Object &data);

class WordBenchmarking
{
public:
    int nDB_NEXT;
    int nDB_SET_RANGE;
    int nSkip;
    void show(){cout << "benchmarking: nDB_SET_RANGE:" << nDB_SET_RANGE << " nDB_NEXT:" << nDB_NEXT <<  " nSkip:" << nSkip << endl;}
    WordBenchmarking()
    {
	nDB_NEXT=0;
	nDB_SET_RANGE=0;
	nSkip=0;
    }
};



// **************************************************
// *************** WordSearchDescription  ***********
// **************************************************
// this is the class that Wordlist::Walk uses for :
// state information : cursor
// search term description
// debug/trace/benchmarking
// search result format description
//
// it is still under developpement
class WordList;

class WordSearchDescription
{
    friend WordList;
// search key
// prefix key
// actions
// tracing
// benchmarking    
// cursor state
// skip info: i0
// max num results
// min num results
// results!!

// constructors : (called when using:  WordList::Walk(WordSearchDescription searchDescription))
// ex list.Walk("toto<UNDEF> 1000 <UNDEF>")
//    list.Walk(wordRef,callback,callback_data); (current Walk implementation)
//    list.Walk(wordKey)
//    list.Walk()


 protected:
    // internal information
    int first_skip_field;
    int setup_ok;


    void Clear();
    int  setup();
 public:
    //  search description
    WordKey searchKey;

    // what do do when something is found
    wordlist_walk_callback_t callback;
    Object *callback_data;
    int action;
    List *collectRes;

    // user set flags
    int noskip;// debuging : don't use skip
    int shutup;// debuging : don't verbose, even if WordList  verbose set

    // tracing/benchmarking (debuging)
    List *traceRes; // trace what's going on in Walk (intended for debuging only)
    WordBenchmarking *benchmarking;

public:
    WordSearchDescription(const WordReference& wordRef, int naction, wordlist_walk_callback_t ncallback, Object *ncallback_data);
    WordSearchDescription(const WordKey &nsearchKey);
    WordSearchDescription(const WordKey &nsearchKey,wordlist_walk_callback_t ncallback,Object * ncallback_data);
};
#endif /* SWIG */


// **************************************************
// *************** WordList   ***********************
// **************************************************

class WordList
{
public:
    //
    // Construction/Destruction
    //
    WordList(const Configuration& config_arg);
    virtual ~WordList();
    

    // WordList specific debuging flag
    int                         verbose;

    //
    // Insert
    //
    int			Insert(const WordReference& wordRef) { return Put(wordRef, DB_NOOVERWRITE); }
    int			Override(const WordReference& wordRef) { return Put(wordRef, 0); }
    int                 Put(const WordReference& wordRef, int flags);

    //
    // Check for existence
    //
    int                 Exists(const WordReference& wordRef) { return db.Exists(wordRef); }
#ifndef SWIG
    int                 Exists(const String& word) { return Exists(WordReference(word)); }
#endif /* SWIG */

    //
    // Delete permanently
    //
    int                 WalkDelete(const WordReference& wordRef);
    int                 Delete(const WordReference& wordRef) {
      if(db.Del(wordRef) == 0)
	return Unref(wordRef);
      else
	return NOTOK;
    }
#ifdef SWIG
%name(DeleteCursor)
#endif /* SWIG */
    int                 Delete(WordCursor& cursor) { return cursor.Del() == OK ? 1 : 0; }

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
#endif /* SWIG */



    //
    // Iterate over the complete database.
    //

#ifndef SWIG
    // This returns a list of all the Words, as String *
    List                *Words();
    // This returns a list of all the Words, as WordReference *
    List		*WordRefs();
#endif /* SWIG */

    //
    // Walk and collect data from the word database.
    // Backend of Collect, Dump, Delete...
    //
#ifndef SWIG
//      List 		*Walk (const WordReference& word, int action, wordlist_walk_callback_t callback, Object &callback_data);
    int   Walk(      WordSearchDescription &SearchDescription);
    List *Search(const WordSearchDescription &SearchDescription);
    int SkipUselessSequentialWalking(const WordSearchDescription &search,WordKey &foundKey,String &key,int &cursor_get_flags);


#endif /* SWIG */

    //
    // Update/get global word statistics statistics
    //
    int Ref(const WordReference& wordRef);
    int Unref(const WordReference& wordRef);
    int Noccurence(const WordKey& key, unsigned int& noccurence) const;

    //
    // Accessors
    //
#ifndef SWIG
    const WordType&      GetWordType() const { return wtype; }
#endif /* SWIG */
    const Configuration& GetConfiguration() const { return config; }
    static void Initialize(const Configuration &config0);
    

#ifndef SWIG
protected:
    //
    // Retrieve WordReferences from the database. 
    // Backend of WordRefs, operator[], Prefix...
    //
    List		*Collect (const WordReference& word);

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


    friend ostream &operator << (ostream &o, WordList &list); 
    friend istream &operator >> (istream &o, WordList &list); 

private:

    WordDB	            	db;
#endif /* SWIG */


// DEBUGING / BENCHMARKING
 protected:
    DB_CMPR_INFO* cmprInfo; // compressor keeps it's own benchmarking info
    inline void WalkBenchmark_Get(WordSearchDescription &search, int cursor_get_flags);
 public:
    int    bm_put_count;
    double bm_put_time;
    int    bm_walk_count;
    double bm_walk_time;
    int    bm_walk_count_DB_SET_RANGE;
    int    bm_walk_count_DB_NEXT;

    inline WordDBCompress *GetCompressor()
	{return(cmprInfo ?  (WordDBCompress *)cmprInfo->user_data : (WordDBCompress *)NULL);}

    WordMonitor *monitor;
};

#endif
