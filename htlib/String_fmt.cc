//
// String_fmt.cc
//
// String_fmt: Formatting functions for the String class. Those functions
//             are also used in other files, they are not purely internal
//             to the String class.
//
//
#if RELEASE
static char RCSid[] = "$Id: String_fmt.cc,v 1.4 1999/09/10 11:45:30 loic Exp $";
#endif

#include "htString.h"
#include <stdarg.h>
#include <stdio.h>

static char	buf[10000];

//*****************************************************************************
// char *form(char *fmt, ...)
//
char *form(char *fmt, ...)
{
	va_list	args;
	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);
	return buf;
}


//*****************************************************************************
// char *vform(char *fmt, va_list args)
//
char *vform(char *fmt, va_list args)
{
	vsprintf(buf, fmt, args);
	return buf;
}


