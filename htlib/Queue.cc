//
// Queue.cc
//
// Queue: This class implements a linked list of objects.  It itself is also an
//        object
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: Queue.cc,v 1.5 2003/06/24 20:05:45 nealr Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

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
	destroy();
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
