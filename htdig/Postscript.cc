//
// Postscript.cc
//
// Implementation of Postscript
//
// $Log: Postscript.cc,v $
// Revision 1.1  1997/02/03 17:11:06  turtle
// Initial revision
//
//
#if RELEASE
static char RCSid[] = "$Id: Postscript.cc,v 1.1 1997/02/03 17:11:06 turtle Exp $";
#endif

#include "Postscript.h"
#include "htdig.h"
#include <String.h>
#include <StringList.h>
#include <ctype.h>


//*****************************************************************************
// Postscript::Postscript()
//
Postscript::Postscript()
{
    offset = 0;
    generatorType = 0;
    in_space = 0;
    valid_punctuation = config["valid_punctuation"];
    last_t = "";
    last_y = "";
}


//*****************************************************************************
// Postscript::~Postscript()
//
Postscript::~Postscript()
{
}


//*****************************************************************************
// void Postscript::parse(Retriever &retriever, URL &)
//
void
Postscript::parse(Retriever &retriever, URL &)
{
    if (contents == 0 || contents->length() == 0)
	return;

    return;
	
    char	*position = contents->get();
    char	*start = position;
    String	line;

    generatorType = 0;
	
    while (*position)
    {
	offset = position - start;

	//
	// Skip leading whitespace
	//
	while (*position && (isspace(*position) ||
			     *position == '\r' || *position == '\n'))
	    position++;

	line = 0;
	while (*position && !strchr("\r\n", *position))
	{
	    line << *position;
	    position++;
	}

	parse_line(retriever, line);
    }
}


//*****************************************************************************
// void Postscript::parse_line(Retriever &retriever, String &line)
//
void
Postscript::parse_line(Retriever &retriever, String &line)
{
    StringList		list;
    int				i;
	
    tokenize(line, list);

    if (list.Count() == 0)
	return;			// Empty or comment line
	
    if (generatorType == 0)
    {
	//
	// We don't know the format we are looking for, yet.
	//
	// We will convert the token list to a string of characters
	// which identify the tokens.  'N' for number, 'S' for string,
	// and 'T' for anything else (commands)
	//
	String	form;

	for (i = 0; i < list.Count(); i++)
	{
	    if (*list[i] == '(')
		form << 'S';
	    else if (isdigit(*list[i]))
		form << 'N';
	    else
		form << 'T';
	}

	//
	// Frame uses SNNT.
	// MS-Word uses NNNSNT.
	//
	if (strstr(form, "SNNT"))
	{
	    generatorType = 1;
	}
	else if (strstr(form, "NNNSNT"))
	{
	    generatorType = 2;
	}
	else
	    return;			// Can't recognize this.
    }

    String	str, x, y, t;
    switch (generatorType)
    {
	case 1:	// frame
	{
	    y = 0;
	    for (i = 0; i < list.Count(); i++)
	    {
		if (list[i][0] == '(')
		{
		    str = list[i];
		    x = list[i + 1];
		    y = list[i + 2];
		    t = list[i + 3];
		    break;
		}
	    }
	    if (y.length() == 0)
		break;

	    if (strcmp(y, last_y) != 0 || strcmp(t, last_t) != 0)
	    {
		//
		// This string is on a new line or in a different font.
		// If there was a previous word, we need to add it.
		//
		flush_word(retriever);
	    }
	    parse_string(retriever, str);
	    last_t = t;
	    last_y = y;
	    break;
	}

	case 2: // MS-Word
	    break;
    }
}


//*****************************************************************************
// void Postscript::tokenize(String &line, StringList &list)
//   Convert the input line into a list of tokens.
//
void
Postscript::tokenize(String &line, StringList &list)
{
    char	*position = line.get();

    //
    // Skip comments
    //
    if (*position == '%')
	return;

    //
    // Split the line up into postscript tokens
    //
    String		token;
    int			state = 0;

    //
    // Simple state machine.
    //
    while (*position)
    {
	switch (state)
	{
	    case 0:
		if (token.length())
		{
		    list.Add(token.get());
		    token = 0;
		}

		if (isalnum(*position) || *position == '/')
		{
		    state = 1;
		    token << *position;
		}
		else if (*position == '(')
		{
		    state = 2;
		    token << *position;
		}
		break;

	    case 1:			// Inside token
		if (isalnum(*position) || *position == '.')
		{
		    token << *position;
		}
		else
		{
		    //
		    // Lambda move to initial state.
		    //
		    state = 0;
		    continue;
		}
		break;

	    case 2:			// Inside string
		if (*position == ')')
		{
		    //
		    // End of string.  Lambda move to initial state
		    //
		    state = 0;
		    continue;
		}
		else
		{
		    token << *position;
		}
	}
	position++;
    }
    if (token.length())
	list.Add(token.get());

    //
    // We now have a list of tokens
    //
//	cout << line << endl;
//	for (int i = 0; i < list.Count(); i++)
//	{
//		cout << form("%2d: '%s'\n", i, list[i]);
//	}
}


//*****************************************************************************
// void Postscript::flush_word(Retriever &retriever)
//
void
Postscript::flush_word(Retriever &retriever)
{
    if (word.length() > 2)
    {
	word.lowercase();
	word.remove(valid_punctuation);
	if (word.length() > 2)
	{
	    retriever.got_word(word,
			       int(offset * 1000 / contents->length()),
			       0);
	    cout << "Flushed word '" << word << "'\n";
	}
    }
    word = 0;
}


//*****************************************************************************
// void Postscript::parse_string(Retriever &retriever, String &str)
//
void
Postscript::parse_string(Retriever &retriever, String &str)
{
    char	*position = str.get() + 1;

    while (*position)
    {
	if (isalnum(*position))
	{
	    //
	    // Start or continuation of word
	    //
	    in_space = 0;
	    while (*position && (
				 isalnum(*position) ||
				 strchr(valid_punctuation, *position)))
	    {
		word << *position;
		position++;
	    }
	    if (head.length() < max_head_length)
	    {
		head << word;
	    }

	    if (*position)
	    {
		//
		// We are not at the end of the string, yet.  This means we really
		// have reached the end of this word.
		//
		if (word.length() > 2)
		{
		    word.lowercase();
		    word.remove(valid_punctuation);
		    if (word.length() > 2)
		    {
			retriever.got_word(word,
					   int(offset * 1000 / contents->length()),
					   0);
			cout << "Added word '" << word << "'\n";
		    }
		}
		word = 0;
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
		// Non-whitespace
		//
		head << *position;
		in_space = 0;
	    }
	}
	if (*position)
	    position++;
    }
}

