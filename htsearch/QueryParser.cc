// 
// QueryParser.cc
//
// QueryParser: (abstract) root of the family of classes that create
//              Query trees by analyzing query strings.
//              The main public interface consists on Parse(),
//              which does the job.
//              The subclasses must provide a lexer.
//              This class implements also the common behaviour needed to
//              parse single words and phrases.
// 
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: QueryParser.cc,v 1.2 2002/02/02 18:18:13 ghutchis Exp $
//

#include "QueryParser.h"
#include "Query.h"
#include "htString.h"
#include "ExactWordQuery.h"
#include "PhraseQuery.h"
#include "FuzzyExpander.h"

extern int debug;

FuzzyExpander *
QueryParser::expander = 0;

//
// parse a query string
//
//
Query *
QueryParser::Parse(const String &query_string)
{
	error = "";
	Token().Set(query_string);

	Query *result = ParseExpression();
	if(result && !Token().IsEnd())
	{
		Expected("end of query");
	//	delete result;
		result = 0;
	}
	return result;
}

// parse one word
// return a fuzzy word query
//
Query *
QueryParser::ParseWord()
{
	Query *result = 0;
	if(expander)
	{
		result = expander->MakeQuery(Token().Value());
	}
	else
	{
		result = new ExactWordQuery(Token().Value());
	}
	Token().Next();
	return result;
}

//
// parse one word
// return an exact query
//
Query *
QueryParser::ParseExactWord()
{
	Query *result = new ExactWordQuery(Token().Value());
	Token().Next();
	return result;
}

// 
// phrase == word { word }
//
Query *
QueryParser::ParsePhrase()
{
	Query *result = 0;
	Query *word = 0;
	if(!Token().IsEnd() && !Token().IsQuote())
	{
		word = ParseExactWord();
	}
	if(word)
	{
		result = new PhraseQuery;
		result->Add(word);
		while(word && !Token().IsEnd() && !Token().IsQuote())
		{
			word = ParseExactWord();
			if(word)
			{
				result->Add(word);
			}
		}
	}
	if(!word && result)
	{
		delete result;
		result = 0;
	}
	if(!result)
	{
		Expected("at least one word after \"");
	}
	return result;
}

void
QueryParser::Expected(const String &what)
{
	error << "Expected " << what;
	if(Token().IsEnd())
	{
		error << " at the end";
	}
	else
	{
		error << " instead of '" << Token().Value() << "'";
	}
}

