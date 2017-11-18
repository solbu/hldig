#ifndef _QueryCache_h_
#define _QueryCache_h_

//
// QueryCache.h
//
// QueryCache: (abstract) interface for the current Query cache policy.
//             A cache stores ResultLists indexed by a signature string.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: QueryCache.h,v 1.4 2004/05/28 13:15:24 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif
#include "Object.h"
#include "htString.h"

class ResultList;

// abstract
class QueryCache : public Object
{
public:
  // destructor
  virtual ~QueryCache() {}

  // get cached result for a query signature
  virtual ResultList *Lookup(const String &signature) = 0;

  // add result to be cached for a query signature
  virtual void Add(const String &signature, ResultList *entry) = 0;

protected:
  // construction
  QueryCache() {}
};

#endif
