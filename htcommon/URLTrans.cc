//
// URLTrans.cc
//
// URLTrans: Helper functions for the implementation of the URL class.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: URLTrans.cc,v 1.3 2003/01/11 02:33:28 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "URL.h"
#include "htString.h"
#include "lib.h"

#include <ctype.h>


//*****************************************************************************
// String &decodeURL(String &str)
//   Convert the given URL string to a normal string.  This means that
//   all escaped characters are converted to their normal values.  The
//   escape character is '%' and is followed by 2 hex digits
//   representing the octet.
//
String &decodeURL(String &str)
{
    String	temp;
    char	*p;

    for (p = str; p && *p; p++)
    {
	if (*p == '%')
	{
	    //
	    // 2 hex digits follow...
	    //
	    int		value = 0;
	    for (int i = 0; p[1] && i < 2; i++)
	    {
		p++;
		value <<= 4;
		if (isdigit(*p))
		    value += *p - '0';
		else
		    value += toupper(*p) - 'A' + 10;
	    }
	    temp << char(value);
	}
	else
	    temp << *p;
    }
    str = temp;
    return (str);
}


//*****************************************************************************
// String &encodeURL(String &str, char *valid)
//   Convert a normal string to a URL 'safe' string.  This means that
//   all characters not explicitly mentioned in the URL BNF will be
//   escaped.  The escape character is '%' and is followed by 2 hex
//   digits representing the octet.
//
String &encodeURL(String &str, char *valid)
{
    String	temp;
    static char	*digits = "0123456789ABCDEF";
    char	*p;

    for (p = str; p && *p; p++)
    {
	if (isascii(*p) && (isdigit(*p) || isalpha(*p) || strchr(valid, *p)))
	    temp << *p;
	else
	{
	    temp << '%';
	    temp << digits[(*p >> 4) & 0x0f];
	    temp << digits[*p & 0x0f];
	}
    }
    str = temp;
    return (str);
}


