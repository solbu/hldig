//
// $Id: good_strtok.h,v 1.2 1999/01/20 18:06:21 ghutchis Exp $
//
// $Log: good_strtok.h,v $
// Revision 1.2  1999/01/20 18:06:21  ghutchis
// Added fixes and speed improvements contributed by Andrew Bishop.
//
// Revision 1.1.1.1  1997/02/03 17:11:04  turtle
// Initial CVS
//
//
#ifndef	_good_strtok_h_
#define	_good_strtok_h_

//
// The good_strtok() function is very similar to the standard strtok()
// library function, except that good_strtok() will only skip over 1 separator
// if it finds one.  This is needed when parsing strings with empty fields.
//
char	*good_strtok(char *, char);

#endif

