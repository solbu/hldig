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
static char RCSid[] = "$Id: URLRef.cc,v 1.2 1999/07/03 20:56:39 ghutchis Exp $";
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


