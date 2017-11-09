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
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: BooleanLexer.h,v 1.4 2004/05/28 13:15:24 lha Exp $
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
