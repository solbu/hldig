// 
// BooleanLexer.cc
//
// BooleanLexer: lexical analyzer for boolean query expressions.
//               defines terminal symbols
//               "word", and, or, not, near, (, ), /
// 
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: BooleanLexer.cc,v 1.3 2003/06/24 19:58:07 nealr Exp $
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


