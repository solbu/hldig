//
// IntObject.h
//
// IntObject: int variable encapsulated in Object derived class
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: IntObject.h,v 1.4.2.1 2000/05/10 18:23:45 loic Exp $
//

#ifndef _IntObject_h_
#define _IntObject_h_

#include "Object.h"

class IntObject : public Object
{
public:
	//
	// Construction/Destruction
	//
	IntObject();
	IntObject(int v) { value = v; }
	~IntObject();

	int			Value()			{return value;}
	void			Value(int v)		{value = v;}

private:
	int			value;
};

#endif


