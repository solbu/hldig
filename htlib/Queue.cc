//
// Queue.cc
//
// (c) Copyright 1993, San Diego State University -- College of Sciences
//       (See the COPYRIGHT file for more Copyright information)
//
// Implementation of the Queue class
//
// $Log: Queue.cc,v $
// Revision 1.1.1.1  1997/02/03 17:11:04  turtle
// Initial CVS
//
//
#if RELEASE
static char	RCSid[] = "$Id: Queue.cc,v 1.1.1.1 1997/02/03 17:11:04 turtle Exp $";
#endif

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
