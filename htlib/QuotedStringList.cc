//
// QuotedStringList.cc
//
// QuotedStringList: Fed with a string it will extract separator delimited
//                   words and store them in a list. The words may be 
//                   delimited by " or ', hence the name.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: QuotedStringList.cc,v 1.7 2004/05/28 13:15:21 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "QuotedStringList.h"


//*****************************************************************************
QuotedStringList::QuotedStringList()
{
}

//*****************************************************************************
int
QuotedStringList::Create(const char *str, const char *sep, int single)
{
    char  quote = 0;
    int    quoted = 0;
    String  word;

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
      Add(new String(word));
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
  Add(new String(word));
    return Count();
}


//*****************************************************************************
int
QuotedStringList::Create(const char *str, char sep, int single)
{
    char  t[2] = "1";

    t[0] = sep;
    return Create(str, t, single);
}


