//
// WeightWord.cc
//
// WeightWord: Contains the information necessary for a particular search word
//             including the resulting weight (scaling factor) and 
//             whether the word should be hidden (ignored).
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WeightWord.cc,v 1.6 2002/02/01 22:49:35 ghutchis Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "WeightWord.h"

#include <fstream.h>


//***************************************************************************
// WeightWord::WeightWord()
//
WeightWord::WeightWord()
{
    weight = 1;
    records = 0;
    isExact = 0;
    isHidden = 0;
    isIgnore = 0;
}


//***************************************************************************
// WeightWord::WeightWord(WeightWord *ww)
//
WeightWord::WeightWord(WeightWord *ww)
{
    weight = ww->weight;
    records = ww->records;
    isExact = ww->isExact;
    isHidden = ww->isHidden;
    word = ww->word;
    isIgnore = 0;
}


//***************************************************************************
// WeightWord::WeightWord(char *word, double weight)
//
WeightWord::WeightWord(char *word, double weight)
{
    records = 0;
    isExact = 0;
    isHidden = 0;
    isIgnore = 0;
    set(word);
    this->weight = weight;
}


//***************************************************************************
// WeightWord::~WeightWord()
//
WeightWord::~WeightWord()
{
}


//***************************************************************************
// void WeightWord::set(char *word)
//
void WeightWord::set(char *word)
{
    isExact = 0;
    isHidden = 0;
    while (strchr(word, ':'))
    {
	//
	// This word contains modifiers.
	//
	if (mystrncasecmp(word, "exact:", 6) == 0)
	{
	    word += 6;
	    isExact = 1;
	}
	else if (mystrncasecmp(word, "hidden:", 7) == 0)
	{
	    word += 7;
	    isHidden = 1;
	}
	else
	{
	    //
	    // There is a ':' but not a valid attribute.  It must be part
	    // of the word we are searching for.
	    //
	    break;
	}
		
    }
    this->word = word;
    this->word.lowercase();
}

