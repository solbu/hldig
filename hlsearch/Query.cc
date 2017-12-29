// 
// Query.cc
//
// Query: (abstract) a parsed, 'executable' digger database query
//        a query tree is formed by leaf objects (ExactWordQuery) and
//        node objects (OperatorQuery) derived from this class.
//        Query execution results are returned as ResultList objects.
//        Query evaluation is cached. Cache policy is delegated to the
//        QueryCache class family.
// 
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: Query.cc,v 1.4 2004/05/28 13:15:24 lha Exp $
//

#include "Query.h"
#include "VolatileCache.h"

//
// the in-memory query result cache. the default instance is
// defined static so its destructor is called at program exit
//
VolatileCache theDefaultCache;

QueryCache *
  Query::cache = &theDefaultCache;

extern int
  debug;

//
// destructor
//
Query::~Query ()
{
}

//
// return a ResultList with the query results
// results are initially fetched from the cache
// if not cached, the query is evaluated
// Weight of the results is adjusted at each invocation, as
// the same result list may be shared by different queries
// but different weights may be assigned to the word
//
//
ResultList *
Query::GetResults ()
{
  ResultList *result = 0;

  // try to find in cache before trying eval
  String signature;
  if (cache)
  {
    signature = GetSignature ();
    result = cache->Lookup (signature);
  }

  // no cache or not in cache, evaluate
  if (!result)
  {
    if (debug)
      cerr << "EVAL: " << signature << endl;
    result = Evaluate ();

    if (cache)
    {
      cache->Add (signature, result);
    }
  }

  // adjust if something found/returned 
  if (result)
  {
    if (result->Count ())
    {
      AdjustWeight (*result);
    }
    else if (!result->IsIgnore ())
    {
      result = 0;
    }
  }
  return result;
}
