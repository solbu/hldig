#ifndef _VolatileCache_h_
#define _VolatileCache_h_

//
// VolatileCache.h
//
// VolatileCache: the simplest non-persistent Query result cache.
//                This is default policy.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: VolatileCache.h,v 1.1.2.1 2000/09/12 14:58:55 qss Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif

#include "QueryCache.h"
#include "Dictionary.h"

class VolatileCache : public QueryCache
{
public:
	// cons & destr
	VolatileCache() {}
	~VolatileCache();

	// get cached result from in-memory cache
	ResultList *Lookup(const String &signature);

	// add result to in-memory cache
	void Add(const String &signature, ResultList *entry);

private:
	Dictionary cache;
	static ResultList * const empty;
};

#endif
