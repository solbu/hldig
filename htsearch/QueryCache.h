#ifndef _QueryCache_h_
#define _QueryCache_h_

//
// QueryCache.h
//
// QueryCache: (abstract) interface for the current Query cache policy.
//             A cache stores ResultLists indexed by a signature string.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: QueryCache.h,v 1.1.2.1 2000/09/12 14:58:55 qss Exp $
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
