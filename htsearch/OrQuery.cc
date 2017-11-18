// 
// OrQuery.cc
//
// OrQuery: an operator query that merges all the results of its operands
//          i.e. does 'or' combination
// 
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
// 
// $Id: OrQuery.cc,v 1.4 2004/05/28 13:15:24 lha Exp $
//


#include "OrQuery.h"
//
// return a ResultList containing an Or of the results of the operands
// evaluate all operands to do so
//
//  l  r  or
//  ---------------------
//  0  0  0
//  0  b  b
//  0  x  x
//  a  0  a
//  a  b  union(a,b)
//  a  x  a
//  x  0  x
//  x  b  b
//  x  x  x
//
// i.e. nulls and ignored are left out union
//
// Note that all operands are evaluated
// Ignored operands are not included in the operation
// the longer input result list is passed separately to Union
//

ResultList *
OrQuery::Evaluate()
{
  ResultList *result = 0;
  ResultList *longer = 0;
  List  shorter;
  int ignores = 0;
  operands.Start_Get();
  Query *operand = (Query *) operands.Get_Next();
  while(operand)
  {
    ResultList *next = operand->GetResults();
    if(next)
    {
      if(!next->IsIgnore())
      {
        if(!longer || longer->Count() < next->Count())
        {
          if(longer)
          {
            shorter.Add(longer);
          }
          longer = next;
        }
        else
        {
          shorter.Add(next);
        }
      }
      else
      {
        ignores++;
      }
    }
    operand = (Query *) operands.Get_Next();
  }
  if(longer)
  {
    result = Union(*longer, shorter);
    shorter.Release();
  }
  else if(ignores == operands.Count())
  {
    result = new ResultList;
    result->Ignore();
  }
  return result;
}

//
// copy unique DocMatches to the resulting list
// matches with the same docId are merged
// the longer list is assumed to be the first parameter
// this is a modest optimisation
//
ResultList *
OrQuery::Union(const ResultList &longer, const List &lists)
{
  ResultList *result = new ResultList(longer);

  ListCursor lc;  
  lists.Start_Get(lc);
  ResultList *current = (ResultList *) lists.Get_Next(lc);
  while(current)
  {
    DictionaryCursor c;
    current->Start_Get(c);
    DocMatch *match = (DocMatch *) current->Get_NextElement(c);
    while(match)
    {
      DocMatch *previous = result->find(match->GetId());
      if(previous)
      {
        previous->Merge(*match);
      }
      else
      {
        DocMatch *copy = new DocMatch(*match);
        result->add(copy);
      }
      match = (DocMatch *) current->Get_NextElement(c);
    }
    current = (ResultList *) lists.Get_Next(lc);
  }
  return result;
}
