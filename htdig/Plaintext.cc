//
// Plaintext.cc
//
// Implementation of Plaintext
//
// $Log: Plaintext.cc,v $
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
static char RCSid[] = "$Id: Plaintext.cc,v 1.5 1998/11/04 18:53:29 ghutchis Exp $";
#endif

#include "Plaintext.h"
#include "htdig.h"
#include <htString.h>
#include <ctype.h>


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

    unsigned char	*position = contents->get();
    unsigned char	*start = position;
    int		offset = 0;
    int		in_space = 0;
    String	word;
    String	head;

    while (*position)
    {
	offset = position - start;
	word = 0;

	if (isalnum(*position))
	{
	    //
	    // Start of a word.  Try to find the whole thing
	    //
	    in_space = 0;
	    while (*position && (isalnum(*position) || strchr(valid_punctuation, *position)))
	    {
		word << *position;
		position++;
	    }

	    if (head.length() < max_head_length)
	    {
		head << word;
	    }

	    if (word.length() > 2)
	    {
		word.lowercase();
		word.remove(valid_punctuation);
		if (word.length() > 2)
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
	    if (!*position && isspace(*position))
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


