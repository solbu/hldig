// 
// BooleanLexer.cc
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
// $Id: BooleanLexer.cc,v 1.1.2.1 2000/09/12 14:58:54 qss Exp $
//

#include "BooleanLexer.h"
bool
BooleanLexer::IsOr() const
{
	return current == String("or");
}
	
bool
BooleanLexer::IsAnd() const
{
	return current == String("and");
}

bool
BooleanLexer::IsNot() const
{
	return current == String("not");
}

bool
BooleanLexer::IsNear() const
{
	return current == String("near");
}

bool
BooleanLexer::IsSlash() const
{
	return current == String("/");
}

bool
BooleanLexer::IsLeftParen() const
{
	return current == String("(");
}

	
bool
BooleanLexer::IsRightParen() const
{
	return current == String(")");
}

bool
BooleanLexer::IsWord() const
{
	return !IsEnd()
	&& !IsQuote()
	&& !IsRightParen()
	&& !IsLeftParen()
	&& !IsSlash()
	&& !IsAnd()
	&& !IsOr()
	&& !IsAnd()
	&& !IsNot()
	&& !IsNear();
}


