//
// parser.h
//
// $Id: parser.h,v 1.3.2.1 1999/02/17 05:03:05 ghutchis Exp $
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
#include <ctype.h>

class Parser
{
public:
    Parser();
	
    int			checkSyntax(List *);
    void		parse(List *, ResultList &);

    void		setDatabase(Database *db)	{dbf = db;}
    char		*getErrorMessage()		{return error.get();}
    int			hadError()			{return valid == 0;}
	
protected:
    void		fullexpr(int);
    int			lexan();
    void		expr(int);
    void		term(int);
    void		factor(int);
    int			match(int);
    void		perform_push();
    void		perform_and(int);
    void		perform_or();

    List		*tokens;
    List		*result;
    WeightWord		*current;
    int			lookahead;
    int			valid;
    Stack		stack;
    Database		*dbf;
    String		error;
};


#endif


