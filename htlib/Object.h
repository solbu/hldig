//
// Object.h
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
// $Id: Object.h,v 1.4 1999/09/11 05:03:52 ghutchis Exp $
//

#ifndef	_Object_h_
#define	_Object_h_

#include "lib.h"

class String;

class Object
{
public:
	//
	// Constructor/Destructor
	//
	inline		Object();
	virtual		~Object()	{}

	//
	// To ensure a consistent comparison interface and to allow comparison
	// of all kinds of different objects, we will define a comparison functions.
	//
	virtual int	compare(Object *)	{ return 0;}

	//
	// To allow a deep copy of data structures we will define a standard interface...
	// This member will return a copy of itself, freshly allocated and deep copied.
	//
	virtual Object	*Copy()	{return new Object;}

	//
	// Persistent storage routines.
	//
	virtual void	Serialize(String &)	{}
	virtual void	Deserialize(String &, int  &)	{}

protected:
};

inline Object::Object() 
{
}

#endif
