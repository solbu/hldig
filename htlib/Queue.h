//
// Queue.h
//
// (c) Copyright 1993, San Diego State University -- College of Sciences
//       (See the COPYRIGHT file for more Copyright information)
//
// This class implements a linked list of objects.  It itself is also an
// object
//
// $Id: Queue.h,v 1.1.1.1 1997/02/03 17:11:04 turtle Exp $
//
// $Log: Queue.h,v $
// Revision 1.1.1.1  1997/02/03 17:11:04  turtle
// Initial CVS
//
//
#ifndef	_Queue_h_
#define	_Queue_h_

#include "Object.h"

class Queue : public Object
{
public:
	//
	// Constructors/Destructor
	//
					Queue();
					~Queue();

	//
	// Queue access
	//
	void			push(Object *obj);
	Object			*peek();
	Object			*pop();
	int				Size()					{return size;}

	//
	// Queue destruction
	//
	void			destroy();

protected:
	//
	// These variables are to keep track of the linked list
	//
	void			*head;
	void			*tail;

	int				size;
};

#endif
