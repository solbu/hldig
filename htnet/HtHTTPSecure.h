//
// HtHTTPSecure.h
//
// HtHTTPSecure: Class for HTTP/HTTPS messaging (derived from Transport)
//		Uses an SSLConnection for secure connections.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: HtHTTPSecure.h,v 1.4 2004/05/28 13:15:23 lha Exp $ 
//

#ifndef _HTHTTPSECURE_H
#define _HTHTTPSECURE_H

#include "HtHTTP.h"
#include "Transport.h"
#include "SSLConnection.h"
#include "URL.h"
#include "htString.h"

#ifdef HAVE_SSL_H

class HtHTTPSecure : public HtHTTP
{
 public:
  HtHTTPSecure();
  ~HtHTTPSecure();
};

#endif

#endif

