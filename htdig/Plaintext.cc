//
// Plaintext.cc
//
// Plaintext: Parses plaintext files. Not much to do, really.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: Plaintext.cc,v 1.19 2003/06/24 20:05:23 nealr Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "Plaintext.h"
#include "htdig.h"
#include "htString.h"
#include "WordType.h"

#include <ctype.h>
#include "defaults.h"


//*****************************************************************************
// Plaintext::Plaintext()
//
Plaintext::Plaintext()
{
}


//*****************************************************************************
// Plaintext::~Plaintext()
//
Plaintext::~Plaintext()
{
}


//*****************************************************************************
// void Plaintext::parse(Retriever &retriever, URL &)
//
void
Plaintext::parse(Retriever &retriever, URL &)
{
    if (contents == 0 || contents->length() == 0)
	return;

	HtConfiguration* config= HtConfiguration::config();
    unsigned char       *position = (unsigned char *) contents->get();
    static int	minimumWordLength = config->Value("minimum_word_length", 3);
    int		wordIndex = 1;
    int		in_space = 0;
    String	word;
    String	head;

    while (*position)
    {
	word = 0;

	if (HtIsStrictWordChar(*position))
	{
	    //
	    // Start of a word.  Try to find the whole thing
	    //
	    in_space = 0;
	    while (*position && HtIsWordChar(*position))
	    {
		word << *position;
		position++;
	    }

	    if (head.length() < max_head_length)
	    {
		head << word;
	    }

	    if (word.length() >= minimumWordLength)
	    {
		retriever.got_word((char*)word, wordIndex++, 0);
	    }
	}
		
	if (head.length() < max_head_length)
	{
	    //
	    // Characters that are not part of a word
	    //
	    if (*position && isspace(*position))
	    {
		//
		// Reduce all multiple whitespace to a single space
		//
		if (!in_space)
		{
		    head << ' ';
		}
		in_space = 1;
	    }
	    else
	    {
	        head << *position;
		in_space = 0;
	    }
	}
	if (*position)
	    position++;
    }
    retriever.got_head((char*)head);
}


