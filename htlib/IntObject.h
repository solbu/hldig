//
// IntObject.h
//
// IntObject: int variable encapsulated in Object derived class
//
// $Id: IntObject.h,v 1.3 1999/09/08 14:42:29 loic Exp $
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
					IntObject(int v) { value = v; }
					~IntObject();

	int				Value()				{return value;}
	void			Value(int v)		{value = v;}

private:
	int				value;
};

#endif


