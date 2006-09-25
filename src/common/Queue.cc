//
// Queue.cc
//
// Queue: This class implements a linked list of objects.  It itself is also an
//        object
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: Queue.cc,v 1.1.2.1 2006/09/25 23:50:31 aarnone Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */
#include "HtDebug.h"

#include "Queue.h"

struct Queuenode
{
	Queuenode	*next;
	Object		*obj;
};

//***************************************************************************
// Queue::Queue()
//
Queue::Queue()
{
	head = tail = 0;
	size = 0;
}


//***************************************************************************
// Queue::~Queue()
//
Queue::~Queue()
{
    HtDebug * debug = HtDebug::Instance();
    debug->outlog(10, "Queue destructor start\n");

	destroy();

    debug->outlog(10, "Queue destructor done\n");
}


//***************************************************************************
// void Queue::destroy()
//
void Queue::destroy()
{
	while (head)
	{
		Object	*obj = pop();
		delete obj;
	}
	size = 0;
	head = tail = 0;
}


//***************************************************************************
// void Queue::push(Object *obj)
//    Push an object onto the Queue.
//
void Queue::push(Object *obj)
{
	Queuenode	*node = new Queuenode;

	node->obj = obj;
	node->next = 0;
	if (tail)
		((Queuenode *) tail)->next = node;
	tail = node;
	if (!head)
		head = tail;
	size++;
}


//***************************************************************************
// Object *Queue::pop()
//    Return the object at the head of the Queue and remove it
//
Object *Queue::pop()
{
	if (size == 0)
		return 0;

	Queuenode	*node = (Queuenode *) head;
	Object		*obj = node->obj;
	head = (void *) node->next;
	delete node;
	size--;

	if (!head)
		tail = 0;
	return obj;
}


//***************************************************************************
// Object *Queue::peek()
//    Return the object at the top of the Queue.
//
Object *Queue::peek()
{
	if (size == 0)
		return 0;

	return ((Queuenode *)head)->obj;
}
