//
// HtHTTPSecure.cc
//
// HtHTTPSecure: Class for HTTP/HTTPS messaging (derived from Transport)
//		Uses an SSLConnection for secure connections.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: HtHTTPSecure.cc,v 1.4 2003/07/21 08:16:11 angusgb Exp $ 
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_SSL_H

#include "HtHTTPSecure.h"

#ifdef HAVE_STD
#include <iostream>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif
#else
#include <iostream.h>
#endif /* HAVE_STD */

// HtHTTPSecure constructor
//
HtHTTPSecure::HtHTTPSecure()
: HtHTTP(*(new SSLConnection())) // Create a new "secure" connection
{
}

// HtHTTPSecure destructor
//
HtHTTPSecure::~HtHTTPSecure()
{
}

#endif
