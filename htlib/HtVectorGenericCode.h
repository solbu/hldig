//
// HtVectorGenericCode.h
//
// HtVectorGeneric: A Vector class which holds objects of type GType.
//           (A vector is an array that can expand as necessary)
//           This class is very similar in interface to the List class
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtVectorGenericCode.h,v 1.3 2002/02/01 22:49:33 ghutchis Exp $
//


//*********************************************************************
// void HtVectorGType::HtVectorGType()
//   Default constructor
//
HtVectorGType::HtVectorGType()
{
  data = new GType[4]; // After all, why would anyone want an empty vector?
  element_count = 0;
  allocated = 4;
  current_index = -1;
}


//*********************************************************************
// void HtVectorGType::HtVectorGType(int capacity)
//   Constructor with known capacity
//   (has the side effect of not allocating double memory)
//
HtVectorGType::HtVectorGType(int capacity)
{
  data = new GType[capacity];
  element_count = 0;
  allocated = capacity;
  current_index = -1;
}


//*********************************************************************
// void HtVectorGType::~HtVectorGType()
//   Destructor
//
HtVectorGType::~HtVectorGType()
{
  Destroy();
}



//*********************************************************************
// void HtVectorGType::Destroy()
//   Deletes all objects from the vector
//
void HtVectorGType::Destroy()
{
  if (data)
    delete [] data;
  data = NULL;
  allocated = 0;
  element_count = 0;
  current_index = -1;
}



//*********************************************************************
// void HtVectorGType::Insert(GType object, int position)
//   Add an object into the list.
//
void HtVectorGType::Insert(const GType &object, int position)
{
  if (position < 0) {CheckBounds(position);}
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
// int HtVectorGType::RemoveFrom(int position)
//   Remove an object from the list.
//
void HtVectorGType::RemoveFrom(int position)
{
    CheckBounds(position);

    for (int i = position; i < element_count - 1; i++)
    {
	data[i] = data[i+1];
    }
    element_count -= 1;
}


//*********************************************************************
// GType HtVectorGType::Get_Next()
//   Return the next object in the list.
//
GType &HtVectorGType::Get_Next()
{
  current_index++;
  CheckBounds(current_index);
  return data[current_index];
}


//*********************************************************************
// GType HtVectorGType::Get_First()
//   Return the first object in the list.
//
GType &HtVectorGType::Get_First()
{
    CheckBounds(0);
    return data[0];
}

#ifndef HTVECTORGENERIC_NOTCOMPARABLE

//*********************************************************************
// int HtVectorGType::Index(GType obj)
//   Return the index of an object in the list.
//
int HtVectorGType::Index(const GType &obj)
{
    int			index0 = 0;

    while (index0 < element_count && data[index0] != obj)
    {
	index0++;
    }
    if (index0 >= element_count)
	return -1;
    else
	return index0;
}


//*********************************************************************
// GType HtVectorGType::Next(GType prev)
//   Return the next object in the list.  Using this, the list will
//   appear as a circular list.
//
GType &HtVectorGType::Next(const GType & prev)
{
  current_index = Index(prev);
  CheckBounds(current_index);

  current_index++; // We should probably do this with remainders
  return Nth(current_index);
}

//*********************************************************************
// GType HtVectorGType::Previous(GType next)
//   Return the previous object in the vector.  Using this, the vector will
//   appear as a circular list.
//
GType &HtVectorGType::Previous(const GType & next)
{
  current_index = Index(next);
  CheckBounds(current_index);

  current_index--; // We should probably do this with remainders
  return Nth(current_index);
}

//*********************************************************************
// int HtVectorGType::Remove(GType object)
//   Remove an object from the list.
//
void HtVectorGType::Remove(const GType &object)
{
    int pos = Index(object);
    CheckBounds(pos);
    RemoveFrom(pos);
}
#endif

//*********************************************************************
// HtVectorGType *HtVectorGType::Copy() const
//   Return a deep copy of the vector.
//
Object		 *HtVectorGType::Copy() const
{
    HtVectorGType	*vector = new HtVectorGType(allocated);

    for(int i = 0; i < Count(); i++)
{
#ifdef HTVECTORGENERIC_OBJECTPTRTYPE
      vector->Add(data[i]->Copy());
#else
      vector->Add(data[i]);
#endif
}
    return vector;
}


//*********************************************************************
// HtVectorGType &HtVectorGType::operator=(HtVectorGType &vector)
//   Return a deep copy of the list.
//
HtVectorGType &HtVectorGType::operator=(const HtVectorGType &vector)
{
    Destroy();

    for(int i = 0; i < vector.Count(); i++)
    {
	Add(vector.data[i]);
    }
    return *this;
}


//*********************************************************************
// int Allocate(int capacity)
//    Ensure there is at least capacity space in the vector
//
void HtVectorGType::ActuallyAllocate(int capacity)
{
  if (capacity > allocated) // Darn, we actually have to do work :-)
    {
      GType	*old_data = data;

      // Ensure we have more than the capacity and we aren't
      // always rebuilding the vector (which leads to quadratic behavior)
      if(!allocated){allocated=1;}
      while (allocated < capacity)
	allocated *= 2;

      data = new GType[allocated];

      for (int i = 0; i < element_count; i++)
	{
	  data[i] = old_data[i];
	}

      if (old_data)
	delete [] old_data;
    }
}


#ifdef  HTVECTORGENERIC_NOTCOMPARABLE
#undef  HTVECTORGENERIC_NOTCOMPARABLE
#endif

#undef HtVectorGType
#undef GType
