#ifndef _QueryParser_h_
#define _QueryParser_h_

//
// QueryParser.h
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
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: QueryParser.h,v 1.4 2004/05/28 13:15:24 lha Exp $
//

#include "QueryLexer.h"

class Query;
class FuzzyExpander;

// abstract
class QueryParser
{
public:
	virtual ~QueryParser() {}

	// do it
	Query *Parse(const String &query_string);

	// contains a diagnostic if Parse() failed
	const String &Error() const
		{ return error; }

	// set a fuzzy word expansion policy 
	static void SetFuzzyExpander(FuzzyExpander *x)
		{ expander = x; }

protected:
	QueryParser() {}

	// apply a syntax -- tbd by derived classes
	virtual Query *ParseExpression() = 0;

	// access to the lexer -- provided by children
	virtual QueryLexer &Token() = 0;

	// parse one (fuzzy) word
	Query *ParseWord();

	// parse an exact word
	Query *ParseExactWord();

	// parse a phrase
	Query *ParsePhrase();

	// set the error string on syntax error
	void Expected(const String &what);

	// the current fuzzy expansion policy if some
	static FuzzyExpander *expander;

private:
	// syntax error if some
	String error;
};

#endif
