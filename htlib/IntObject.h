//
// IntObject.h
//
// $Id: IntObject.h,v 1.2 1999/03/12 00:46:58 hp Exp $
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


