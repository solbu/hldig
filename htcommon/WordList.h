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
// $Id: WordList.h,v 1.9 1999/09/11 05:03:50 ghutchis Exp $
//

#ifndef _WordList_h_
#define _WordList_h_

#include "Dictionary.h"
#include "List.h"
#include "htString.h"
#include "Database.h"
#include "WordRecord.h"
#include "WordReference.h"

class WordList;

//
// Type of the callback argument of WordList::Walk
//
typedef int (*wordlist_walk_callback_t)(WordList *words, WordReference *word, Object &data);

class WordList
{
public:
    //
    // Construction/Destruction
    //
    WordList();
    ~WordList();

    //
    // Set some operating parameters
    //
    void		BadWordFile(char *filename);
    int			DocumentID(int id)		{ int tmp = docID; docID = id; return tmp; }
    int			DocumentID()			{ return docID; }

    //
    // Update/add a word
    //
    void		Word(String word, unsigned int location, 
			     unsigned int anchor_number, 
			     unsigned int flags);

    //
    // Mark a document as already scanned for words or mark it as disappeared
    //
    void		MarkGone();

    //
    // Dump the words to the database
    //
    void		Flush();

    //
    // Check if the given word is valid
    //
    int			IsValid(String word)	{return valid_word(word);}


    //
    // Database access methods
    //

    int                 Open(char *filename);
    int                 Read(char *filename);
    int                 Close();

    int                 Add(WordReference *wordRef);

    // This returns a list of all the WordReference * for this word
    List		*operator [] (String word);
    // This returns a list of all the WordReference * matching the prefix
    List		*Prefix (String prefix);

    int                 Exists(WordReference wordRef);
    int                 Exists(String word);
    int                 Delete(WordReference wordRef);


    //
    // We will need to be able to iterate over the complete database.
    //

    // This returns a list of all the Words, as String *
    List                *Words();
    // This returns a list of all the Words, as WordReference *
    List		*WordRefs();
    // Write an ascii version of the word database in <filename>
    int			Dump(char* filename);


protected:
    //
    // Retrieve WordReferences from the database. 
    // Backend of WordRefs, operator[], Prefix...
    //
    List		*WordList::Collect (String word, int action);
    //
    // Walk and collect data from the word database.
    // Backend of Collect, Dump...
    //
    List 		*Walk (String word, int action, wordlist_walk_callback_t callback, Object &callback_data);

private:

    int			docID;
    String		tempfile;
    List		*words;
    Dictionary		badwords;

    Database            *dbf;
    int                 isopen;
    int                 isread;


    int			valid_word(char *);
};

#endif


