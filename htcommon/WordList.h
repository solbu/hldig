//
// WordList.h
//
//: Interface to the word database. Previously, this wrote to a temporary text
//  file. Now it writes directly to the word database.
//  NOTE: Some code previously attempted to directly read from the word db.
//  This will no longer work, so it's preferred to use the access methods here
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordList.h,v 1.3 1999/07/19 01:50:25 ghutchis Exp $
//
//
#ifndef _WordList_h_
#define _WordList_h_

#include "Dictionary.h"
#include "htString.h"
#include "Database.h"
#include "WordRecord.h"
#include "WordReference.h"

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
    void		DocumentID(int id)		{docID = id;}

    //
    // Update/add a word
    //
    void		Word(String word, unsigned int location, 
			     unsigned int anchor_number, 
			     unsigned long int flags);

    //
    // Mark a document as already scanned for words or mark it as disappeared
    //
    void		MarkScanned();
    void		MarkGone();
    void		MarkModified();

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

    int                 Exists(WordReference wordRef);
    int                 Delete(WordReference wordRef);


    //
    // We will need to be able to iterate over the complete database.
    //

    // This returns a list of all the Words, as String *
    List                *Words();
    // This returns a list of all the Words, as WordReference *
    List		*WordRefs();

private:
    int			docID;
    String		tempfile;
    Dictionary		*words;
    Dictionary		badwords;

    Database            *dbf;
    int                 isopen;
    int                 isread;


    int			valid_word(char *);
};

#endif


