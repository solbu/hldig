//
// parser.cc
//
// Implementation of parser
//
//
#if RELEASE
static char RCSid[] = "$Id: parser.cc,v 1.6.2.6 2002/01/18 21:59:47 grdetil Exp $";
#endif

#include "parser.h"
#include "QuotedStringList.h"

#define	WORD	1000
#define	DONE	1001

extern StringList	boolean_keywords;
QuotedStringList	boolean_syntax_errors;


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
//   As the name of the function implies, we will only perform a syntax check
//   on the list of tokens.
//
int
Parser::checkSyntax(List *tokenList)
{
    void	reportError(char *);
    // Load boolean_syntax_errors from configuration
    // they should be placed in this order:
    // 0        1               2            3            4
    // Expected "a search word" "at the end" "instead of" "end of expression"
    boolean_syntax_errors.Create(config["boolean_syntax_errors"], "| \t\r\n\001");
    if (boolean_syntax_errors.Count() != 5)
	reportError("boolean_syntax_errors attribute is not correctly specified");
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
	setError(boolean_syntax_errors[4]);
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
    if (valid && lookahead == WORD)
    {
	String	expected = "'";
	expected << boolean_keywords[0] << "' " << boolean_keywords[1] << " '"
		 << boolean_keywords[1] << "'";
	setError(expected.get());
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
    else
    {
	setError(boolean_syntax_errors[1]);
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
void
Parser::setError(char *expected)
{
    if (valid)
    {
	valid = 0;
	error = 0;
	error << boolean_syntax_errors[0] << ' ' << expected;
	if (lookahead == DONE || !current)
	{
	    error << ' ' << boolean_syntax_errors[2];
	}
	else
	{
	    error << ' ' << boolean_syntax_errors[3] << " '"
		  << current->word.get() << "'";
	    switch (lookahead)
	    {
	    case '&':	error << ' ' << boolean_keywords[1] << " '"
			      << boolean_keywords[0] << "'";
			break;
	    case '|':	error << ' ' << boolean_keywords[1] << " '"
			      << boolean_keywords[1] << "'";
			break;
	    case '!':	error << ' ' << boolean_keywords[1] << " '"
			      << boolean_keywords[2] << "'";
			break;
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
    static char	*wildcard = config["prefix_match_character"];
    if (*wildcard == '\0')
	wildcard = "*";
    if (strcmp(temp.get(), wildcard) == 0) {
	Database	*docIndex = Database::getDatabaseInstance();
	docIndex->OpenRead(config["doc_index"]);
	docIndex->Start_Get();
	while ((p = docIndex->Get_Next()))
	{
	    dm = new DocMatch;
	    dm->score = current->weight;
	    dm->id = atoi(p);
	    dm->anchor = 0;
	    list->add(dm);
	}
	delete docIndex;
	return;
    }
    temp.lowercase();
    p = temp.get();
    if (temp.length() > maximum_word_length)
	p[maximum_word_length] = '\0';
    if (dbf->Get(p, data) == OK)
    {
	p = data.get();
	for (unsigned int i = 0; i < data.length() / sizeof(WordRecord); i++)
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
    static int	multimatch_factor = config.Value("multimatch_factor", 1);
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
	    // Boost matches that contain more than one word...
	    dm2->score *= multimatch_factor;
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
	if (!valid)
	    return;
	valid = 0;
	error = 0;
	error << boolean_syntax_errors[0] << ' ' << boolean_syntax_errors[1];
	return;
      }
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
