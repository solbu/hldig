//
// VolatileCache.cc
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
// $Id: VolatileCache.cc,v 1.1.2.1 2000/09/12 14:58:55 qss Exp $ 
//

#include "VolatileCache.h"
#include "ResultList.h"

//
// a pseudo-constant empty result list
// used to avoid null pointers in the cache
//
ResultList theEmptyResult;

ResultList * const
VolatileCache::empty = &theEmptyResult;

extern int debug;

//
// find a cache entry
//
ResultList *
VolatileCache::Lookup(const String &signature)
{
	ResultList *result = (ResultList *)cache[signature];
	return result;
}

//
// add a cache entry
//
void
VolatileCache::Add(const String &signature, ResultList *entry)
{
	ResultList *previous = (ResultList *)cache[signature];
	if(previous && previous != empty)
	{
		delete previous;
	}
	if(!entry)
	{
		entry = empty;
	}
	cache.Add(signature, entry);
}

//
// clear the in-memory cache
// avoids deletion of the shared 'empty' element
//
VolatileCache::~VolatileCache()
{
	if(debug) cerr << "query CLEAR: entries=" << cache.Count() << endl;
	cache.Start_Get();
	ResultList *kill = (ResultList *)cache.Get_NextElement();
	while(kill)
	{
		if(kill != empty)
		{
			delete kill;
		}
		kill = (ResultList *)cache.Get_NextElement();
	}
	cache.Release();
}

