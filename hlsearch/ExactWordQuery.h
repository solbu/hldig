#ifndef _ExactWordQuery_h_
#define _ExactWordQuery_h_

//
// ExactWordQuery.h
//
// ExactWordQuery: A Query tree leaf object. Wraps a database access
//                 that generates ResultLists for word matches.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: ExactWordQuery.h,v 1.4 2004/05/28 13:15:24 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "hlconfig.h"
#endif

#include "Query.h"

class WordSearcher;

class ExactWordQuery:public Query
{
public:
  // construct for word w
  ExactWordQuery (const String & w):word (w), weight (1.0)
  {
  }

  // destruct
   ~ExactWordQuery ()
  {
  }

  // set the common db wrapper
  static void SetSearcher (WordSearcher * c)
  {
    searcher = c;
  }

  // weight accessor
  void SetWeight (double x)
  {
    weight = x;
  }
  double GetWeight () const
  {
    return weight;
  }

private:
  // forbidden
    ExactWordQuery ()
  {
  }

  // go search the db
  ResultList *Evaluate ();

  // set my weight to the list
  void AdjustWeight (ResultList &);

  // unparse
  String GetLogicalWords () const
  {
    return word;
  }

  // unique cache index
  String GetSignature () const
  {
    return String ("Exact:") + GetLogicalWords ();
  }

  // i represent this
  String word;

  // my weight
  double weight;

  // db wrapper common to all word queries
  static WordSearcher *searcher;
};

#endif
