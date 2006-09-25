//
// HtVector.h
//
// HtVector: A Vector class which holds objects of type Object.
//           (A vector is an array that can expand as necessary)
//           This class is very similar in interface to the List class
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: HtVector.h,v 1.1.2.1 2006/09/25 22:51:14 aarnone Exp $
//
//
#ifndef	_HtVector_h_
#define	_HtVector_h_
#include "Object.h"

class HtVector : public Object
{
public:
    //
    // Constructor/Destructor
    //
    HtVector();
    HtVector(int capacity);
    ~HtVector();

    //
    // Add() will append an Object to the end of the vector
    //
    void	Add(Object *);

    //
    // Insert() will insert an object at the given position.  If the
    // position is larger than the number of objects in the vector, the
    // object is appended; no new objects are created between the end
    // of the vector and the given position.
    //
    void	Insert(Object *, int position);

    //
    // Assign() will assign the object to the given position, replacing
    // the object currently there. It is functionally equivalent to calling
    // RemoveFrom() followed by Insert()
    void	Assign(Object *, int position);

    //
    // Find the given object in the vector and remove it from the vector.
    // The object will NOT be deleted.  If the object is not found,
    // NOTOK will be returned, else OK.
    //
    int		Remove(Object *);

    //
    // Remove the object at the given position
    // (in some sense, the inverse of Insert)
    //
    int		RemoveFrom(int position);

    //
    // Release() will remove all the objects from the vector.
    // This will NOT delete them
    void	Release();

    //
    // Destroy() will delete all the objects in the vector.  This is
    // equivalent to calling the destructor
    //
    void	Destroy();

    //
    // Vector traversel (a bit redundant since you can use [])
    //
    void		Start_Get()		{current_index = -1;}
    Object		*Get_Next();
    Object		*Get_First();
    Object		*Next(Object *current);
    Object		*Previous(Object *current);
    Object		*Last()			{return element_count<=0?(Object *)NULL:data[element_count-1];}

    //
    // Direct access to vector items. To assign new objects, use
    // Insert() or Add() or Assign()
    //
    Object		*operator[] (int n)		{return (n<0||n>=element_count)?(Object *)NULL:data[n];}
    Object		*Nth(int n)			{return (n<0||n>=element_count)?(Object *)NULL:data[n];}

    //
    // Access to the number of elements
    //
    int			Count() const		{return element_count;}
    int			IsEmpty()		{return element_count==0;}

    //
    // Get the index number of an object.  If the object is not found,
    // returns -1
    //
    int			Index(Object *);

    //
    // Deep copy member function
    //
    Object			*Copy() const;

    //
    // Vector Assignment
    //
    HtVector		&operator= (HtVector *vector) {return *this = *vector;}
    HtVector		&operator= (HtVector &vector);

protected:
    //
    // The actual internal data array
    Object		**data;

    //
    // For traversal it is nice to know where we are...
    //
    int			current_index;

    //
    // It's nice to keep track of how many things we contain...
    // as well as how many slots we've declared
    //
    int			element_count;
    int			allocated;

    //
    // Protected function to ensure capacity
    //
    void                 Allocate(int ensureCapacity);
};

#endif
