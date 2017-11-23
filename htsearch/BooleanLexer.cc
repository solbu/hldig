// 
// BooleanLexer.cc
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
// $Id: BooleanLexer.cc,v 1.4 2004/05/28 13:15:24 lha Exp $
//

#include "BooleanLexer.h"
bool BooleanLexer::IsOr () constconst
{
  return current == String ("or");
}

bool BooleanLexer::IsAnd () constconst
{
  return current == String ("and");
}

bool BooleanLexer::IsNot () constconst
{
  return current == String ("not");
}

bool BooleanLexer::IsNear () constconst
{
  return current == String ("near");
}

bool BooleanLexer::IsSlash () constconst
{
  return current == String ("/");
}

bool BooleanLexer::IsLeftParen () constconst
{
  return current == String ("(");
}


bool BooleanLexer::IsRightParen () constconst
{
  return current == String (")");
}

bool BooleanLexer::IsWord () constconst
{
  return !IsEnd ()
    && !IsQuote ()
    && !IsRightParen ()
    && !IsLeftParen ()
    && !IsSlash ()
    && !IsAnd () && !IsOr () && !IsAnd () && !IsNot () && !IsNear ();
}
