//
// URLRef.cc
//
// URLRef: A definition of a URL/Referer pair with associated hopcount
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: URLRef.cc,v 1.6.2.1 2000/05/06 20:46:38 loic Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

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
//
int URLRef::compare(const URLRef& to) const
{
  return hopcount - to.hopcount;
}



