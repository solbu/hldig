#ifndef _QueryLexer_h_
#define _QueryLexer_h_

//
// QueryLexer.h
//
// QueryLexer: (abstract) a lexical analyzer used by a QueryParser.
//             This class defines the common public interface of this
//             family of lexers. It implements a tokenizer, and also
//             the definition of the 'quote' and 'end' terminal symbols.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: QueryLexer.h,v 1.4 2004/05/28 13:15:24 lha Exp $
//


#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif

#include "htString.h"

class QueryLexer
{
public:
	virtual ~QueryLexer() {}

	// set the query string and advance to the first token
	void Set(const String &query_string);

	// advance to the next token
	virtual void Next();

	// is the current token a word?
	virtual bool IsWord() const = 0;

	// is the current token a quote sign?
	bool IsQuote() const;

	// is the current token end-of-query?
	bool IsEnd() const;

	// get the current token value
	const String &Value() const { return current; }

	// get the full query string
	const String &FullString() const { return query; }


protected:
	QueryLexer();

	// the full query string
	String query;

	// the current token value
	String current;

	// the current position in the query string
	int current_char;

	// suffix string used by the 'prefix' fuzzy
	String prefix_match;
};

#endif
