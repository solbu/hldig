//
// $Log: good_strtok.cc,v $
// Revision 1.2  1999/01/20 18:06:21  ghutchis
// Added fixes and speed improvements contributed by Andrew Bishop.
//
// Revision 1.1.1.1  1997/02/03 17:11:04  turtle
// Initial CVS
//
//
#if RELEASE
static char	RCSid[] = "$Id: good_strtok.cc,v 1.2 1999/01/20 18:06:21 ghutchis Exp $";
#endif

#include "lib.h"

//
// Perform the same function as the standard strtok() function except that
// multiple separators are NOT collapsed into one.
//
char *good_strtok(char *str, char term)
{
    static char		*string;

    if (str)
    {
	string = str;
    }

    if (string == NULL || *string == '\0')
	return NULL;

    char *p = string;
    while (*string && *string!=term)
	string++;
    if (*string)
	*string++ = '\0';
    return p;
}
