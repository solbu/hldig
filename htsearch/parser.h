//
// parser.h
//
// $Id: parser.h,v 1.3 1997/04/27 14:43:31 turtle Exp $
//
// $Log: parser.h,v $
// Revision 1.3  1997/04/27 14:43:31  turtle
// changes
//
// Revision 1.2  1997/03/24 04:33:25  turtle
// Renamed the String.h file to htString.h to help compiling under win32
//
// Revision 1.1.1.1  1997/02/03 17:11:05  turtle
// Initial CVS
//
//
#ifndef _parser_h_
#define _parser_h_

#include "htsearch.h"
#include "WeightWord.h"
#include "ResultList.h"
#include "DocMatch.h"
#include <Database.h>
#include <htString.h>
#include <Stack.h>
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


