//
// parser.h
//
// $Id: parser.h,v 1.8 1999/08/29 09:04:02 ghutchis Exp $
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
    void		phrase(int);
    void		expr(int);
    void		term(int);
    void		factor(int);
    int			match(int);
    void		setError(char *);
    void		perform_push();
    void		perform_and(int);
    void		perform_or();
    void		perform_phrase(List &);

    void		score(List *, double weight);

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


