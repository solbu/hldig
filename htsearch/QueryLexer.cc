// 
// QueryLexer.cc
//
// QueryLexer: (abstract) a lexical analyzer used by a QueryParser.
//             This class defines the common public interface of this
//             family of lexers. It implements a tokenizer, and also
//             the definition of the 'quote' and 'end' terminal symbols.
// 
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
// 
// $Id: QueryLexer.cc,v 1.2 2002/02/02 18:18:13 ghutchis Exp $
// 

#include "QueryLexer.h"
#include "defaults.h"
#include "WordType.h"


extern int debug;

QueryLexer::QueryLexer()
{
	HtConfiguration* config= HtConfiguration::config();
	prefix_match = config->Find("prefix_match_character");
}

void
QueryLexer::Set(const String &query_string)
{
	query = query_string;
	current_char = 0;
	Next();
}

void
QueryLexer::Next()
{
	HtConfiguration* config= HtConfiguration::config();
	unsigned char	text = query[current_char];
	WordType	type(*config);
	current = "";

	while (text
	&& !current.length()
	&& !type.IsStrictChar(text))
	{
		if (text == '(' || text == ')' || text == '\"' || text == '/')
		{
	    		current << text;
			if (debug) cerr << "lexer symbol: " << current << endl;
		}
		text = query[++current_char];
	}

	if (!current.length() && text)
	{
		while (text
		&& (type.IsChar(text) && text != '/'
			|| prefix_match.indexOf(text, 0) != -1))
		{
			current << text;
			text = query[++current_char];
		}
	}
	current.lowercase();
	if (debug) cerr << "lexer current word: " << current << endl;
}

bool
QueryLexer::IsEnd() const
{
	return current == String("");
}

bool
QueryLexer::IsQuote() const
{
	return current == String("\"");
}

