//
// $Log: good_strtok.cc,v $
// Revision 1.1.1.1  1997/02/03 17:11:04  turtle
// Initial CVS
//
//
#if RELEASE
static char	RCSid[] = "$Id: good_strtok.cc,v 1.1.1.1 1997/02/03 17:11:04 turtle Exp $";
#endif

#include "lib.h"

//
// Perform the same function as the standard strtok() function except that
// multiple separators are NOT collapsed into one.
//
char *good_strtok(char *str, char *term)
{
    static char		*string;
    static char		valids[256];

    if (!term)
    {
	//
	// This is for the case where there is only one argument.
	// In this case we will assume that we have to use the
	// same string as in the previous call to this function
	//
	term = str;
	str = 0;
    }

    if (str)
    {
	string = str;
    }

    if (string == NULL || *string == '\0')
	return NULL;

    memset(valids, 1, sizeof(valids));
    while (*term)
    {
	valids[(unsigned char) *term++] = 0;
    }
    valids[0] = 0;

    char *p = string;
    while (valids[(unsigned char) *string])
	string++;
    if (*string)
	*string++ = '\0';
    return p;
}
