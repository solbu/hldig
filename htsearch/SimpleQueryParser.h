#ifndef _SimpleQueryParser_h_
#define _SimpleQueryParser_h_

//
// SimpleQueryParser.h
//
// SimpleQueryParser: (abstract) a family of parsers that generate queries
//                    for strings with the syntax (word|phrase){(word|phrase)}
//                    combining them in a single operator.
//                    The operator to apply is tbd by concrete classes.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: SimpleQueryParser.h,v 1.1.2.1 2000/09/12 14:58:55 qss Exp $
//

#include "QueryParser.h"
#include "SimpleLexer.h"

// abstract
class OperatorQuery;

class SimpleQueryParser : public QueryParser
{
public:
	virtual ~SimpleQueryParser() {}

protected:
	SimpleQueryParser() {}

	// get a combination query
	virtual OperatorQuery *MakeQuery() = 0;

private:
	// apply expr == term { term }
	Query *ParseExpression();

	// apply term == word | phrase
	Query *ParseTerm();

	// let the parent access the lexer
	QueryLexer &Token() { return token; }

	// the used lexer
	SimpleLexer token;
};

#endif
