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
// $Id: AndParseTree.cc,v 1.1.2.4 2000/08/29 13:57:21 ghutchis Exp $
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
  String	phrase, token;
  int		inPhrase = 0;
  int		currentToken = 0;
  ParseTree	*child;

  initialQuery = query;
  children = new List;

  token = WordToken(query, currentToken);
  while ( token.length() != 0 )
    {
      
      if (mystrcasecmp(token.get(), "\"") == 0)
	{
	  inPhrase = !inPhrase;
	  if (!inPhrase) // We just finished one...
	    {
	      child = new ExactParseTree;
	      child->Parse(phrase);
	      children->Add(child);
	      phrase = "";
	    }
	}
      else if (inPhrase)
	{
	  phrase << " " << token;
	}
      else if (!inPhrase)
	{
	  children->Add(new ParseTree(token));
	}
      else
	{
	  // Ignore (e.g. parentheses)
	}

      token = WordToken(query, currentToken);
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

  if (!children || children->Count() == 0)
    return initialQuery;

  children->Start_Get();
  child = (ParseTree *)children->Get_Next();
  if (child)
    logicalWords << child->GetLogicalWords();
  while ( (child = (ParseTree *)children->Get_Next()) )
    logicalWords << " and " << child->GetLogicalWords();

  return logicalWords;
}
