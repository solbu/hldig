//
// parser.cc
//
// parser: Parses a boolean expression tree, retrieving and scoring 
//         the resulting document list
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: parser.cc,v 1.22.2.4 2000/05/06 20:46:41 loic Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "parser.h"
#include "HtPack.h"
#include "Collection.h"

#define	WORD	1000
#define	DONE	1001


//*****************************************************************************
Parser::Parser() :
  words(config)
{
    tokens = 0;
    result = 0;
    current = 0;
    valid = 1;
}


//*****************************************************************************
// int Parser::checkSyntax(List *tokenList)
//   As the name of the function implies, we will only perform a syntax check
//   on the list of tokens.
//
int
Parser::checkSyntax(List *tokenList)
{
    tokens = tokenList;
    valid = 1;
    fullexpr(0);
    return valid;
}

//*****************************************************************************
void
Parser::fullexpr(int output)
{
    tokens->Start_Get();
    lookahead = lexan();
    expr(output);
    if (valid && lookahead != DONE)
    {
	setError("end of expression");
    }
}

//*****************************************************************************
int
Parser::lexan()
{
    current = (WeightWord *) tokens->Get_Next();
    if (!current)
	return DONE;
    else if (mystrcasecmp((char*)current->word, "&") == 0)
	return '&';
    else if (mystrcasecmp((char*)current->word, "|") == 0)
	return '|';
    else if (mystrcasecmp((char*)current->word, "!") == 0)
	return '!';
    else if (mystrcasecmp((char*)current->word, "(") == 0)
	return '(';
    else if (mystrcasecmp((char*)current->word, ")") == 0)
	return ')';
    else if (mystrcasecmp((char*)current->word, "\"") == 0)
      return '"';
    else
	return WORD;
}

//*****************************************************************************
//   Attempt to deal with expressions in the form
//		term | term | term ...
//
void
Parser::expr(int output)
{
    term(output);
    while (1)
    {
	if (match('|'))
	{
	    term(output);
	    if (output)
	    {
		perform_or();
	    }
	}
	else
	    break;
    }
    if (valid && lookahead == WORD)
    {
	setError("'AND' or 'OR'");
    }
}

//*****************************************************************************
void
Parser::term(int output)
{
    int	isand;
    
    factor(output);
    while (1)
    {
	if ((isand = match('&')) || match('!'))
	{
	    factor(output);
	    if (output)
	    {
		perform_and(isand);
	    }
	}
	else
	    break;
    }
}

//*****************************************************************************
void
Parser::factor(int output)
{
    phrase(output);

    if (match('('))
    {
	expr(output);
	if (match(')'))
	{
	    return;
	}
	else
	{
	    setError("')'");
	}
    }
    else if (lookahead == WORD)
    {
	if (output)
	{
	    perform_push();
	}
	lookahead = lexan();
    }
    //    else
    //    {
    //	setError("a search word");
    //    }
}

//*****************************************************************************
void
Parser::phrase(int output)
{
  if (match('"'))
    {
      List *wordList = new List;
      double weight = 1.0;

      while (1)
	{
	  if (match('"'))
	    {
	      if (output)
		score(wordList, weight);
	      break;
	    }
	  else if (lookahead == WORD)
	    {
	      weight *= current->weight;
	      if (output)
		perform_phrase(*wordList);
	      
	      lookahead = lexan();
	    }

	} // end while
      delete wordList;
    } // end if
}

//*****************************************************************************
int
Parser::match(int t)
{
    if (lookahead == t)
    {
	lookahead = lexan();
	return 1;
    }
    else
	return 0;
}

//*****************************************************************************
void
Parser::setError(char *expected)
{
    if (valid)
    {
	valid = 0;
	error = 0;
	error << "Expected " << expected;
	if (lookahead == DONE || !current)
	{
	    error << " at the end";
	}
	else
	{
	    error << " instead of '" << current->word.get();
	    error << '\'';
	    switch (lookahead)
	    {
	    case '&':	error << " or 'AND'";	break;
	    case '|':	error << " or 'OR'";	break;
	    case '!':	error << " or 'NOT'";	break;
	    }
	}
    }
}

//*****************************************************************************
// Perform a lookup of the current word and push the result onto the stack
//
void
Parser::perform_push()
{
    static int	maximum_word_length = config.Value("maximum_word_length", 12);
    String	temp = current->word.get();
    char	*p;

    if (current->isIgnore)
    {
	//
	// This word needs to be ignored.  Make it so.
	//
	return;
    }

    temp.lowercase();
    p = temp.get();
    if (temp.length() > maximum_word_length)
	p[maximum_word_length] = '\0';

    List* result = words[p];
    score(result, current->weight);
    delete result;
}

