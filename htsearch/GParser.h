#ifndef _GParser_h_
#define _GParser_h_

//
// GParser.h
//
// GParser: An alternate boolean parser, does not use operator precedence.
//          -- but why is it called G? :-)
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: GParser.h,v 1.1.2.1 2000/09/12 14:58:55 qss Exp $
//

#include "QueryParser.h"
#include "BooleanLexer.h"

class OperatorQuery;

class GParser : public QueryParser
{
public:
	GParser() {}
	~GParser() {}

private:
	// apply the single-level syntax
	Query *ParseExpression();

	// apply the factor syntax
	Query *ParseFactor();

	// return the adequate operator for an operator keyword
	OperatorQuery *MakeOperatorQuery(const String &op) const;

	// give the parent access to the lexical analyzer
	QueryLexer &Token() { return token; }

	// the lexer
	BooleanLexer token;
};

#endif
