//
// SuffixEntry.h
//
// $Id: SuffixEntry.h,v 1.2 1997/03/24 04:33:18 turtle Exp $
//
// $Log: SuffixEntry.h,v $
// Revision 1.2  1997/03/24 04:33:18  turtle
// Renamed the String.h file to htString.h to help compiling under win32
//
// Revision 1.1.1.1  1997/02/03 17:11:12  turtle
// Initial CVS
//
//
#ifndef _SuffixEntry_h_
#define _SuffixEntry_h_

#include "Object.h"
#include <htString.h>


class SuffixEntry : public Object
{
public:
	//
	// Construction/Destruction
	//
					SuffixEntry(char *);
					~SuffixEntry();

	String			expression;
	String			rule;

	void			parse(char *str);
	
private:
};

#endif


