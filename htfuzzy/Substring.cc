//
// Substring.cc
//
// Implementation of Substring
//
//
#if RELEASE
static char RCSid[] = "$Id: Substring.cc,v 1.4 1999/03/03 04:46:57 ghutchis Exp $";
#endif

#include "Substring.h"
#include "htString.h"
#include "List.h"
#include "StringMatch.h"
#include "Configuration.h"

extern Configuration	config;


//*****************************************************************************
// Substring::Substring()
//
Substring::Substring()
{
}


//*****************************************************************************
// Substring::~Substring()
//
Substring::~Substring()
{
}


//*****************************************************************************
// A very simplistic and inefficient substring search.  For every word
// that is looked for we do a complete linear search through the word
// database.
// Maybe a better method of doing this would be to mmap a list of words
// to memory and then run the StringMatch on it.  It would still be a
// linear search, but with much less overhead.
//
void
Substring::getWords(char *w, List &words)
{
    StringMatch	match;

    match.Pattern(w);

    Database	*dbf = Database::getDatabaseInstance(DB_BTREE);
    dbf->OpenRead(config["word_db"]);

    int		wordCount = 0;
    int		maximumWords = config.Value("substring_max_words", 25);
    
    dbf->Start_Get();
    while (wordCount < maximumWords && (w = dbf->Get_Next()))
    {
	if (match.FindFirst(w) >= 0)
	{
	    words.Add(new String(w));
	    wordCount++;
	}
    }
    dbf->Close();
    delete dbf;
}


//*****************************************************************************
int
Substring::openIndex(Configuration &)
{
  return 0;
}


//*****************************************************************************
void
Substring::generateKey(char *, String &)
{
}


//*****************************************************************************
void
Substring::addWord(char *)
{
}




