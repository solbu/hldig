//
// NearQuery.cc
//
// NearQuery: An operator query that filters matches by proximity.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: NearQuery.cc,v 1.4 2004/05/28 13:15:24 lha Exp $
//

#include "NearQuery.h"

String NearQuery::OperatorString () const
{
  String
    s;
  s << "near/" << distance;
  return s;
}

//
//  l  r  nextTo
//  -----------------------
//  0  0  0
//  0  b  0
//  0  x  0
//  a  0  0
//  a  b  near(a, b)
//  a  x  a
//  x  0  0
//  x  b  b
//  x  x  x
//
ResultList *
NearQuery::Evaluate ()
{
  ResultList *result = 0;
  Query *left = (Query *) operands[0];
  Query *right = (Query *) operands[1];

  if (left && right)
  {
    ResultList *l = left->GetResults ();
    if (l)
    {
      ResultList *r = right->GetResults ();
      if (r)
      {
        if (l->IsIgnore ())
        {
          result = new ResultList (*r);
        }
        else if (r->IsIgnore ())
        {
          result = new ResultList (*l);
        }
        else
        {
          result = Near (*l, *r);
        }
      }
    }
  }
  return result;
}

ResultList *
NearQuery::Near (const ResultList & l, const ResultList & r)
{
  ResultList *result = 0;
  DictionaryCursor c;
  l.Start_Get (c);
  DocMatch *match = (DocMatch *) l.Get_NextElement (c);
  while (match)
  {
    DocMatch *confirm = r.find (match->GetId ());
    if (confirm)
    {
      List *locations = MergeLocations (*match->GetLocations (),
                                        *confirm->GetLocations ());
      if (locations)
      {
        if (!result)
        {
          result = new ResultList;
        }
        DocMatch *copy = new DocMatch (*match);
        copy->SetLocations (locations);
        result->add (copy);
      }
    }
    match = (DocMatch *) l.Get_NextElement (c);
  }
  return result;
}

//
//: merge match positions in a 'near' operation
// all combinations are tested; the pairs of positions near enough are kept
//
List *
NearQuery::MergeLocations (const List & p, const List & q)
{
  List *result = 0;
  ListCursor pc;
  p.Start_Get (pc);
  const Location *left = (const Location *) p.Get_Next (pc);
  while (left)
  {
    ListCursor qc;
    q.Start_Get (qc);
    const Location *right = (const Location *) q.Get_Next (qc);
    while (right)
    {
      int dist = right->from - left->to;
      if (dist < 1)
      {
        dist = left->from - right->to;
        if (dist < 1)
        {
          dist = 0;
        }
      }
      if (unsigned (dist) <= distance)
      {
        if (!result)
        {
          result = new List;
        }
        result->Add (new Location (*left));
        result->Add (new Location (*right));
      }
      right = (const Location *) q.Get_Next (qc);
    }
    left = (const Location *) p.Get_Next (pc);
  }
  return result;
}
