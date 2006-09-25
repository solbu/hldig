//
// HtWordType.h
//
//  functions for determining valid words/characters
// 
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: HtWordType.cc,v 1.1.2.1 2006/09/25 22:51:14 aarnone Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

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
