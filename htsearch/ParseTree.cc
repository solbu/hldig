//
// ParseTree.cc
//
// ParseTree: A general class to parse user queries and turn
//            them into fully parsed trees of WeightWord objects
//	      In particular, this class takes care of the most general case
//	      of parsing a boolean query
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: ParseTree.cc,v 1.1.2.2 2000/08/01 16:51:51 ghutchis Exp $
//

#include "ParseTree.h"
#include "Stack.h"
#include "AndParseTree.h"
#include "OrParseTree.h"
#include "ExactParseTree.h"
#include "HtWordType.h"

//*********************************************************************
// ParseTree::ParseTree()
//
ParseTree::ParseTree()
{
  children = NULL;
}


//*********************************************************************
// ParseTree::ParseTree(String word)
// In this case we are just a leaf with this particular string as data
//
ParseTree::ParseTree(String word)
{
  children = NULL;
  initialQuery = word;
  data.set(word);
}


//*********************************************************************
// ParseTree::~ParseTree()
//
ParseTree::~ParseTree()
{

}


//*********************************************************************
// void ParseTree::Release()
// Release the children
//
void ParseTree::Release()
{
  if (children)
    children->Release();
}


//*********************************************************************
// void ParseTree::Destroy()
// Destroy disconnects branches AND frees them
//
void ParseTree::Destroy()
{
  if (children)
    children->Destroy();
}


//*********************************************************************
// void ParseTree::Parse(String)
// Parse a query, returning OK or NOTOK depending on the correctness
// This version is the most complex--it must deal with arbitrary expressions
//
int ParseTree::Parse(String query)
{
  String	phrase;
  char		*token, *unparsed;
  int		inPhrase;
  ParseTree	*currentOp, *word;
  Stack		operators;	// a stack of ParseTree objects for parens

  initialQuery = query;
  unparsed = query.get();
  children = new List; // This will actually be *our* child but we need to build

  // We will build the ParseTree essentially through a bottom-up parse
  // And we will use the Adopt() method to attach children to new parents

  // We probably should have a separate tokenizer for this alone
  // that adds special characters to extra_word_chars and can be modified
  // for syntaxes like +word and title:word and * for all matches...

  inPhrase = 0;
  currentOp = NULL;
  word = NULL;
  token = HtWordToken(unparsed);
  while ( token != NULL )
    {
      // We switch between currentOp and word to indicate the state
      // i.e. have we just parsed an operator or a word
      // There's probably a better way to do this, but this seems easy
      //   (plus it makes it a bit more readable)

      if (mystrcasecmp(token, "(") == 0 && !inPhrase)
	{
	  if (currentOp != NULL && word == NULL)
	    {
	      operators.push(currentOp);
	      currentOp = NULL;
	    }
	  else if (currentOp == NULL && word == NULL) // Haven't parsed anything yet
	    operators.push(NULL);
	  else
	    return NOTOK; // We have a right parens in the wrong spot
	}

      else if (mystrcasecmp(token, ")") == 0 && !inPhrase)
	{
	  if (operators.Size() == 0)
	    return NOTOK; // Ooops, too many left parens!
	  if (currentOp == NULL && word != NULL && operators.peek() != NULL)
	    {
	      ((ParseTree *) operators.peek())->Adopt(word);
	      word = (ParseTree *) operators.pop();
	    }
	  else if (operators.peek() == NULL) // Pushed when we hadn't seen anything
	    operators.pop();
	  else if (currentOp == NULL && word == NULL) // Haven't parsed anything
	    {
	      word = (ParseTree *) operators.pop();
	    }
	  else
	    return NOTOK;
	}

      else if (mystrcasecmp(token, "\"") == 0)
	{
	  inPhrase = !inPhrase;
	  if (!inPhrase) // We just finished one...
	    {
	      word = new ExactParseTree;
	      word->Parse(phrase);
	      word->Adopt(currentOp);
	      currentOp = NULL;
	      phrase = "";
	    }
	  else if (inPhrase && word != NULL)
	    return NOTOK;
	}

      else if (mystrcasecmp(token,"and") == 0 && !inPhrase) // boolean_keywords[0]
	{
	  if (currentOp != NULL)
	    return NOTOK;
	  else if (word != NULL)
	    {
	      currentOp = new AndParseTree;
	      currentOp->Adopt(word);
	      word = NULL;
	    }
	  else
	    return NOTOK; // This is infix notation, so we need a first argument
	}

      else if (mystrcasecmp(token, "or") == 0 && !inPhrase) // boolean_keywords[1]
	{
	  if (currentOp != NULL)
	    return NOTOK;
	  else if (word != NULL)
	    {
	      currentOp = new OrParseTree;
	      currentOp->Adopt(word);
	      word = NULL;
	    }
	  else
	    return NOTOK; // This is infix notation, so we need a first argument
	}

      // NOT and NEAR keywords (or any others) would come here.
      // NOT could be a bit more complicated if we allowed single negation.

      else if (inPhrase)
	{
	  phrase << " " << token;
	}

      else
	{
	  if (word != NULL)
	    return NOTOK;
	  else if (currentOp != NULL)
	    {
	      word = new ParseTree(token);
	      currentOp->Adopt(word);
	      word = currentOp;
	      currentOp = NULL;
	    }
	  else // First word
	    word = new ParseTree(token);
	}
      
      token = HtWordToken(NULL);
    } // end while

  if (inPhrase)		// Mismatched "" marks
    return NOTOK;
  else if (operators.Size() != 0)	// Left an operator still on the stack!
    return NOTOK;
  else if (currentOp != NULL)	// Looking for an ending word
    return NOTOK;

  // Cool, we can just adopt word, which should be the top node!
  Adopt(word);
  return OK;
}


