//
// IntObject.h
//
// $Id: IntObject.h,v 1.1.1.1 1997/02/03 17:11:04 turtle Exp $
//
// $Log: IntObject.h,v $
// Revision 1.1.1.1  1997/02/03 17:11:04  turtle
// Initial CVS
//
//
#ifndef _IntObject_h_
#define _IntObject_h_

#include "Object.h"

class IntObject : public Object
{
public:
	//
	// Construction/Destruction
	//
					IntObject();
					~IntObject();

	int				Value()				{return value;}
	void			Value(int v)		{value = v;}

private:
	int				value;
};

#endif


