//
// AndParseTree.cc
//
// AndParseTree: A class to parse queries of words to be merged together
//               to form an intersection (AND) of the matching documents.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: AndParseTree.cc,v 1.1.2.3 2000/08/24 04:36:32 ghutchis Exp $
//

#include "AndParseTree.h"
#include "HtWordType.h"
#include "ExactParseTree.h"

//*********************************************************************
// AndParseTree::AndParseTree()
//
AndParseTree::AndParseTree()
{
}


//*********************************************************************
// AndParseTree::AndParseTree(String word)
// In this case we are just a leaf with this particular string as data
//
AndParseTree::AndParseTree(String word)
{
}


//*********************************************************************
// AndParseTree::~AndParseTree()
//
AndParseTree::~AndParseTree()
{
}


//*********************************************************************
// void AndParseTree::Parse(String)
// Parse a query, returning OK or NOTOK depending on the correctness
// 
int AndParseTree::Parse(String query)
{
  char		*word;
  int		phraseStart, phraseEnd;
  ParseTree	*child;

  initialQuery = query;
  children = new List;

  phraseStart = query.indexOf('"');
  while ( phraseStart != -1 )
    {
      if (phraseStart > 0)
	{
	  child = new AndParseTree;
	  child->Parse(query.sub(0, phraseStart - 1));
	  children->Add(child);
	}

      // Now get the phrase query and potentially anything after it
      phraseEnd = query.indexOf('"', phraseStart + 1);
      if (phraseEnd <= query.length() && phraseEnd != -1)
	{
	  child = new ExactParseTree;
	  child->Parse(query.sub(phraseStart + 1, (phraseEnd - phraseStart - 1)));
	  children->Add(child);
	  query = query.sub(phraseEnd + 1);
	}
      else
	  return NOTOK;

      // need to gobble up whitespace here in case we have multiple phrases together
      phraseStart = query.indexOf('"');
    }

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
// void AndParseTree::GetResults()
// Combine the children (if present) according to our specific
// operator type (e.g. AND, OR, NOT, NEAR, etc.)
//
void AndParseTree::GetResults()
{
  // We will eventually do an intersection of the children to produce our list
  // Until then, this does nothing
}


//*********************************************************************
// void AndParseTree::Fuzzy(List fuzzies)
// If passed a list of Fuzzy methods, use them to fill out the tree
// (note that some subclasses may choose to ignore this if desired)
//
void AndParseTree::Fuzzy(List fuzzies)
{
  ParseTree	*child;

  // All we have to do is pass it along. We're never a leaf by convention
  if (children)
    {
      children->Start_Get();
      while ( (child = (ParseTree *)children->Get_Next()) )
	child->Fuzzy(fuzzies);
    }
}


//*********************************************************************
// String	AndParseTree::GetLogicalWords()
//
String	AndParseTree::GetLogicalWords()
{
  String	logicalWords;
  ParseTree	*child;

  if (!children)
    return initialQuery;

  children->Start_Get();
  child = (ParseTree *)children->Get_Next();
  if (child)
    logicalWords << child->GetLogicalWords();
  while ( (child = (ParseTree *)children->Get_Next()) )
    logicalWords << " and " << child->GetLogicalWords();

  return logicalWords;
}
