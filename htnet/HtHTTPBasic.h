//
// HtHTTPBasic.h
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
// $Id: HtHTTPBasic.h,v 1.3 2003/06/24 19:58:07 nealr Exp $ 
//

#ifndef _HTHTTPBASIC_H
#define _HTHTTPBASIC_H

#include "HtHTTP.h"      // We inherrit from this
#include "Transport.h"
#include "Connection.h"
#include "URL.h"
#include "htString.h"

class HtHTTPBasic : public HtHTTP
{
 public:

  HtHTTPBasic();
  ~HtHTTPBasic();

};

#endif