//*****************************************************************************
void
Parser::perform_phrase(List &oldWords)
{
    static int	maximum_word_length = config.Value("maximum_word_length", 12);
    String	temp = current->word.get();
    char	*p;
    List	*newWords = 0;
    HtWordReference *oldWord, *newWord;

    if (current->isIgnore)
    {
	//
	// This word needs to be ignored.  Make it so.
	//
	return;
    }

    temp.lowercase();
    p = temp.get();
    if (temp.length() > maximum_word_length)
	p[maximum_word_length] = '\0';

    newWords = words[p];

    // If we don't have a prior list of words, we want this one...
    if (oldWords.Count() == 0)
      {
	newWords->Start_Get();
	while ((newWord = (HtWordReference *) newWords->Get_Next()))
	  oldWords.Add(newWord);
	return;
      }

    // OK, now we have a previous list in wordList and a new list
    List	*results = new List;

    oldWords.Start_Get();
    while ((oldWord = (HtWordReference *) oldWords.Get_Next()))
      {
	newWords->Start_Get();
	while ((newWord = (HtWordReference *) newWords->Get_Next()))
	  {
	    if (oldWord->DocID() == newWord->DocID())
	      if ((oldWord->Location() + 1) == newWord->Location())
		{
		  HtWordReference *result = new HtWordReference(*oldWord);

		  result->Flags(oldWord->Flags() & newWord->Flags());
		  result->Location(newWord->Location());
		  
		  results->Add(result);
		}
	  }
      }

    oldWords.Destroy();
    results->Start_Get();
    while ((newWord = (HtWordReference *) results->Get_Next()))
      oldWords.Add(newWord);
    results->Release();
    delete results;

    newWords->Destroy();
    delete newWords;
}

//*****************************************************************************
void
Parser::score(List *wordList, double weight)
{
    ResultList	*list = new ResultList;
    DocMatch	*dm;
    HtWordReference *wr;
    static double text_factor = config.Double("text_factor", 1);
    static double caps_factor = config.Double("caps_factor", 1);
    static double title_factor = config.Double("title_factor", 1);
    static double heading_factor = config.Double("heading_factor", 1);
    static double keywords_factor = config.Double("keywords_factor", 1);
    static double meta_description_factor = config.Double("meta_description_factor", 1);
    static double author_factor = config.Double("author_factor", 1);
    static double description_factor = config.Double("description_factor", 1);
    double	  wscore;
    int		  docanchor;
    int		  word_count;

    stack.push(list);

    if (!wordList || wordList->Count() == 0)
      {
	// We can't score an empty list, so this should be ignored...
	list->isIgnore = 1;
	return;
      }

    // We're now guaranteed to have a non-empty list
    // We'll use the number of occurences of this word for scoring
    word_count = wordList->Count();

    wordList->Start_Get();
    while ((wr = (HtWordReference *) wordList->Get_Next()))
      {
	//
	// *******  Compute the score for the document
	//
	wscore = 0.0;
	if (wr->Flags() == FLAG_TEXT)		wscore += text_factor;
	if (wr->Flags() & FLAG_CAPITAL)		wscore += caps_factor;
	if (wr->Flags() & FLAG_TITLE)		wscore += title_factor;
	if (wr->Flags() & FLAG_HEADING)		wscore += heading_factor;
	if (wr->Flags() & FLAG_KEYWORDS)	wscore += keywords_factor;
	if (wr->Flags() & FLAG_DESCRIPTION)	wscore += meta_description_factor;
	if (wr->Flags() & FLAG_AUTHOR)		wscore += author_factor;
	if (wr->Flags() & FLAG_LINK_TEXT)	wscore += description_factor;
	wscore *= weight;
	wscore = wscore / (double)word_count;
	docanchor = wr->Anchor();
	dm = list->find(wr->DocID());
	if (dm)
	  {
	    wscore += dm->score;
	    if (dm->anchor < docanchor)
		docanchor = dm->anchor;
	    // We wish to *update* this, not add a duplicate
	    list->remove(wr->DocID());
	  }

	dm = new DocMatch;
	dm->id = wr->DocID();
	dm->score = wscore;
	dm->anchor = docanchor;
	list->add(dm);
      }
}


