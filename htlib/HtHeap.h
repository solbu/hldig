//
// HtHeap.h
//
// A Heap class which holds objects of type Object.
// (A heap is a semi-ordered tree-like structure.
//  it ensures that the first item is *always* the largest.
//  NOTE: To use a heap, you must implement the Compare() function for your
//        Object classes. The assumption used here is -1 means less-than, 0
//        means equal, and +1 means greater-than. Thus this is a "min heap"
//        for that definition.)
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtHeap.h,v 1.2 1999/04/14 19:36:01 bergolth Exp $
//
//
#ifndef	_HtHeap_h_
#define	_HtHeap_h_
#include "Object.h"
#include "HtVector.h"

class HtHeap : public Object
{
public:
    //
    // Constructor/Destructor
    //
    HtHeap();
    HtHeap(HtVector vector);
    ~HtHeap();

    //
    // Add() will add an Object to the heap in the appropriate location
    //
    void		Add(Object *);

    //
    // Destroy() will delete all the objects in the heap.  This is
    // equivalent to calling the destructor
    //
    void		Destroy();

    //
    // Peek() will return a reference to the top object in the heap.
    //
    Object		*Peek()			{return data->Nth(0);}

    //
    // Remove() will return a reference as Peek() but will also
    // remove the reference from the heap and re-heapify
    //
    Object 		*Remove();

    //
    // Access to the number of elements
    //
    int			Count() 		{return data->Count();}
    int			IsEmpty()		{return data->IsEmpty();}

    //
    // Deep copy member function
    //
    HtHeap		*Copy();

    //
    // Assignment
    //
    HtHeap		&operator= (HtHeap *heap)  {return *this = *heap;}
    HtHeap		&operator= (HtHeap &heap);

protected:
    // The vector class should keep track of everything for us
    HtVector		*data;

    // Functions for establishing the relations between elements
    int	parentOf (int i)
      { return (i - 1)/2; }
    int	leftChildOf (int i)
      { return 2*i + 1; }
    int rightChildOf (int i)
      { return 2* (i+1); }

    // Protected procedures for performing heap-making operations
    void percolateUp (int leaf);  // pushes the node up as far as possible
    void pushDownRoot (int root); // pushes the node down as necessary
};

#endif
