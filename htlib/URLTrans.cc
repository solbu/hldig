//
// URL.cc
//
// Implementation of URL
//
// $Log: URLTrans.cc,v $
// Revision 1.2.2.1  1999/09/01 19:48:38  grdetil
// Fix encoding so all non-ascii characters get hex-encoded. (PR#339)
//
// Revision 1.2  1997/03/24 04:33:22  turtle
// Renamed the String.h file to htString.h to help compiling under win32
//
// Revision 1.1.1.1  1997/02/03 17:11:04  turtle
// Initial CVS
//
//
#if RELEASE
static char RCSid[] = "$Id: URLTrans.cc,v 1.2.2.1 1999/09/01 19:48:38 grdetil Exp $";
#endif

#include "URL.h"
#include "htString.h"
#include "lib.h"

#include <ctype.h>


//*****************************************************************************
// void decodeURL(String &str)
//   Convert the given URL string to a normal string.  This means that
//   all escaped characters are converted to their normal values.  The
//   escape character is '%' and is followed by 2 hex digits
//   representing the octet.
//
void decodeURL(String &str)
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
}


//*****************************************************************************
// void encodeURL(String &str, char *valid)
//   Convert a normal string to a URL 'safe' string.  This means that
//   all characters not explicitly mentioned in the URL BNF will be
//   escaped.  The escape character is '%' and is followed by 2 hex
//   digits representing the octet.
//
void encodeURL(String &str, char *valid)
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
}


