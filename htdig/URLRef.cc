//
// URLRef.cc
//
// A definition of a URL/Referer pair with associated hopcount
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
//
#if RELEASE
static char RCSid[] = "$Id: URLRef.cc,v 1.3 1999/08/27 15:52:48 ghutchis Exp $";
#endif

#include "URLRef.h"


//*****************************************************************************
// URLRef::URLRef()
//
URLRef::URLRef()
{
  hopcount = 0;
}


//*****************************************************************************
// URLRef::~URLRef()
//
URLRef::~URLRef()
{
}


//*****************************************************************************
// int URLRef::compare(Object *to)
//
int URLRef::compare(Object *to)
{
  URLRef	*u = (URLRef *) to;

  return hopcount - u->hopcount;
}



