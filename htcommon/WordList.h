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
// $Id: WordList.h,v 1.10 1999/09/24 10:28:56 loic Exp $
//

#ifndef _WordList_h_
#define _WordList_h_

#include <fcntl.h>

#include "Dictionary.h"
#include "List.h"
#include "htString.h"
#include "Database.h"
#include "WordRecord.h"
#include "WordReference.h"
#include "Configuration.h"

class WordList;

//
// Type of the callback argument of WordList::Walk
//
typedef int (*wordlist_walk_callback_t)(WordList *words, const WordReference *word, Object &data);

class WordList
{
public:
    //
    // Construction/Destruction
    //
    WordList(const Configuration& config_arg);
    ~WordList();
    
    //
    // Update/add a word, perform sanity checking and
    // fill information.
    //
    void		Replace(const WordReference& wordRef);
    //
    // Update/add a word (backend of Replace)
    //
    int                 Add(const WordReference& wordRef);

    //
    // Delete permanently
    //
    int                 Delete(const WordReference& wordRef);

    //
    // Mark a document as already scanned for words or mark it as disappeared
    //
    void		MarkGone();

    //
    // Flush the words stored in the object to the database
    //
    void		Flush();

    //
    // Handling the list of bad words 
    //
    void		BadWordFile(const String& filename);
    //
    // Check if the given word is valid
    //
    int			IsValid(String word)	{ return IsValid(word.get()); }
    int			IsValid(const char* word);


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
    // Check for existence
    //
    int                 Exists(const WordReference& wordRef);
    int                 Exists(const String& word) { return Exists(WordReference(word)); }


    //
    // Iterate over the complete database.
    //

    // This returns a list of all the Words, as String *
    List                *Words();
    // This returns a list of all the Words, as WordReference *
    List		*WordRefs();
    // Write an ascii version of the word database in <filename>
    int			Dump(const String& filename);


protected:
    //
    // Retrieve WordReferences from the database. 
    // Backend of WordRefs, operator[], Prefix...
    //
    List		*WordList::Collect (const WordReference& word, int action);
    //
    // Walk and collect data from the word database.
    // Backend of Collect, Dump...
    //
    List 		*Walk (const WordReference& word, int action, wordlist_walk_callback_t callback, Object &callback_data);

private:

    List			*words;
    Dictionary			badwords;

    Database            	*dbf;
    int                 	isopen;
    int                 	isread;

    const Configuration&	config;
};

#endif


