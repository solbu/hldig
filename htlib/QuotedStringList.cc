//
// QuotedStringList.cc
//
// QuotedStringList: Fed with a string it will extract separator delimited
//                   words and store them in a list. The words may be 
//                   delimited by " or ', hence the name.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: QuotedStringList.cc,v 1.3 1999/09/24 10:29:03 loic Exp $
//

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
int
QuotedStringList::Create(const char *str, const char *sep, int single)
{
    char	quote = 0;
    String	word;

    while (str && *str)
    {
	if (*str == '\\')
	{
	    word << *++str;
	}
	else if (*str == quote)
	{
	    quote = 0;
	}
	else if (*str == '"' || *str == '\'')
	{
	    quote = *str;
	}
	else if (quote == 0 && strchr(sep, *str))
	{
	    List::Add(new String(word));
	    word = 0;
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
    if (word.length())
	List::Add(new String(word));
    return Count();
}


//*****************************************************************************
int
QuotedStringList::Create(const char *str, char sep, int single)
{
    char	t[2] = "1";

    t[0] = sep;
    return Create(str, t, single);
}


