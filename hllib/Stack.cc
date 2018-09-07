//
// Stack.cc
//
// Stack: This class implements a linked list of objects.  It itself is also an
//        object
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: Stack.cc,v 1.5 2004/05/28 13:15:21 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "hlconfig.h"
#endif /* HAVE_CONFIG_H */

#include "Stack.h"

struct stacknode
{
  stacknode *next;
  Object *obj;
};

//***************************************************************************
// Stack::Stack()
//
Stack::Stack ()
{
  sp = 0;
  size = 0;
}


//***************************************************************************
// Stack::~Stack()
//
Stack::~Stack ()
{
  while (sp)
  {
    Object *obj = pop ();
    delete obj;
  }
}


//***************************************************************************
// void Stack::destroy()
//
void
Stack::destroy ()
{
  while (sp)
  {
    Object *obj = pop ();
    delete obj;
  }
}


//***************************************************************************
// void Stack::push(Object *obj)
// PURPOSE:
//    Push an object onto the stack.
//
void
Stack::push (Object * obj)
{
  stacknode *node = new stacknode;

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
Object *
Stack::pop ()
{
  if (size == 0)
    return 0;

  stacknode *node = (stacknode *) sp;
  Object *obj = node->obj;
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
Object *
Stack::peek ()
{
  if (size == 0)
    return 0;

  return ((stacknode *) sp)->obj;
}
