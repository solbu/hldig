//
// String_fmt.cc
//
// String_fmt: Formatting functions for the String class. Those functions
//             are also used in other files, they are not purely internal
//             to the String class.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: String_fmt.cc,v 1.5.2.3 2000/05/10 18:23:45 loic Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "htString.h"

#include <stdarg.h>
#include <stdio.h>

static char	buf[10000];

//*****************************************************************************
// char *form(char *fmt, ...)
//
char *form(const char *fmt, ...)
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
char *vform(const char *fmt, va_list args)
{
	vsprintf(buf, fmt, args);
	return buf;
}


