//
// Stack.cc
//
// (c) Copyright 1993, San Diego State University -- College of Sciences
//       (See the COPYRIGHT file for more Copyright information)
//
// Implementation of the Stack class
//
// $Log: Stack.cc,v $
// Revision 1.1.1.1  1997/02/03 17:11:04  turtle
// Initial CVS
//
//
#if RELEASE
static char	RCSid[] = "$Id: Stack.cc,v 1.1.1.1 1997/02/03 17:11:04 turtle Exp $";
#endif

#include "Stack.h"

struct stacknode
{
	stacknode	*next;
	Object		*obj;
};

//***************************************************************************
// Stack::Stack()
//
Stack::Stack()
{
	sp = 0;
	size = 0;
}


//***************************************************************************
// Stack::~Stack()
//
Stack::~Stack()
{
	while (sp)
	{
		Object	*obj = pop();
		delete obj;
	}
}


//***************************************************************************
// void Stack::destroy()
//
void Stack::destroy()
{
	while (sp)
	{
		Object	*obj = pop();
		delete obj;
	}
}


//***************************************************************************
// void Stack::push(Object *obj)
// PURPOSE:
//    Push an object onto the stack.
//
void Stack::push(Object *obj)
{
	stacknode	*node = new stacknode;

	node->obj = obj;
	node->next = (stacknode *) sp;
	sp = node;
	size++;
}


//***************************************************************************
// Object *Stack::pop()
// PURPOSE:
//    Return the object at the top of the stack and remove it from the stack.
//
Object *Stack::pop()
{
	if (size == 0)
		return 0;

	stacknode	*node = (stacknode *) sp;
	Object		*obj = node->obj;
	sp = (void *) node->next;
	delete node;
	size--;

	return obj;
}


//***************************************************************************
// Object *Stack::peek()
// PURPOSE:
//    Return the object at the top of the stack.
//
Object *Stack::peek()
{
	if (size == 0)
		return 0;

	return ((stacknode *)sp)->obj;
}
