//
// List.cc
//
// List: A List class which holds objects of type Object.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: List.cc,v 1.9 2004/05/28 13:15:21 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "List.h"

class listnode
{
public:

  listnode    *next;
  Object    *object;
};


//*********************************************************************
// List::List()
//   Constructor
//
List::List()
{
    head = tail = 0;
    number = 0;
}


//*********************************************************************
// List::~List()
//   Destructor
//
List::~List()
{
    Destroy();
}


//*********************************************************************
// void List::Release()
//   Release all the objects from our list.
//
void List::Release()
{
    listnode    *node;
    while (head)
    {
  node = head;
  head = head->next;
  delete node;
    }
    head = tail = 0;
    number = 0;
    cursor.Clear();
}


//*********************************************************************
// void List::Destroy()
//   Delete all the objects from our list.
//
void List::Destroy()
{
    listnode    *node;
    while (head)
    {
  node = head;
  head = head->next;
  delete node->object;
  delete node;
    }
    head = tail = 0;
    number = 0;
    cursor.Clear();
}


//*********************************************************************
// void List::Add(Object *object)
//   Add an object to the list.
//
void List::Add(Object *object)
{
    listnode    *node = new listnode;
    node->next = 0;
    node->object = object;
    if (tail)
    {
  tail->next = node;
  tail = node;
    }
    else
    {
  head = tail = node;
    }

    number++;
}


//*********************************************************************
// void List::Insert(Object *object, int position)
//   Add an object to the list.
//
void List::Insert(Object *object, int position)
{
    listnode    *node = new listnode;
    node->next = 0;
    node->object = object;

    listnode    *ln = head;
    listnode    *prev = 0;

    for (int i = 0; i < position && ln; i++, ln = ln->next)
  prev = ln;
    if (!ln)
    {
  if (tail)
      tail->next = node;
  tail = node;

  //
  // The list is empty.  This is a simple case, then.
  //
  if (!head)
      head = node;
    }
    else
    {
  if (ln == head)
  {
      node->next = head;
      head = node;
  }
  else
  {
      node->next = ln;
      prev->next = node;
  }
    }

    cursor.current_index = -1;
    number++;
}


//*********************************************************************
// void List::Assign(Object *object, int position)
//   Assign a new value to an index.
//
void List::Assign(Object *object, int position)
{
    //
    // First make sure that there is something there!
    //
    while (number < position + 1)
    {
  Add(0);
    }

    //
    // Now find the listnode to put the new object in
    //
    listnode  *temp = head;

    for (int i = 0; temp && i < position; i++)
    {
  temp = temp->next;
    }

    cursor.current_index = -1;
    delete temp->object;
    temp->object = object;
}


//*********************************************************************
// int List::Remove(Object *object)
//   Remove an object from the list.
//
int List::Remove(Object *object)
{
    listnode    *node = head;
    listnode    *prev = 0;
    while (node)
    {
  if (node->object == object)
  {
      //
      // Found it!
      //
      //
      // If we are in the middle of a Get_Next() sequence, we need to
      // fix up any problems with the current node.
      //
      if (cursor.current == node)
      {
    cursor.current = node->next;
      }

      if (head == tail)
      {
    head = tail = 0;
      }
      else if (head == node)
      {
    head = head->next;
      }
      else if (tail == node)
      {
    tail = prev;
    tail->next = 0;
      }
      else
      {
    prev->next = node->next;
      }

      delete node;
      number--;
      cursor.current_index = -1;
      return 1;
  }
  prev = node;
  node = node->next;
    }
    return 0;
}

//*********************************************************************
//
int List::Remove(int position, int action /* = LIST_REMOVE_DESTROY */)
{
  Object *o = List::operator[](position);
  if(action == LIST_REMOVE_DESTROY) delete o;
  return List::Remove(o);
}

//*********************************************************************
// Object *List::Get_Next()
//   Return the next object in the list.
//
Object *List::Get_Next(ListCursor& cursor) const
{
    listnode  *temp = cursor.current;

    if (cursor.current)
    {
        cursor.prev = cursor.current;
  cursor.current = cursor.current->next;
  if (cursor.current_index >= 0)
      cursor.current_index++;
    }
    else
  return 0;
    return temp->object;
}


