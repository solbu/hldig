//
// parser.h
//
// $Id: parser.h,v 1.1.1.1 1997/02/03 17:11:05 turtle Exp $
//
// $Log: parser.h,v $
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
#include <String.h>
#include <Stack.h>
#include <ctype.h>

class Parser
{
public:
	                Parser();
	
	int				checkSyntax(List *);
	void			parse(List *, ResultList &);

	void			setDatabase(Database *db)	{dbf = db;}
	char			*getErrorMessage()			{return error.get();}
	int				hadError()					{return valid == 0;}
	
protected:
	int				lexan();
	void			expr(int);
	void			term(int);
	void			factor(int);
	int				match(int);
	void			perform_push();
	void			perform_and();
	void			perform_or();

	List			*tokens;
	List			*result;
	WeightWord		*current;
	int				lookahead;
	int				valid;
	Stack			stack;
	Database		*dbf;
	String			error;
};


#endif


