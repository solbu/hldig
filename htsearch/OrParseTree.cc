//
// OrParseTree.cc
//
// OrParseTree: A class to parse queries of words to be merged together
//               to form a union (OR) of the matching documents.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: OrParseTree.cc,v 1.1.2.3 2000/08/24 04:36:32 ghutchis Exp $
//

#include "OrParseTree.h"
#include "HtWordType.h"
#include "ExactParseTree.h"

//*********************************************************************
// OrParseTree::OrParseTree()
//
OrParseTree::OrParseTree()
{
  children = NULL;
}


//*********************************************************************
// OrParseTree::OrParseTree(String word)
// In this case we are just a leaf with this particular string as data
//
OrParseTree::OrParseTree(String word)
{
  children = NULL;
  initialQuery = word;
  data.set(word);
}


//*********************************************************************
// OrParseTree::~OrParseTree()
//
OrParseTree::~OrParseTree()
{

}


//*********************************************************************
// void OrParseTree::Parse(String)
// Parse a query, returning OK or NOTOK depending on the correctness
// 
int OrParseTree::Parse(String query)
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
	  child = new OrParseTree;
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
// void OrParseTree::GetResults()
// Combine the children (if present) according to our specific
// operator type (e.g. AND, OR, NOT, NEAR, etc.)
//
void OrParseTree::GetResults()
{
  // We will eventually do a union of the children to produce our list
  // (Unfortunately, we have to tally duplicates, so it does take time.)
  // Until then, this does nothing
}


//*********************************************************************
// void OrParseTree::Fuzzy(List fuzzies)
// If passed a list of Fuzzy methods, use them to fill out the tree
// (note that some subclasses may choose to ignore this if desired)
//
void OrParseTree::Fuzzy(List fuzzies)
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
// String	OrParseTree::GetLogicalWords()
//
String	OrParseTree::GetLogicalWords()
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
    logicalWords << " or " << child->GetLogicalWords();

  return logicalWords;
}
