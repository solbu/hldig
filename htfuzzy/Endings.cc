//
// Endings.cc
//
// Implementation of Endings
//
// $Log: Endings.cc,v $
// Revision 1.2.2.2  2001/06/15 21:38:39  grdetil
// * htfuzzy/Endings.cc (getWords): Undid change introduced in 3.1.3,
//   in part. It now gets permutations of word whether or not it has
//   a root, but it also gets permutations of one or more roots that
//   the word has, based on a suggestion by Alexander Lebedev.
// * htfuzzy/EndingsDB.cc (createRoot): Fixed to handle words that have
//   more than one root.
// * installdir/english.0: Removed P flag from wit, like and high, so
//   they're not treated as roots of witness, likeness and highness, which
//   are already in the dictionary.
//
// Revision 1.2.2.1  1999/09/01 19:50:59  grdetil
// Suffix-handling improvement (PR#560), to prevent inappropriate suffix
// stripping in endings fuzzy matches.
//
// Revision 1.2  1998/10/12 02:04:00  ghutchis
//
// Updated Makefiles and configure variables.
//
// Revision 1.1.1.1  1997/02/03 17:11:12  turtle
// Initial CVS
//
//
#if RELEASE
static char RCSid[] = "$Id: Endings.cc,v 1.2.2.2 2001/06/15 21:38:39 grdetil Exp $";
#endif

#include "StringList.h"
#include "Endings.h"
#include "htfuzzy.h"
#include <Configuration.h>


//*****************************************************************************
// Endings::Endings()
//
Endings::Endings()
{
    root2word = 0;
    word2root = 0;
    name = "endings";
}


//*****************************************************************************
// Endings::~Endings()
//
Endings::~Endings()
{
    if (root2word)
    {
	root2word->Close();
	delete root2word;
	root2word = 0;
    }
	
    if (word2root)
    {
	word2root->Close();
	delete word2root;
	word2root = 0;
    }
}


//*****************************************************************************
// void Endings::getWords(char *word, String &words)
//   Return a list of words with some common English word endings.
//
void
Endings::getWords(char *w, List &words)
{
    if (!word2root || !root2word)
	return;

    String	data;

    String	word = w;
    word.lowercase();
	
    //
    // Look for word's root(s).  Some words may have more than one root,
    // so handle them all.  Whether or not a word has a root, it's assumed
    // to be root in itself.
    //
    if (word2root->Get(word, data) == OK)
    {
	word << ' ' << data;
    }
    StringList	roots(word, " ");
    Object	*root;
    roots.Start_Get();
    while ((root = roots.Get_Next()) != 0)
    {
	//
	// Found a root.  Look for new words that have this root.
	//
	word = ((String *)root)->get();
	if (root2word->Get(word, data) == OK)
	{
	    word << ' ' << data;
	}
	//
	// Iterate through the root's permutations
	//
	char	*token = strtok(word.get(), " ");
	while (token)
	{
	    if (mystrcasecmp(token, w) != 0)
	    {
		//
		// This permutation isn't the original word, so we add it
		// to the list if it's not already there.
		//
		Object	*obj;
		words.Start_Get();
		while((obj = words.Get_Next()) != 0)
		{
		    if (mystrcasecmp(token, ((String *)obj)->get()) == 0)
		    {
			break;
		    }
		}
		if (obj == 0)
		{
		    words.Add(new String(token));
		}
	    }
	    token = strtok(0, " ");
	}
    }
}


//*****************************************************************************
// void Endings::generateKey(char *word, String &key)
//   Not needed.
void
Endings::generateKey(char *, String &)
{
}


//*****************************************************************************
// void Endings::addWord(char *word)
//   Not needed.
void
Endings::addWord(char *)
{
}


//*****************************************************************************
// int Endings::openIndex(Configuration &)
//   Dummy method.  Just makde sure we don't actually create a database.
//
int
Endings::openIndex(Configuration &)
{
    String	filename;
    filename = config["endings_word2root_db"];
    word2root = Database::getDatabaseInstance();
    if (word2root->OpenRead(filename) == NOTOK)
	return NOTOK;

    filename = config["endings_root2word_db"];
    root2word = Database::getDatabaseInstance();
    if (root2word->OpenRead(filename) == NOTOK)
	return NOTOK;

    return OK;
}


//*****************************************************************************
// int Endings::writeDB(Configuration &)
//   Dummy method.  Just making sure we don't actually write anything.
//
int
Endings::writeDB(Configuration &)
{
    return OK;
}


