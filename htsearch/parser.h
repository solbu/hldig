//
// parser.h
//
// $Id: parser.h,v 1.6 1999/08/28 21:14:28 ghutchis Exp $
//
#ifndef _parser_h_
#define _parser_h_

#include "htsearch.h"
#include "WeightWord.h"
#include "ResultList.h"
#include "DocMatch.h"
#include "Database.h"
#include "htString.h"
#include "Stack.h"
#include "WordList.h"
#include <ctype.h>

class Parser
{
public:
    Parser();
	
    int			checkSyntax(List *);
    void		parse(List *, ResultList &);

    void		setDatabase(char *db)		{words.Read(db);}
    char		*getErrorMessage()		{return error.get();}
    int			hadError()			{return valid == 0;}
	
protected:
    void		fullexpr(int);
    int			lexan();
    void		expr(int);
    void		term(int);
    void		factor(int);
    int			match(int);
    void		setError(char *);
    void		perform_push();
    void		perform_and(int);
    void		perform_or();

    List		*tokens;
    List		*result;
    WeightWord		*current;
    int			lookahead;
    int			valid;
    Stack		stack;
    String		error;

    WordList		words;
};


#endif


