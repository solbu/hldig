//
// List.h
//
// List: A List class which holds objects of type Object.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: List.h,v 1.7 2002/02/01 22:49:34 ghutchis Exp $
//

#ifndef	_List_h_
#define	_List_h_

#include "Object.h"

//
// Behaviour of the Remove method. See comment before method
// declaration for more information.
//
#define LIST_REMOVE_DESTROY	1
#define LIST_REMOVE_RELEASE	2

class List;
class listnode;

class ListCursor {
 public:
  ListCursor() { current = 0; prev = 0; current_index = -1; }
  void Clear() { current = 0; prev = 0; current_index = -1; }

  //
  // Support for the Start_Get and Get_Next routines
  //
  listnode		*current;
  listnode		*prev;
  int			current_index;
};

class List : public Object
{
public:
    //
    // Constructor/Destructor
    //
    List();
    virtual		~List();

    //
    // Insert at beginning of list.
    //
    virtual void	Unshift(Object *o) { Insert(o, 0); }
    //
    // Remove from the beginning of the list and return the
    // object.
    //
    virtual Object*	Shift(int action = LIST_REMOVE_DESTROY) {
      Object* o = Nth(0);
      if(Remove(0, action) == NOTOK) return 0;
      return o;
    }
    //
    // Append an Object to the end of the list
    //
    virtual void	Push(Object *o) { Add(o); }
    //
    // Remove the last object from the list and return it.
    //
    virtual Object	*Pop(int action = LIST_REMOVE_DESTROY);

    //
    // Add() will append an Object to the end of the list
    //
    virtual void	Add(Object *);

    //
    // Insert() will insert an object at the given position.  If the
    // position is larger than the number of objects in the list, the
    // object is appended; no new objects are created between the end
    // of the list and the given position.
    //
    virtual void	Insert(Object *, int position);

    //
    // Assign() will replace the object already at the given position
    // with the new object.  If there is no object at the position,the
    // list is extended with nil objects until the position is reached
    // and then the given object is put there.  (This really makes the
    // List analogous to a dynamic array...)
    //
    virtual void	Assign(Object *, int position);

    //
    // Find the given object in the list and remove it from the list.
    // The object will NOT be deleted.  If the object is not found,
    // NOTOK will be returned, else OK.
    //
    virtual int		Remove(Object *);

    //
    // Remove object at position from the list. If action is 
    // LIST_REMOVE_DESTROY delete the object stored at position.
    // If action is LIST_REMOVE_RELEASE the object is not deleted.
    // If the object is not found,
    // NOTOK will be returned, else OK.
    //
    virtual int		Remove(int position, int action = LIST_REMOVE_DESTROY);

    //
    // Release() will set the list to empty.  This call will NOT
    // delete objects that were in the list before this call.
    //
    virtual void	Release();

    //
    // Destroy() will delete all the objects in the list.  This is
    // equivalent to calling the destructor
    //
    virtual void	Destroy();

    //
    // List traversel
    //
    void		Start_Get()	{ Start_Get(cursor); }
    void		Start_Get(ListCursor& cursor0) const { cursor0.current = head; cursor0.prev = 0; cursor0.current_index = -1;}
    Object		*Get_Next()	{ return Get_Next(cursor); }
    Object		*Get_Next(ListCursor& cursor) const;
    Object		*Get_First();
    Object		*Next(Object *current);
    Object		*Previous(Object *current);
    Object		*Last();

    //
    // Direct access to list items.  This can only be used to retrieve
    // objects from the list.  To assign new objects, use Insert(),
    // Add(), or Assign().
    //
    Object		*operator[] (int n)		{ return Nth(n); }
    const Object	*operator[] (int n) const	{ return Nth(((List*)this)->cursor, n); }
    const Object	*Nth(ListCursor& cursor, int n) const;
    const Object	*Nth(int n) const { return Nth(((List*)this)->cursor, n); }
    Object		*Nth(int n) { return (Object*)((List*)this)->Nth(((List*)this)->cursor, n); }

    //
    // Access to the number of elements
    //
    int			Count() const			{ return number; }

    //
    // Get the index number of an object.  If the object is not found,
    // returnes -1
    //
    int			Index(Object *);

    //
    // Deep copy member function
    //
    Object		*Copy() const;

    //
    // Assignment
    //
    List		&operator= (List *list)		{return *this = *list;}
    List		&operator= (List &list);

    // Move one list to the end of another, emptying the other list.
    void		AppendList (List &list);

protected:
    //
    // Pointers into the list
    //
    listnode		*head;
    listnode		*tail;

    //
    // For list traversal it is nice to know where we are...
    //
    ListCursor		cursor;

    //
    // Its nice to keep track of how many things we contain...
    //
    int			number;
};

#endif
