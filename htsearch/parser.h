//
// parser.h
//
// parser: Parses a boolean expression tree, retrieving and scoring 
//         the resulting document list
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: parser.h,v 1.16 2003/02/11 09:49:38 lha Exp $
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
#include "HtWordList.h"
#include <ctype.h>

class Collection;

class Parser
{
public:
    Parser();
	
    int			checkSyntax(List *);
    void		parse(List *, ResultList &);

    // void		setDatabase(const String& db)		{ words.Open(db, O_RDONLY); }
    void                setCollection(Collection *collection);
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
    void		perform_and();
    void		perform_not();
    void		perform_or();
    void		perform_phrase(List * &);

    void		score(List *, double weight, unsigned int flags);

    List		*tokens;
    List		*result;
    WeightWord		*current;
    int			lookahead;
    int			valid;
    Stack		stack;
    String		error;
    Collection          *collection; // Multiple database support

    HtWordList		words;
};

extern StringList	boolean_keywords;
enum KeywordIndices	{ AND, OR, NOT };

#endif


