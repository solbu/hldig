//
// HtWordType.h
//
//  functions for determining valid words/characters
// 
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtWordType.cc,v 1.7.2.2 2000/02/22 23:15:49 grdetil Exp $
//

#include "HtWordType.h"
#include "WordType.h"

int HtIsWordChar(char c)          { return WordType::Instance()->IsChar(c); }
int HtIsStrictWordChar(char c)	  { return WordType::Instance()->IsStrictChar(c); }
int HtWordNormalize(String &w)	  { return WordType::Instance()->Normalize(w); }
int HtStripPunctuation(String &w) { return WordType::Instance()->StripPunctuation(w); }


//  much like strtok(), and destructive of the source string like strtok(),
//  but does word separation by our rules.
char *
HtWordToken(char *str)
{
    unsigned char		*text = (unsigned char *)str;
    char			*ret = 0;
    static unsigned char	*prev = 0;

    if (!text)
	text = prev;
    while (text && *text && !HtIsStrictWordChar(*text))
	text++;
    if (text && *text)
    {
	ret = (char *)text;
	while (*text && HtIsWordChar(*text))
	    text++;
	if (*text)
	    *text++ = '\0';
    }
    prev = text;
    return ret;
}
