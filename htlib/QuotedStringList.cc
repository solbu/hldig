//
// QuotedStringList.cc
//
// Implementation of QuotedStringList
//
// $Log: QuotedStringList.cc,v $
// Revision 1.1.1.1.2.2  2000/02/15 21:43:47  grdetil
// htlib/QuotedStringList.cc (Create): fix PR#743, where quoted string
// lists didn't allow embedded quotes of opposite sort in strings
// (e.g. "'" or '"'), and fix to avoid overrunning end of string
// if it ends with backslash.
//
// Revision 1.1.1.1.2.1  1999/12/09 00:27:36  ghutchis
// (Create): Make sure an empty token isn't ignored.
//
// Revision 1.1.1.1  1997/02/03 17:11:05  turtle
// Initial CVS
//
//
#if RELEASE
static char RCSid[] = "$Id: QuotedStringList.cc,v 1.1.1.1.2.2 2000/02/15 21:43:47 grdetil Exp $";
#endif

#include "QuotedStringList.h"


//*****************************************************************************
QuotedStringList::QuotedStringList()
{
}


//*****************************************************************************
QuotedStringList::~QuotedStringList()
{
}


//*****************************************************************************
QuotedStringList::QuotedStringList(char *str, char sep, int single)
{
    Create(str, sep, single);
}


//*****************************************************************************
QuotedStringList::QuotedStringList(String &str, char sep, int single)
{
    Create(str, sep, single);
}


//*****************************************************************************
QuotedStringList::QuotedStringList(char *str, char *sep, int single)
{
    Create(str, sep, single);
}


//*****************************************************************************
QuotedStringList::QuotedStringList(String &str, char *sep, int single)
{
    Create(str, sep, single);
}


//*****************************************************************************
int
QuotedStringList::Create(String &str, char sep, int single)
{
    return Create(str.get(), sep, single);
}


//*****************************************************************************
int
QuotedStringList::Create(String &str, char *sep, int single)
{
    return Create(str.get(), sep, single);
}


//*****************************************************************************
int
QuotedStringList::Create(char *str, char *sep, int single)
{
    char	quote = 0;
    int	quoted = 0;
    String	word;

    while (str && *str)
    {
	if (*str == '\\')
	{
	    if (!str[1])
		break;
	    word << *++str;
	}
	else if (*str == quote)
	{
	    quote = 0;
	}
	else if (!quote && (*str == '"' || *str == '\''))
	{
	    quote = *str;
	    quoted++;
	}
	else if (quote == 0 && strchr(sep, *str))
	{
	    List::Add(new String(word));
	    word = 0;
	    quoted = 0;
	    if (!single)
	    {
		while (strchr(sep, *str))
		    str++;
		str--;
	    }
	}
	else
	    word << *str;
	str++;
    }

    //
    // Add the last word to the list
    //
    if (word.length() || quoted)
	List::Add(new String(word));
    return Count();
}


//*****************************************************************************
int
QuotedStringList::Create(char *str, char sep, int single)
{
    char	t[2] = "1";

    t[0] = sep;
    return Create(str, t, single);
}


