//
// GParser.cc
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
// $Id: GParser.cc,v 1.1.2.1 2000/09/12 14:58:55 qss Exp $
//

#include "GParser.h"
#include "OrQuery.h"
#include "NearQuery.h"
#include "AndQuery.h"
#include "NotQuery.h"

Query *
GParser::ParseFactor()
{
	Query *result = 0;

	if(token.IsWord())
	{
		result = ParseWord();
	}
	else if(token.IsQuote())
	{
		token.Next();
		result = ParsePhrase();
		if(result)
		{
			if(token.IsQuote())
			{
				token.Next();
			}
			else
			{
				Expected("closing \"");
				delete result;
				result = 0;
			}
		}
	}
	else if(token.IsLeftParen())
	{
		token.Next();
		result = ParseExpression();
		if(result)
		{
			if(token.IsRightParen())
			{
				token.Next();
			}
			else
			{
				Expected(")");
				delete result;
				result = 0;
			}
		}
	}
	else
	{
		Expected("'(', '\"', or a word");
	}
	return result;
}

OperatorQuery *
GParser::MakeOperatorQuery(const String &op) const
{
cerr << "Making operator for " << op << endl;
	OperatorQuery *result = 0;
	if(op == String("or"))
	{
		result = new OrQuery;
	}
	else if(op == String("and"))
	{
		result = new AndQuery;
	}
	else if(op == String("not"))
	{
		result = new NotQuery;
	}
	else if(op == String("near"))
	{
		result = new NearQuery;
	}
	return result;
}


Query *
GParser::ParseExpression()
{
	List factors;
	Query *result = 0;
	String op = "";

	Query *factor = ParseFactor();
	if(factor)
	{
		result = factor;
	}
	while(factor && (token.IsOr() || token.IsAnd() || token.IsNot() || token.IsNear()))
	{
		if(op != token.Value())
		{
			Query *previous = result;
			result = MakeOperatorQuery(token.Value());
			result->Add(previous);
			op = token.Value();
		}
		token.Next();
		factor = ParseFactor();
		if(factor)
		{
			result->Add(factor);
		}
	}
	if(!factor && result)
	{
		delete result;
		result = 0;
	}
	return result;
}

