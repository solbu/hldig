//
// HtVector.cc
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
// $Id: HtVector.cc,v 1.11 2004/05/28 13:15:21 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "HtVector.h"

//*********************************************************************
// void HtVector::HtVector()
//   Default constructor
//
HtVector::HtVector()
{
  data = new Object  *[4]; // After all, why would anyone want an empty vector?
  element_count = 0;
  allocated = 4;
  current_index = -1;
}


//*********************************************************************
// void HtVector::HtVector(int capacity)
//   Constructor with known capacity
//   (has the side effect of not allocating double memory)
//
HtVector::HtVector(int capacity)
{
  data = new Object  *[capacity];
  element_count = 0;
  allocated = capacity;
  current_index = -1;
}


//*********************************************************************
// void HtVector::~HtVector()
//   Destructor
//
HtVector::~HtVector()
{
  Destroy();
}


//*********************************************************************
// void HtVector::Release()
//   Remove all objects from the vector, but do not delete them
void HtVector::Release()
{
  for (current_index = 0; current_index < element_count; current_index++)
    {
      data[current_index] = NULL;
    }
  if (data)
    delete [] data;
  data = NULL;
  allocated = 0;
  element_count = 0;
  current_index = -1;
}

//*********************************************************************
// void HtVector::Destroy()
//   Deletes all objects from the vector
//
void HtVector::Destroy()
{
  for (current_index = 0; current_index < element_count; current_index++)
    if (data[current_index])
      {
  delete data[current_index];
  data[current_index] = NULL;
      }
  if (data)
    delete [] data;
  data = NULL;
  allocated = 0;
  element_count = 0;
  current_index = -1;
}


//*********************************************************************
// void HtVector::Add(Object *object)
//   Add an object to the list.
//
void HtVector::Add(Object *object)
{
  Allocate(element_count+1);
  data[element_count] = object;
  element_count += 1;
}


//*********************************************************************
// void HtVector::Insert(Object *object, int position)
//   Add an object into the list.
//
void HtVector::Insert(Object *object, int position)
{
  if (position < 0) return;
  if (position >= element_count)
    {
      Add(object);
      return;
    }
  
  Allocate(element_count + 1);
  for (int i = element_count; i > position; i--)
    data[i] = data[i-1];
  data[position] = object;
  element_count += 1;
}


//*********************************************************************
// void HtVector::Assign(Object *object, int position)
// Assign an object to the position
//
void HtVector:: Assign(Object *object, int position)
{
  // Simply perform an insert, followed by a remove!
  Insert(object, position);
  RemoveFrom(position + 1);
  return;
}


//*********************************************************************
// int HtVector::Remove(Object *object)
//   Remove an object from the list.
//
int HtVector::Remove(Object *object)
{
  return RemoveFrom(Index(object));
}


//*********************************************************************
// int HtVector::RemoveFrom(int position)
//   Remove an object from the list.
//
int HtVector::RemoveFrom(int position)
{
  if (position < 0 || position >= element_count)
    return NOTOK;

  for (int i = position; i < element_count - 1; i++)
    data[i] = data[i+1];
  element_count -= 1;
  return OK;
}


//*********************************************************************
// Object *HtVector::Get_Next()
//   Return the next object in the list.
//
Object *HtVector::Get_Next()
{
  current_index++;
  if (current_index >= element_count)
    return 0;
  return data[current_index];
}


//*********************************************************************
// Object *HtVector::Get_First()
//   Return the first object in the list.
//
Object *HtVector::Get_First()
{
    if (!IsEmpty())
      {
  current_index = 0;
  return data[0];
      }
    else
  return 0;
}


//*********************************************************************
// int HtVector::Index(Object *obj)
//   Return the index of an object in the list.
//
int HtVector::Index(Object *obj)
{
    int      index = 0;

    while (index < element_count && data[index] != obj)
    {
  index++;
    }
    if (index >= element_count)
  return -1;
    else
  return index;
}


//*********************************************************************
// Object *HtVector::Next(Object *prev)
//   Return the next object in the list.  Using this, the list will
//   appear as a circular list.
//
Object *HtVector::Next(Object *prev)
{
  current_index = Index(prev);
  if (current_index == -1)
    return 0;

  current_index++; // We should probably do this with remainders
  if (current_index >= element_count)
    current_index = 0;
  return data[current_index];
}


//*********************************************************************
// Object *HtVector::Previous(Object *next)
//   Return the previous object in the vector.  Using this, the vector will
//   appear as a circular list.
//
Object *HtVector::Previous(Object *next)
{
  current_index = Index(next);
  if (current_index == -1)
    return 0;

  current_index--; // We should probably do this with remainders
  if (current_index < 0)
    current_index = element_count - 1;
  return data[current_index];
}


//*********************************************************************
// Object *HtVector::Copy() const
//   Return a deep copy of the vector.
//
Object *HtVector::Copy() const
{
    HtVector  *vector = new HtVector(allocated);

    for(int i = 0; i < Count(); i++)
      vector->Add(data[i]->Copy());

    return vector;
}


//*********************************************************************
// HtVector &HtVector::operator=(HtVector &vector)
//   Return a deep copy of the list.
//
HtVector &HtVector::operator=(HtVector &vector)
{
    Destroy();

    for(int i = 0; i < vector.Count(); i++)
      Add(vector.data[i]->Copy());

    return *this;
}


//*********************************************************************
// int Allocate(int capacity)
//    Ensure there is at least capacity space in the vector
//
void HtVector::Allocate(int capacity)
{
  if (capacity > allocated) // Darn, we actually have to do work :-)
    {
      Object  **old_data = data;

      // Ensure we have more than the capacity and we aren't
      // always rebuilding the vector (which leads to quadratic behavior)
      while (allocated < capacity)
  allocated *= 2;

      data = new Object *[allocated];

      for (int i = 0; i < element_count; i++)
  {
    data[i] = old_data[i];
    old_data[i] = NULL;
  }

      if (old_data)
  delete [] old_data;
    }
}
