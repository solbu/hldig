//
// Stack.h
//
// (c) Copyright 1993, San Diego State University -- College of Sciences
//       (See the COPYRIGHT file for more Copyright information)
//
// This class implements a linked list of objects.  It itself is also an
// object
//
// $Id: Stack.h,v 1.1.1.1 1997/02/03 17:11:04 turtle Exp $
//
// $Log: Stack.h,v $
// Revision 1.1.1.1  1997/02/03 17:11:04  turtle
// Initial CVS
//
//
#ifndef	_Stack_h_
#define	_Stack_h_

#include "Object.h"

class Stack : public Object
{
public:
	//
	// Constructors/Destructor
	//
					Stack();
					~Stack();

	//
	// Stack access
	//
	void			push(Object *obj);
	Object			*peek();
	Object			*pop();
	int				Size()					{return size;}

	//
	// Stack destruction
	//
	void			destroy();

protected:
	//
	// These variables are to keep track of the linked list
	//
	void			*sp;

	int				size;
};

#endif
