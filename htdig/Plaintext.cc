//
// Plaintext.cc
//
// Implementation of Plaintext
//
// $Log: Plaintext.cc,v $
// Revision 1.9  1999/03/23 18:21:54  grdetil
// Use minimum_word_length instead of hardcoded constant 2.
//
// Revision 1.8  1999/03/16 02:04:27  hp
// * New attribute extra_word_characters
// * Remove all code "everywhere". reading and caching
//   valid_punctuation, replace with new ctype-like functions
//   HtIsWordChar(), HtIsStrictWordChar() and HtStripPunctuation()
//
// Revision 1.7  1999/01/08 19:39:18  bergolth
// bugfixes in htdig/Plaintext.cc and htlib/URL.cc
//
// Revision 1.6  1998/12/04 04:13:08  ghutchis
//
// Removed compiler warnings.
//
// Revision 1.5  1998/11/04 18:53:29  ghutchis
//
// Added patch from Vadim Chekan to change char to unsigned char to fix reading
// Cyrillic plaintext files.
//
// Revision 1.4  1997/04/20 15:23:40  turtle
// Fixed bug
//
// Revision 1.3  1997/03/27 00:06:05  turtle
// Applied patch supplied by Peter Enderborg <pme@ufh.se> to fix a problem with
// a pointer running off the end of a string.
//
// Revision 1.2  1997/03/24 04:33:17  turtle
// Renamed the String.h file to htString.h to help compiling under win32
//
// Revision 1.1.1.1  1997/02/03 17:11:06  turtle
// Initial CVS
//
//
#if RELEASE
static char RCSid[] = "$Id: Plaintext.cc,v 1.9 1999/03/23 18:21:54 grdetil Exp $";
#endif

#include "Plaintext.h"
#include "htdig.h"
#include <htString.h>
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
		word.lowercase();
		HtStripPunctuation(word);
		if (word.length() >= minimumWordLength)
		{
		    retriever.got_word(word,
				       int(offset * 1000 / contents->length()),
				       0);
		}
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


