//
// Plaintext.cc
//
// Implementation of Plaintext
//
//
#if RELEASE
static char RCSid[] = "$Id: Plaintext.cc,v 1.7.2.4 1999/09/01 21:02:47 grdetil Exp $";
#endif

#include "Plaintext.h"
#include "htdig.h"
#include "htString.h"
#include <ctype.h>
#include "HtWordType.h"


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

    unsigned char       *position = (unsigned char *) contents->get();
    unsigned char	*start = position;
    static int	minimumWordLength = config.Value("minimum_word_length", 3);
    int		offset = 0;
    int		in_space = 0;
    String	word;
    String	head;

    while (*position)
    {
	offset = position - start;
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
		retriever.got_word(word,
				   int(offset * 1000 / contents->length()),
				   0);
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
		//
		// Non whitespace
		//
		switch (*position)
		{
		    case '<':
			head << "&lt;";
			break;
		    case '>':
			head << "&gt;";
			break;
		    case '&':
			head << "&amp;";
			break;
		    case '\0':
			break;
		    default:
			head << *position;
			break;
		}
		in_space = 0;
	    }
	}
	if (*position)
	    position++;
    }
    retriever.got_head(head);
}


