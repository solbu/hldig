// 
// BooleanQueryParser.cc
//
// BooleanQueryParser: Query parser for full-blown boolean expressions
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
// 
// $Id: BooleanQueryParser.cc,v 1.1.2.2 2000/10/21 22:20:14 ghutchis Exp $
//

#include "BooleanQueryParser.h"

#include "OrQuery.h"
#include "NotQuery.h"
#include "AndQuery.h"
#include "NearQuery.h"
#include "PhraseQuery.h"
#include "FuzzyExpander.h"

//
// expr == andlist ( 'or' andlist )
//
Query *
BooleanQueryParser::ParseExpression()
{
	Query *result = 0;
	Query *term = ParseAnd();
	if(term)
	{
		if(token.IsOr())
		{
			result = new OrQuery;
			result->Add(term);
			while(term && token.IsOr())
			{
				token.Next();
				term = ParseAnd();
				if(term)
				{
					result->Add(term);
				}
			}
		}
		else
		{
			result = term;
		}
	}
	if(!term && result)
	{
		delete result;
		result = 0;
	}
	return result;
}

//
// notlist = nearlist { 'not' nearlist }
//
Query *
BooleanQueryParser::ParseNot()
{
	Query *result = 0;
	Query *near = ParseNear();
	if(near)
	{
		if(token.IsNot())
		{
			result = new NotQuery();
			result->Add(near);
			while(near && token.IsNot())
			{
				token.Next();
				near = ParseNear();
				if(near)
				{
					result->Add(near);
				}
			}
		}
		else
		{
			result = near;
		}
	}
	if(!near && result)
	{
		delete result;
		result = 0;
	}
	return result;
}

//
// andlist = notlist { 'and' notlist }
//
Query *
BooleanQueryParser::ParseAnd()
{
	Query *result = 0;
	Query *notList = ParseNot();

	if(notList)
	{
		if(token.IsAnd())
		{
			result = new AndQuery();
			result->Add(notList);
			while(notList && token.IsAnd())
			{
				token.Next();
				notList = ParseNot();
				if(notList)
				{
					result->Add(notList);
				}
			}
		}
		else
		{
			result = notList;
		}
	}
	if(!notList && result)
	{
		delete result;
		result = 0;
	}
	return result;
}

//
// near == factor { 'near'  [ '/' number ] factor }
// 'near' query is binary
//
Query *
BooleanQueryParser::ParseNear()
{
	Query *result = ParseFactor();
	while(result && token.IsNear())
	{
		token.Next();
		int distance = 10; // config["default_near_distance"];
		if(token.IsSlash())
		{
			distance = 0;
			token.Next();
			if(token.IsWord())
			{
				distance = token.Value().as_integer();
				token.Next();
			}
		}
		if(distance > 0)
		{
			Query *right = ParseFactor();
			if(right)
			{
				Query *tmp = new NearQuery(distance);
				tmp->Add(result);
				tmp->Add(right);
				result = tmp;
			}
			else
			{
				delete result;
				result = 0;
			}
		}
		else
		{
			Expected("a distance > 0 for 'Near'");
			delete result;
			result = 0;
		}
	}
	return result;
}

//
// factor == word | '"' phrase '"' | '(' expression ')'
//
Query *
BooleanQueryParser::ParseFactor()
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

