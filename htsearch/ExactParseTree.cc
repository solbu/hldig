//
// ExactParseTree.cc
//
// ExactParseTree: A class to parse phrase-match queries
//		   to form an intersection (and filtering) of results
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: ExactParseTree.cc,v 1.1.2.3 2000/08/29 13:57:34 ghutchis Exp $
//

#include "ExactParseTree.h"
#include "HtWordType.h"

//*********************************************************************
// ExactParseTree::ExactParseTree()
//
ExactParseTree::ExactParseTree()
{
}


//*********************************************************************
// ExactParseTree::ExactParseTree(String word)
// In this case we are just a leaf with this particular string as data
//
ExactParseTree::ExactParseTree(String word)
{
}


//*********************************************************************
// ExactParseTree::~ExactParseTree()
//
ExactParseTree::~ExactParseTree()
{
}


//*********************************************************************
// void ExactParseTree::Parse(String)
// Parse a query, returning OK or NOTOK depending on the correctness
// 
int ExactParseTree::Parse(String query)
{
  char	*word;

  initialQuery = query;
  children = new List;

  word = HtWordToken(query);
  while ( word != NULL )
    {
      // By convention, leaves should be plain ParseTrees
      children->Add(new ParseTree(word));
      word = HtWordToken(NULL);
    }

  return OK;
}


//*********************************************************************
// void ExactParseTree::GetResults()
// Combine the children (if present) according to our specific
// operator type (e.g. AND, OR, NOT, NEAR, etc.)
//
void ExactParseTree::GetResults()
{
  // We will eventually do an intersection of the children to produce our list
  // Until then, this does nothing
}


//*********************************************************************
// void ExactParseTree::Fuzzy(List fuzzies)
// If passed a list of Fuzzy methods, use them to fill out the tree
// (note that some subclasses may choose to ignore this if desired)
//
void ExactParseTree::Fuzzy(List fuzzies)
{
  // This class should not produce fuzzy alternatives
  // Exact, after all means exact!
}


//*********************************************************************
// String	ExactParseTree::GetLogicalWords()
//
String	ExactParseTree::GetLogicalWords()
{
  String	logicalWords;
  ParseTree	*child;

  if (!children || children->Count() == 0)
    return initialQuery;

  logicalWords << '"';
  children->Start_Get();
  child = (ParseTree *)children->Get_Next();
  logicalWords << child->GetLogicalWords();
  while ( (child = (ParseTree *)children->Get_Next()) )
    logicalWords << " " << child->GetLogicalWords();

  logicalWords << '"';

  return logicalWords;
}
