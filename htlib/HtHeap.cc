//
// HtHeap.h
//
//: A Heap class which holds objects of type Object.
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
//
#if RELEASE
static char	RCSid[] = "$Id: HtHeap.cc,v 1.5 1999/09/05 06:49:47 ghutchis Exp $";
#endif

#include "HtHeap.h"
#include "fstream.h"

//*********************************************************************
// void HtHeap::HtHeap()
//   Default constructor
//
HtHeap::HtHeap()
{
  data = new HtVector;
}


//*********************************************************************
// void HtHeap::HtHeap(HtVector vector)
//   Constructor from vector
//   (has the side effect of not allocating double memory)
//
HtHeap::HtHeap(HtVector vector)
{
  int size = vector.Count();
  data = vector.Copy();

  // Now we have to "heapify" -- start at the first interior node
  // And push each node down into its subtree
  // (This is O(n)!)
  for (int i = parentOf(size); i >= 0; i--)
    pushDownRoot(i);
}


//*********************************************************************
// void HtHeap::~HtHeap()
//   Destructor
//
HtHeap::~HtHeap()
{
  Destroy();
}


//*********************************************************************
// void HtHeap::Destroy()
//   Deletes all objects from the heap
//
void HtHeap::Destroy()
{
  data->Destroy();
  delete data;
}


//*********************************************************************
// void HtHeap::Add(Object *object)
//   Add an object to the heap.
//
void HtHeap::Add(Object *object)
{
  data->Add(object);
  percolateUp(data->Count() - 1);
}



//*********************************************************************
// Object  *HtHeap::Remove()
//   Remove an object from the top of the heap
//   This requires re-heapifying by placing the last element on the top
//   and pushing it down.
//
Object *HtHeap::Remove()
{
  Object *min = Peek();

  data->Assign(data->Last(), 0);
  data->RemoveFrom(data->Count()-1);
  
  if (data->Count() > 1)
    pushDownRoot(0);

  return min;
}

//*********************************************************************
// HtHeap *HtHeap::Copy()
//   Return a deep copy of the heap.
//
HtHeap *HtHeap::Copy()
{
    HtHeap	*heap = new HtHeap(*data);

    return heap;
}


//*********************************************************************
// HtHeap &HtHeap::operator=(HtHeap &heap)
//   Return a deep copy of the heap.
//
HtHeap &HtHeap::operator=(HtHeap &heap)
{
    Destroy();
    data = heap.data;
    return *this;
}

//*********************************************************************
// voide HtHeap::percolateUp(int leaf)
// Pushes the node pointed to by leaf upwards
// it will travel as far as possible upwards to ensure the data is a heap
//
void HtHeap:: percolateUp(int leaf)
{
  int parent = parentOf(leaf);
  Object *value = data->Nth(leaf);
  while (leaf > 0 &&
	 (value->compare(data->Nth(parent)) < 0))
    {
      data->Assign(data->Nth(parent), leaf);
      leaf = parent;
      parent = parentOf(leaf);
    }
  data->Assign(value, leaf);
}

//*********************************************************************
// void HtHeap::pushDownRoot(int root)
// Pushes the node pointed to by root into the heap
// it will go down as far as necessary to ensure the data is a heap
//
void HtHeap::pushDownRoot(int root)
{
  int size = data->Count() - 1;
  Object *value = data->Nth(root); 
  while (root < size)
    {
      int childPos = leftChildOf(root);
      if (childPos < size)
	{
	  if ( rightChildOf(root) < size &&
	       data->Nth(childPos + 1)->compare(data->Nth(childPos)) < 0 )
	    {
	      childPos++;
	    }
	  if ( data->Nth(childPos)->compare(value) < 0 ) // -1, so smaller
	    {
	      // We have to swap this node with the root and then loop
	      data->Assign(data->Nth(childPos), root);
	      data->Assign(value, childPos);
	      root = childPos;
	    }
	  else
	    {
	      // Found the right position, so we're done
	      data->Assign(value, root);
	      return;
	    }
	}
      else // childPos >= heapSize
	{
	  // At a leaf, so we're done
	  data->Assign(value, root);
	  return;
	}
    }
}
