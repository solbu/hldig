#ifndef _BooleanQueryParser_h_
#define _BooleanQueryParser_h_

//
// BooleanQueryParser.h
//
// BooleanQueryParser: Query parser for full-blown boolean expressions
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: BooleanQueryParser.h,v 1.1.2.1 2000/09/12 14:58:54 qss Exp $
//

#include "QueryParser.h"
#include "BooleanLexer.h"

class BooleanQueryParser : public QueryParser
{
public:
	BooleanQueryParser() {}
	~BooleanQueryParser() {}

private:
	// recursive parse levels
	// returning constructed query trees
	Query *ParseExpression();
	Query *ParseAnd();
	Query *ParseNot();
	Query *ParseNear();
	Query *ParseFactor();

	// lexer access needed by parent class
	QueryLexer &Token() { return token; }

	// the lexical analyzer
	BooleanLexer token;
};

#endif
