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
// $Id: words.cc,v 1.18 1999/09/24 10:29:04 loic Exp $
//

#include "htmerge.h"
#include "HtPack.h"


//*****************************************************************************
// void mergeWords()
//
void mergeWords()
{
    WordList		 wordDB(config);
    List		*words;
    WordReference	*wordRef = 0;
    String		docIDStr;

    wordDB.Open(config["word_db"], O_RDWR);
    words = wordDB.WordRefs();

    words->Start_Get();
    while ((wordRef = (WordReference *) words->Get_Next()))
      {
	//
	// If the word is from a document we need to discard, we
	// don't want to add it to the database
	//
	docIDStr = 0;
	docIDStr << wordRef->DocID();
	if (discard_list.Exists(docIDStr))
	  {
	    if (verbose)
	      {
		cout << "htmerge: Discarding " << wordRef->Word()
		     << " in doc #" << wordRef->DocID() << "     \n";
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


