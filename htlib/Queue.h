//
// Queue.h
//
// Queue: This class implements a linked list of objects.  It itself is also an
//        object
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Queue.h,v 1.3.2.1 2000/05/10 18:23:45 loic Exp $
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
