//
// String.cc
//
// Formatting code for the String class
//
// $Log: String_fmt.cc,v $
// Revision 1.2  1997/03/24 04:33:22  turtle
// Renamed the String.h file to htString.h to help compiling under win32
//
// Revision 1.1.1.1  1997/02/03 17:11:04  turtle
// Initial CVS
//
//
#if RELEASE
static char RCSid[] = "$Id: String_fmt.cc,v 1.2 1997/03/24 04:33:22 turtle Exp $";
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


