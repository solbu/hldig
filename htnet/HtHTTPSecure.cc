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
// $Id: HtHTTPSecure.cc,v 1.3 2003/06/24 19:58:07 nealr Exp $ 
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_SSL_H

#include "HtHTTPSecure.h"
#include <iostream.h>

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
