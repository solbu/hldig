//
// WeightWord.cc
//
// Implementation of WeightWord
//
// $Log: WeightWord.cc,v $
// Revision 1.1.1.1  1997/02/03 17:11:05  turtle
// Initial CVS
//
// Revision 1.1  1996/01/03 19:02:00  turtle
// Before rewrite
//
//
#if RELEASE
static char RCSid[] = "$Id: WeightWord.cc,v 1.1.1.1 1997/02/03 17:11:05 turtle Exp $";
#endif

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
	if (strncasecmp(word, "exact:", 6) == 0)
	{
	    word += 6;
	    isExact = 1;
	}
	else if (strncasecmp(word, "hidden:", 7) == 0)
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
}

