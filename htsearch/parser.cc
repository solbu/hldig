//
// parser.cc
//
// Implementation of parser
//
//
#if RELEASE
static char RCSid[] = "$Id: parser.cc,v 1.15 1999/08/29 09:04:21 ghutchis Exp $";
#endif

#include "parser.h"
#include "HtPack.h"

#define	WORD	1000
#define	DONE	1001


//*****************************************************************************
Parser::Parser()
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
    else if (mystrcasecmp(current->word, "&") == 0)
	return '&';
    else if (mystrcasecmp(current->word, "|") == 0)
	return '|';
    else if (mystrcasecmp(current->word, "!") == 0)
	return '!';
    else if (mystrcasecmp(current->word, "(") == 0)
	return '(';
    else if (mystrcasecmp(current->word, ")") == 0)
	return ')';
    else if (mystrcasecmp(current->word, "\"") == 0)
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
  List *wordList = new List;
  double weight = 1.0;

  if (match('"'))
    {
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

    score(words[p], current->weight);
}

//*****************************************************************************
void
Parser::perform_phrase(List &oldWords)
{
    static int	maximum_word_length = config.Value("maximum_word_length", 12);
    String	temp = current->word.get();
    char	*p;
    List	*newWords = 0;
    WordReference *oldWord, *newWord;

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
	while ((newWord = (WordReference *) newWords->Get_Next()))
	  oldWords.Add(newWord);
	return;
      }

    // OK, now we have a previous list in wordList and a new list
    List	*results = new List;

    oldWords.Start_Get();
    while ((oldWord = (WordReference *) oldWords.Get_Next()))
      {
	newWords->Start_Get();
	while ((newWord = (WordReference *) newWords->Get_Next()))
	  {
	    if (oldWord->DocumentID == newWord->DocumentID)
	      if ((oldWord->Location + 1) == newWord->Location)
		{
		  WordReference *result = new WordReference;
		  result->DocumentID = oldWord->DocumentID;
		  result->Location = newWord->Location;	      
		  
		  result->Flags = oldWord->Flags & newWord->Flags;
		  result->Anchor = oldWord->Anchor;
		  
		  results->Add(result);
		}
	  }
      }

    oldWords.Destroy();
    results->Start_Get();
    while ((newWord = (WordReference *) results->Get_Next()))
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
    WordReference *wr;

    stack.push(list);

    if (!wordList || wordList->Count() == 0)
      {
	// We can't score an empty list, so this should be ignored...
	list->isIgnore = 1;
	return;
      }

    wordList->Start_Get();
    while ((wr = (WordReference *) wordList->Get_Next()))
      {
	dm = list->find(wr->DocumentID);
	if (dm)
	  {

	    int prevAnchor;
	    double prevScore;
	    prevScore = dm->score;
	    prevAnchor = dm->anchor;
	    // We wish to *update* this, not add a duplicate
	    list->remove(wr->DocumentID);

	    dm = new DocMatch;

	    dm->score = (wr->Flags & FLAG_TEXT) * config.Value("text_factor", 1);
	    dm->score += (wr->Flags & FLAG_CAPITAL) * config.Value("caps_factor", 1);
	    dm->score += (wr->Flags & FLAG_TITLE) * config.Value("title_factor", 1);
	    dm->score += (wr->Flags & FLAG_HEADING) * config.Value("heading_factor", 1);
	    dm->score += (wr->Flags & FLAG_KEYWORDS) * config.Value("keywords_factor", 1);
	    dm->score += (wr->Flags & FLAG_DESCRIPTION) * config.Value("meta_description_factor", 1);
	    dm->score += (wr->Flags & FLAG_AUTHOR) * config.Value("author_factor", 1);
	    dm->score += (wr->Flags & FLAG_LINK_TEXT) * config.Value("description_factor", 1);
	    dm->score = weight * dm->score + prevScore;
	    if (prevAnchor > wr->Anchor)
	      dm->anchor = wr->Anchor;
	    else
	      dm->anchor = prevAnchor;
	    
	  }
	else
	  {

	    //
	    // *******  Compute the score for the document
	    //
	    dm = new DocMatch;
	    dm->score = (wr->Flags & FLAG_TEXT) * config.Value("text_factor", 1);
	    dm->score += (wr->Flags & FLAG_CAPITAL) * config.Value("caps_factor", 1);
	    dm->score += (wr->Flags & FLAG_TITLE) * config.Value("title_factor", 1);
	    dm->score += (wr->Flags & FLAG_HEADING) * config.Value("heading_factor", 1);
	    dm->score += (wr->Flags & FLAG_KEYWORDS) * config.Value("keywords_factor", 1);
	    dm->score += (wr->Flags & FLAG_DESCRIPTION) * config.Value("meta_description_factor", 1);
	    dm->score += (wr->Flags & FLAG_AUTHOR) * config.Value("author_factor", 1);
	    dm->score += (wr->Flags & FLAG_LINK_TEXT) * config.Value("description_factor", 1);
	    dm->score *= weight;
	    dm->id = wr->DocumentID;
	    dm->anchor = wr->Anchor;
	  }
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
	resultMatches.add(dm);
    }
    elements->Release();
    result->Release();
    delete elements;
    delete result;
}
