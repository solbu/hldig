//
// SuffixEntry.h
//
// $Id: SuffixEntry.h,v 1.1.1.1 1997/02/03 17:11:12 turtle Exp $
//
// $Log: SuffixEntry.h,v $
// Revision 1.1.1.1  1997/02/03 17:11:12  turtle
// Initial CVS
//
//
#ifndef _SuffixEntry_h_
#define _SuffixEntry_h_

#include "Object.h"
#include <String.h>


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


