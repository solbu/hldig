#ifndef _GParser_h_
#define _GParser_h_

//
// GParser.h
//
// GParser: An alternate boolean parser, does not use operator precedence.
//          -- but why is it called G? :-)
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: GParser.h,v 1.3 2003/06/24 19:58:07 nealr Exp $
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
