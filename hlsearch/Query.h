#ifndef __Query_h__
#define __Query_h__

//
// Query.h
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
// $Id: Query.h,v 1.4 2004/05/28 13:15:24 lha Exp $
//

//
// for details about the basic architectural patterns see the book:
// Design Patterns, by the infamous GoF
// Interpreter pattern
// Factory pattern
// Flyweight pattern
//

#ifdef HAVE_CONFIG_H
#include "hlconfig.h"
#endif

#include "Object.h"
#include "htString.h"
#include "ResultList.h"

class QueryCache;

// abstract
class Query:public Object
{
public:
  // destr
  virtual ~ Query ();

  // does nothing here -- hack for comfortable parser coding
  virtual void Add (Query *)
  {
  }

  // get a boolean-style query string
  virtual String GetLogicalWords () const = 0;

  // evaluate if necessary and return results
  ResultList *GetResults ();

  // set a cache policy 
  static void SetCache (QueryCache * c)
  {
    cache = c;
  }

protected:
  // get an unique cache index
  virtual String GetSignature ()const = 0;

  Query ()
  {
  }

  // generate results
  virtual ResultList *Evaluate () = 0;

  // by default, nothing -- for use of leaf queries
  virtual void AdjustWeight (ResultList &)
  {
  }

private:
  // the current cache object, if some
  static QueryCache *cache;
};

#endif
