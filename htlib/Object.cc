//
// Object.cc
//
// Object: This baseclass defines how an object should behave.
//         This includes the ability to be put into a list
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Object.cc,v 1.3 1999/09/11 05:03:52 ghutchis Exp $
//

#include "Object.h"

#include <stdio.h>


//***************************************************************************
// Object::Object()
//
#ifdef NOINLINE
Object::Object()
{
}


//***************************************************************************
// Object::~Object()
//
Object::~Object()
{
}


//***************************************************************************
// int Object::compare(Object *)
//
int Object::compare(Object *)
{
	return 0;
}


//***************************************************************************
// Object *Object::Copy()
//
Object *Object::Copy()
{
	return new Object;
}


//***************************************************************************
// void Object::Serialize(String &)
//
void Object::Serialize(String &)
{
}


//***************************************************************************
// void Object::Deserialize(String &, int &)
//
void Object::Deserialize(String &, int &)
{
}
#endif


