//
// words.cc
//
// words: Remove words from documents that have been purged by the docs code.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: words.cc,v 1.16 1999/09/11 05:03:53 ghutchis Exp $
//

#include "htmerge.h"
#include "HtPack.h"


//*****************************************************************************
// void mergeWords()
//
void mergeWords()
{
    WordList		 wordDB;
    List		*words;
    WordReference	*wordRef = 0;

    wordDB.Open(config["word_db"]);
    words = wordDB.WordRefs();

    words->Start_Get();
    while ((wordRef = (WordReference *) words->Get_Next()))
      {
	//
	// If the word is from a document we need to discard, we
	// don't want to add it to the database
	//
	if (discard_list.Exists(wordRef->Word))
	  {
	    if (verbose > 1)
	      {
		cout << "htmerge: Discarding " << wordRef->Word
		     << " in doc #" << wordRef->DocumentID << "     \n";
		cout.flush();
	      }
	    continue;
	  }

	//
	// The other problem is if this is somehow a word for a document
	// that no longer exists. In this case, it's deleted by default.
	// We don't do this yet. *FIX*
	//
      }
    
    if (verbose)
	cout << "\n";
    if (stats)
      {
	cout << "htmerge: Total unique word count: " 
	     << wordDB.Words()->Count() << endl;
	cout << "htmerge: Total words: " << words->Count() << endl;
      }

    delete words;
    wordDB.Close();
}


