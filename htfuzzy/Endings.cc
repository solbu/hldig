//
// Endings.cc
//
// Endings: A fuzzy matching algorithm to match the grammatical endings rules
//          used by the ispell dictionary files.
//           
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: Endings.cc,v 1.11 2003/06/24 20:06:19 nealr Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <fcntl.h>

#include "StringList.h"
#include "Endings.h"
#include "htfuzzy.h"
#include "HtConfiguration.h"


//*****************************************************************************
// Endings::Endings()
//
Endings::Endings(const HtConfiguration& config_arg) :
  Fuzzy(config_arg)
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
    HtStripPunctuation(word);
    String	saveword = word.get();

    //
    // Look for word's root(s).  Some words may have more than one root,
    // so handle them all.  Whether or not a word has a root, it's assumed
    // to be root in itself.
    //
    if (word2root->Get(word, data) == OK)
	word << ' ' << data;
 
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
	    word << ' ' << data;

	//
	// Iterate through the root's permutations
	//
	char	*token = strtok(word.get(), " ");
	while (token)
	{
	    if (mystrcasecmp(token, saveword.get()) != 0)
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
			break;
		}
		if (obj == 0)
		    words.Add(new String(token));
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
// int Endings::openIndex()
//   Dummy method.  Just makde sure we don't actually create a database.
//
int
Endings::openIndex()
{
    String	filename = config["endings_word2root_db"];
    word2root = Database::getDatabaseInstance(DB_BTREE);
    if (word2root->OpenRead((char*)filename) == NOTOK)
	return NOTOK;

    filename = config["endings_root2word_db"];
    root2word = Database::getDatabaseInstance(DB_BTREE);
    if (root2word->OpenRead((char*)filename) == NOTOK)
	return NOTOK;

    return OK;
}


//*****************************************************************************
// int Endings::writeDB()
//   Dummy method.  Just making sure we don't actually write anything.
//
int
Endings::writeDB()
{
    return OK;
}


