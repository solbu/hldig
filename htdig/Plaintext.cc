//
// Plaintext.cc
//
// Implementation of Plaintext
//
// $Log: Plaintext.cc,v $
// Revision 1.2  1997/03/24 04:33:17  turtle
// Renamed the String.h file to htString.h to help compiling under win32
//
// Revision 1.1.1.1  1997/02/03 17:11:06  turtle
// Initial CVS
//
//
#if RELEASE
static char RCSid[] = "$Id: Plaintext.cc,v 1.2 1997/03/24 04:33:17 turtle Exp $";
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

    char	*position = contents->get();
    char	*start = position;
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
	    while (isalnum(*position) || strchr(valid_punctuation, *position))
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
	    if (isspace(*position))
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
		    default:
			head << *position;
			break;
		}
		in_space = 0;
	    }
	}
	position++;
    }
    retriever.got_head(head);
}


