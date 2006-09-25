//
// URLRef.cc
//
// URLRef: A definition of a URL/Referer pair with associated hopcount
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: URLRef.cc,v 1.1.2.1 2006/09/25 23:51:13 aarnone Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "URLRef.h"
#include "HtDebug.h"


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
    HtDebug * debug = HtDebug::Instance();
    debug->outlog(10, "URLRef destructor start\n");

    debug->outlog(10, "URLRef destructor done\n");
}


//*****************************************************************************
//
int URLRef::compare(const URLRef& to) const
{
  return hopcount - to.hopcount;
}



