//
// parser.cc
//
// Implementation of parser
//
// $Log: parser.cc,v $
// Revision 1.2  1997/04/27 14:43:31  turtle
// changes
//
// Revision 1.1.1.1  1997/02/03 17:11:05  turtle
// Initial CVS
//
//
#if RELEASE
static char RCSid[] = "$Id: parser.cc,v 1.2 1997/04/27 14:43:31 turtle Exp $";
#endif

#include "parser.h"

#define	WORD	1000
#define	DONE	1001


//*****************************************************************************
Parser::Parser()
{
    tokens = 0;
    result = 0;
    current = 0;
    dbf = 0;
    valid = 1;
}


//*****************************************************************************
// int Parser::checkSyntax(List *tokenList, Database *dbf)
//   As the name of the function implies, we will only perform a syntex check
//   on the list of tokens.
//
int
Parser::checkSyntax(List *tokenList)
{
    tokens = tokenList;
    tokens->Start_Get();
    lookahead = lexan();
    valid = 1;
    expr(0);
    return valid;
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
//	else if (mystrcasecmp(current->word, "and") == 0)
//		return '&';
    else if (mystrcasecmp(current->word, "|") == 0)
	return '|';
    else if (mystrcasecmp(current->word, "!") == 0)
	return '!';
//	else if (mystrcasecmp(current->word, "or") == 0)
//		return '|';
    else if (mystrcasecmp(current->word, "(") == 0)
	return '(';
    else if (mystrcasecmp(current->word, ")") == 0)
	return ')';
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
    if (lookahead == WORD)
    {
	valid = 0;
	error = 0;
	error << "expected 'AND' or 'OR' instead of '" << current->word.get();
	error << '\'';
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
    if (match('('))
    {
	expr(output);
	if (match(')'))
	{
	    return;
	}
	else
	{
	    valid = 0;
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
    else
    {
	valid = 0;
    }
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
// Perform a lookup of the current word and push the result onto the stack
//
void
Parser::perform_push()
{
    String	temp = current->word.get();
    String	data;
    char	*p;
    ResultList	*list = new ResultList;
    WordRecord	wr;
    DocMatch	*dm;

    stack.push(list);
    if (current->isIgnore)
    {
	//
	// This word needs to be ignored.  Make it so.
	//
	list->isIgnore = 1;
	return;
    }
    temp.lowercase();
    p = temp.get();
    if (temp.length() > MAX_WORD_LENGTH)
	p[MAX_WORD_LENGTH] = '\0';
    if (dbf->Get(p, data) == OK)
    {
	p = data.get();
	for (int i = 0; i < data.length() / sizeof(WordRecord); i++)
	{
	    p = data.get() + i * sizeof(WordRecord);
	    memcpy((char *) &wr, p, sizeof(WordRecord));

	    //
	    // *******  Compute the score for the document
	    //
	    dm = new DocMatch;
	    dm->score = wr.weight * current->weight;
	    dm->id = wr.id;
	    dm->anchor = wr.anchor;
	    list->add(dm);
	}
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
    List		*elements;

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
    List		*elements;

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
    tokens->Start_Get();
    lookahead = lexan();
    expr(1);

    ResultList	*result = (ResultList *) stack.pop();
    List		*elements = result->elements();
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
