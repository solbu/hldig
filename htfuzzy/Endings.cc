//
// Endings.cc
//
// Endings: A fuzzy matching algorithm to match the grammatical endings rules
//          used by the ispell dictionary files.
//           
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Endings.cc,v 1.6 1999/09/10 17:22:24 ghutchis Exp $
//

#include "Endings.h"
#include "htfuzzy.h"
#include "Configuration.h"


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
    HtStripPunctuation(word);

    if (root2word->Get(word, data) == OK)
      {
        //
        // Found the root's permutations
        //
        char    *token = strtok(data.get(), " ");
        while (token)
	  {
            if (mystrcasecmp(token, w) != 0)
	      {
                words.Add(new String(token));
	      }
            token = strtok(0, " ");
	  }
      }
    else
      {
	if (word2root->Get(word, data) == OK)
	  {
	    //
	    // Found the root of the word.  We'll add it to the list already
	    //
	    word = data;
	    words.Add(new String(word));
	  }
	else
	  {
	    //
	    // The root wasn't found.  This could mean that the word
	    // is already the root.
	    //
	  }

	if (root2word->Get(word, data) == OK)
	  {
	    //
	    // Found the root's permutations
	    //
	    char	*token = strtok(data.get(), " ");
	    while (token)
	      {
		if (mystrcasecmp(token, w) != 0)
		  {
		    words.Add(new String(token));
		  }
		token = strtok(0, " ");
	      }
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
    word2root = Database::getDatabaseInstance(DB_BTREE);
    if (word2root->OpenRead(filename) == NOTOK)
	return NOTOK;

    filename = config["endings_root2word_db"];
    root2word = Database::getDatabaseInstance(DB_BTREE);
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


