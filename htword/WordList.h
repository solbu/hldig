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
// $Id: WordList.h,v 1.5.2.3 1999/11/02 11:40:43 bosc Exp $
//

#ifndef _WordList_h_
#define _WordList_h_

#include <fcntl.h>

#include "Dictionary.h"
#include "List.h"
#include "htString.h"
#include "WordRecord.h"
#include "WordReference.h"
#include "WordType.h"
#include "WordDB.h"
#include "Configuration.h"

class WordList;
class WordCursor;

//
// Possible values of the action argument of WordList::Walk
// check walk function in WordList.cc for info on these:
//
#define HTDIG_WORDLIST_COLLECTOR	0x0001
#define HTDIG_WORDLIST_WALKER		0x0002

//
// Type of the callback argument of WordList::Walk
//
typedef int (*wordlist_walk_callback_t)(WordList *words, WordCursor& cursor, const WordReference *word, Object &data);

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
    int                 Put(const WordReference& wordRef, int flags);

    //
    // Check for existence
    //
    int                 Exists(const WordReference& wordRef) { return db.Exists(wordRef); }
    int                 Exists(const String& word) { return Exists(WordReference(word)); }

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
    List		*operator [] (const WordReference& wordRef);
    List		*operator [] (const String& word)  { return (*this)[WordReference(word)]; }
    //
    // Return the list of word occurences matching the beginning of wordRef
    //
    List		*Prefix (const WordReference& prefix);
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
    List 		*Walk (const WordReference& word, int action, wordlist_walk_callback_t callback, Object &callback_data);
    int SkipUselessSequentialWalking(const WordKey &wordRefKey,int i0,WordKey &foundKey,String &key,int &cursor_get_flags);

    // trace what's going on in Walk (intended for debuging only)
    void BeginTrace(){traceOn=1;traceRes=new List;}
    List *EndTrace(){traceOn=0;return(traceRes);}
    void CleanupTrace(){if(traceRes){delete traceRes;traceRes=NULL;}traceOn=0;}
    List *GetTraceResult(){return(traceRes);}

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

protected:
    //
    // Retrieve WordReferences from the database. 
    // Backend of WordRefs, operator[], Prefix...
    //
    List		*WordList::Collect (const WordReference& word);

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

    // WordList specific debuging flag
    int                         verbose;
    int traceOn;
    List *traceRes;

    friend ostream &operator << (ostream &o, WordList &list); 
    friend istream &operator >> (istream &o, WordList &list); 


private:

    WordDB	            	db;
};

#endif
