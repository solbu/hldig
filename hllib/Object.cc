//
// Object.cc
//
// Object: This baseclass defines how an object should behave.
//         This includes the ability to be put into a list
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: Object.cc,v 1.6 2004/05/28 13:15:21 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "hlconfig.h"
#endif /* HAVE_CONFIG_H */

#include "Object.h"

#include <stdio.h>


//***************************************************************************
// Object::Object()
//
#ifdef NOINLINE
Object::Object ()
{
}


//***************************************************************************
// Object::~Object()
//
Object::~Object ()
{
}


//***************************************************************************
// int Object::compare(Object *)
//
int
Object::compare (Object *)
{
  return 0;
}


//***************************************************************************
// Object *Object::Copy()
//
Object *
Object::Copy ()
{
  return new Object;
}


//***************************************************************************
// void Object::Serialize(String &)
//
void
Object::Serialize (String &)
{
}


//***************************************************************************
// void Object::Deserialize(String &, int &)
//
void
Object::Deserialize (String &, int &)
{
}
#endif
