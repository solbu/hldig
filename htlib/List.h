//
// List.h
//
// A List class which holds objects of type Object.
//
// $Id: List.h,v 1.1.1.1 1997/02/03 17:11:04 turtle Exp $
//
// $Log: List.h,v $
// Revision 1.1.1.1  1997/02/03 17:11:04  turtle
// Initial CVS
//
//
#ifndef	_List_h_
#define	_List_h_
#include "Object.h"

struct listnode
{
    listnode		*next;
    listnode		*prev;
    Object		*object;
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
    void		Start_Get()	{current = head; current_index = -1;}
    Object		*Get_Next();
    Object		*Get_First();
    Object		*Next(Object *current);
    Object		*Previous(Object *current);
    Object		*Last();

    //
    // Direct access to list items.  This can only be used to retrieve
    // objects from the list.  To assign new objects, use Insert(),
    // Add(), or Assign().
    //
    Object		*operator[] (int n)		{return Nth(n);}
    Object		*Nth(int n);

    //
    // Access to the number of elements
    //
    int			Count() const			{return number;}

    //
    // Get the index number of an object.  If the object is not found,
    // returnes -1
    //
    int			Index(Object *);

    //
    // Deep copy member function
    //
    Object		*Copy();

    //
    // Assignment
    //
    List		&operator= (List *list)		{return *this = *list;}
    List		&operator= (List &list);

protected:
    //
    // Pointers into the list
    //
    listnode		*head;
    listnode		*tail;

    //
    // For list traversal it is nice to know where we are...
    //
    listnode		*current;
    int			current_index;

    //
    // Its nice to keep track of how many things we contain...
    //
    int			number;
};

#endif