//*********************************************************************
// void ParseTree::Parse(StringList *list)
// Parse a query, returning OK or NOTOK depending on the correctness
// (We're assuming that the list passed in is split up correctly too)
// This version is pretty easy and is probably going to be inherited by
//  most subclasses--an individual node doesn't need to know what type it is
//
int ParseTree::Parse(StringList *list)
{
  String	*word;

  if (initialQuery.length() == 0)
    initialQuery = list->Join(' ');
  
  children = new List;
  list->Start_Get();
  while ( (word = (String *)list->Get_Next()) )
    {
      if (word)
	children->Add(new ParseTree(*word));
    }

  if (children->Count() != 0)
    return OK;
  else
    {
      delete children;
      children = NULL;
      return NOTOK;
    }
}


//*********************************************************************
// void ParseTree::GetResults()
// Combine result lists (if present) according to our specific
// operator type (e.g. AND, OR, NOT, NEAR, etc.)
//
void ParseTree::GetResults()
{
  // The base class has at most one child, so just use those results
  // It really serves as a set of parentheses
  if (children)
    results = ((ParseTree *) children->Get_First())->results;
}

//*********************************************************************
// void ParseTree::Adopt(ParseTree *child)
// Adopt this ParseTree as one of our children
// Alows us to build a tree through a bottom-up parse
//
void ParseTree::Adopt(ParseTree *child)
{
  if (!child)
    return;

  if (children)
    children->Add(child);
  else
    {
      children = new List;
      children->Add(child);
    }

  // I don't know a better way of doing this right now -GRH 6/00
  // ideally we'd want to update initialQuery while building bottom-up
  initialQuery = GetLogicalWords();
}


//*********************************************************************
// void ParseTree::Fuzzy(List fuzzies)
// If passed a list of Fuzzy methods, use them to fill out the tree
// (note that some subclasses may choose to ignore this if desired)
//
void ParseTree::Fuzzy(List fuzzies)
{
  ParseTree	*child;

  // All we have to do is pass it along. We're never a leaf by convention
  if (children)
    {
      children->Start_Get();
      while ( (child = (ParseTree *)children->Get_Next()) )
	child->Fuzzy(fuzzies);
    }
  else // Our subclasses don't have to worry about this... Lazy children! ;-)
    {
    }
}


//*********************************************************************
// String	ParseTree::GetLogicalWords()
//
String	ParseTree::GetLogicalWords()
{
  String	logicalWords;

  if (!children)
    return initialQuery;

  logicalWords << "(" << ((ParseTree *) children->Get_First())->GetLogicalWords();
  logicalWords << ")";

  return logicalWords;
}