//*****************************************************************************
// The top two entries in the stack need to be ANDed together.
//
void
Parser::perform_and(int isand)
{
    ResultList		*l1 = (ResultList *) stack.pop();
    ResultList		*l2 = (ResultList *) stack.pop();
    ResultList		*result = new ResultList;
    int			i;
    DocMatch		*dm, *dm2, *dm3;
    HtVector		*elements;

    //
    // If either of the arguments is not present, we will use the other as
    // the result.
    //
    if (!l1 && l2)
    {
	stack.push(l2);
	return;
    }
    else if (l1 && !l2)
    {
	stack.push(l1);
	return;
    }
    else if (!l1 && !l2)
    {
	stack.push(result);
	return;
    }
    
    //
    // If either of the arguments is set to be ignored, we will use the
    // other as the result.
    //
    if (l1->isIgnore)
    {
	stack.push(l2);
	delete l1;
	return;
    }
    else if (l2->isIgnore)
    {
	stack.push(isand ? l1 : result);
	delete l2;
	return;
    }
    
    stack.push(result);
    elements = l2->elements();
    for (i = 0; i < elements->Count(); i++)
    {
	dm = (DocMatch *) (*elements)[i];
	dm2 = l1->find(dm->id);
	if (dm2 ? isand : (isand == 0))
	{
	    //
	    // Duplicate document.  We just need to add the scored together.
	    //
	    dm3 = new DocMatch;
	    dm3->score = dm->score + (dm2 ? dm2->score : 0);
	    dm3->id = dm->id;
	    dm3->anchor = dm->anchor;
	    if (dm2 && dm2->anchor < dm3->anchor)
		dm3->anchor = dm2->anchor;
	    result->add(dm3);
	}
    }
    elements->Release();
    delete elements;
    delete l1;
    delete l2;
}

//*****************************************************************************
// The top two entries in the stack need to be ORed together.
//
void
Parser::perform_or()
{
    ResultList		*l1 = (ResultList *) stack.pop();
    ResultList		*result = (ResultList *) stack.peek();
    int			i;
    DocMatch		*dm, *dm2;
    HtVector		*elements;

    //
    // If either of the arguments is not present, we will use the other as
    // the results.
    //
    if (!l1 && result)
    {
	return;
    }
    else if (l1 && !result)
    {
	stack.push(l1);
	return;
    }
    else if (!l1 & !result)
    {
	stack.push(new ResultList);
	return;
    }
    
    //
    // If either of the arguments is set to be ignored, we will use the
    // other as the result.
    //
    if (l1->isIgnore)
    {
	delete l1;
	return;
    }
    else if (result->isIgnore)
    {
	result = (ResultList *) stack.pop();
	stack.push(l1);
	delete result;
	return;
    }
    
    elements = l1->elements();
    for (i = 0; i < elements->Count(); i++)
    {
	dm = (DocMatch *) (*elements)[i];
	dm2 = result->find(dm->id);
	if (dm2)
	{
	    //
	    // Duplicate document.  We just need to add the scores together
	    //
	    dm2->score += dm->score;
	    if (dm->anchor < dm2->anchor)
		dm2->anchor = dm->anchor;
	}
	else
	{
	    dm2 = new DocMatch;
	    dm2->score = dm->score;
	    dm2->id = dm->id;
	    dm2->anchor = dm->anchor;
	    result->add(dm2);
	}
    }
    elements->Release();
    delete elements;
    delete l1;
}

//*****************************************************************************
// void Parser::parse(List *tokenList, ResultList &resultMatches)
//
void
Parser::parse(List *tokenList, ResultList &resultMatches)
{
    tokens = tokenList;
    fullexpr(1);

    ResultList	*result = (ResultList *) stack.pop();
    if (!result)  // Ouch!
      {
	valid = 0;
	error = 0;
	error << "Expected to have something to parse!";
	return;
      }
    HtVector	*elements = result->elements();
    DocMatch	*dm;

    for (int i = 0; i < elements->Count(); i++)
    {
	dm = (DocMatch *) (*elements)[i];
        dm->collection = collection; // back reference
	resultMatches.add(dm);
    }
    elements->Release();
    result->Release();
    delete elements;
    delete result;
}

void
Parser::setCollection(Collection *coll)
{
    if (coll)
        words.Open(coll->getWordFile(), O_RDONLY);
    collection = coll;
}       

