//
// SimpleQueryParser.cc 
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
// $Id: SimpleQueryParser.cc,v 1.1.2.1 2000/09/12 14:58:55 qss Exp $
//

#include "SimpleQueryParser.h"
#include "OperatorQuery.h"

//
// expr == term { term }
//
Query *
SimpleQueryParser::ParseExpression()
{
	Query *result = 0;
	Query *term = ParseTerm();
	if(term)
	{
		if(token.IsEnd())
		{
			result = term;
		}
		else
		{
			result = MakeQuery();
			result->Add(term);
			while(!token.IsEnd())
			{
				term = ParseTerm();
				if(term)
				{
					result->Add(term);
				}
			}
		}
	}
	if(!term)
	{
		delete result;
		result = 0;
	}
	return result;
}


//
// term == word | '"' phrase '"'
//
Query *
SimpleQueryParser::ParseTerm()
{
	Query *result = 0;

	if(token.IsQuote())
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
	else if(token.IsWord())
	{
		// don't advance token here!
		result = ParseWord();
	}
	else
	{
		Expected("a word or a quoted phrase");
	}
	return result;
}


