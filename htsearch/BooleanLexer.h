#ifndef _BooleanLexer_h_
#define _BooleanLexer_h_

//
// BooleanLexer.h
//
// BooleanLexer: lexical analyzer for boolean query expressions.
//               defines terminal symbols
//               "word", and, or, not, near, (, ), /
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: BooleanLexer.h,v 1.1.2.1 2000/09/12 14:58:54 qss Exp $
//

#include "QueryLexer.h"
 
class BooleanLexer : public QueryLexer
{
public:
	// is the current token a word?
	bool IsWord() const;

	// is the current token the 'and' keyword?
	bool IsAnd() const;

	// is the current token the 'or' keyword?
	bool IsOr() const;

	// is the current token the 'near' keyword?
	bool IsNear() const;

	// is the current token the 'not' keyword?
	bool IsNot() const;

	// is the current token the '(' sign?
	bool IsLeftParen() const;

	// is the current token the ')' sign?
	bool IsRightParen() const;

	// is the current token the '/' sign?
	bool IsSlash() const;
};

#endif