//*********************************************************************
// Object *List::Get_First()
//   Return the first object in the list.
//
Object *List::Get_First()
{
    if (head)
  return head->object;
    else
  return 0;
}


//*********************************************************************
// int List::Index(Object *obj)
//   Return the index of an object in the list.
//
int List::Index(Object *obj)
{
    listnode  *temp = head;
    int      index = 0;

    while (temp && temp->object != obj)
    {
  temp = temp->next;
  index++;
    }
    if (index >= number)
  return -1;
    else
  return index;
}


//*********************************************************************
// Object *List::Next(Object *prev)
//   Return the next object in the list.  Using this, the list will
//   appear as a circular list.
//
Object *List::Next(Object *prev)
{
    listnode  *node = head;
    while (node)
    {
  if (node->object == prev)
  {
      node = node->next;
      if (!node)
    return head->object;
      else
    return node->object;
  }
  node = node->next;
    }
  
    return 0;
}


//*********************************************************************
// Object *List::Previous(Object *next)
//   Return the next object in the list.  Using this, the list will
//   appear as a circular list.
//
Object *List::Previous(Object *next)
{
    listnode  *node = head;
    listnode   *prev = 0;
    while (node)
    {
  if (node->object == next)
  {
      if (!prev)
          return 0;
      else
    return prev->object;
  }
  prev = node;
  node = node->next;
    }
  
    return 0;
}


//*********************************************************************
//   Return the nth object in the list.
//
const Object *List::Nth(ListCursor& cursor, int n) const
{
  if (n < 0 || n >= number)
    return 0;

  listnode  *temp = head;

  if (cursor.current_index == n)
    return cursor.current->object;

  if (cursor.current && cursor.current_index >= 0 && n == cursor.current_index + 1)
    {
      cursor.prev = cursor.current;
      cursor.current = cursor.current->next;
      if (!cursor.current)
  {
    cursor.current_index = -1;
    return 0;
  }
      cursor.current_index = n;
      return cursor.current->object;
    }

  for (int i = 0; temp && i < n; i++)
    {
      temp = temp->next;
    }

  if (temp)
    {
      cursor.current_index = n;
      cursor.current = temp;
      return temp->object;
    }
  else
    return 0;
}


//*********************************************************************
// Object *List::Last()
//   Return the last object inserted.
//
Object *List::Last()
{
    if (tail)
    {
  return tail->object;
    }

    return 0;
}

//*********************************************************************
//
Object *List::Pop(int action /* = LIST_REMOVE_DESTROY */)
{
  Object *o = 0;
  listnode *ln = head;
  listnode *prev = 0;

  if (tail) {
    if(action == LIST_REMOVE_DESTROY) {
      delete tail->object;
    } else {
      o = tail->object;
    }
    if(head == tail) {
      head = tail = 0;
    } else {

      for (int i = 0; ln != tail; i++, ln = ln->next)
  prev = ln;
      tail = prev;
      tail->next = 0;
    }
  }

  return o;
}


//*********************************************************************
// Object *List::Copy() const
//   Return a deep copy of the list.
//
Object *List::Copy() const
{
    List  *list = new List;
    ListCursor  cursor;

    Start_Get(cursor);
    Object  *obj;
    while ((obj = Get_Next(cursor)))
    {
  list->Add(obj->Copy());
    }
    return list;
}


//*********************************************************************
// List &List::operator=(List &list)
//   Return a deep copy of the list.
//
List &List::operator=(List &list)
{
    Destroy();
    list.Start_Get();
    Object  *obj;
    while ((obj = list.Get_Next()))
    {
  Add(obj->Copy());
    }
    return *this;
}


//*********************************************************************
// void AppendList(List &list)
//   Move contents of other list to the end of this list, and empty the
//   other list.
//
void List::AppendList(List &list)
{
    // Never mind an empty list or ourselves.
    if (list.number == 0 || &list == this)
  return;

    // Correct our pointers in head and tail.
    if (tail)
    {
  // Link in other list.
  tail->next = list.head;

  // Update members for added contents.
  number += list.number;
  tail = list.tail;
    }
    else
    {
  head = list.head;
  tail = list.tail;
  number = list.number;
    }

    // Clear others members to be an empty list.
    list.head = list.tail = 0;
    list.cursor.current = 0;
    list.cursor.current_index = -1;
    list.number = 0;
}
