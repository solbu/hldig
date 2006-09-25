//
// HtHTTPBasic.cc
//
// HtHTTPBasic: Class for HTTP messaging (derived from Transport)
//              Does not handle HTTPS connections -- use HtHTTPSecure
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: HtHTTPBasic.cc,v 1.1.2.1 2006/09/25 23:51:00 aarnone Exp $ 
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_STD
#include <iostream>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif
#else
#include <iostream.h>
#endif /* HAVE_STD */

#include "HtHTTPBasic.h"

// HtHTTPBasic constructor
//
HtHTTPBasic::HtHTTPBasic()
: HtHTTP(*(new Connection())) // Creates a new connection
{
}

// HtHTTPBasic destructor
//
HtHTTPBasic::~HtHTTPBasic()
{
}

