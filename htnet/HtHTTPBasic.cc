//
// HtHTTPBasic.cc
//
// HtHTTPBasic: Class for HTTP messaging (derived from Transport)
//              Does not handle HTTPS connections -- use HtHTTPSecure
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: HtHTTPBasic.cc,v 1.3 2003/06/24 19:58:07 nealr Exp $ 
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <iostream.h>
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

