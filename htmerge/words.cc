//
// words.cc
//
// Implementation of htmerge
//
//
#if RELEASE
static char RCSid[] = "$Id: words.cc,v 1.14 1999/07/19 02:04:32 ghutchis Exp $";
#endif

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


