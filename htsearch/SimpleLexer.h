#ifndef _SimpleLexer_h_
#define _SimpleLexer_h_

//
// SimpleLexer.h
//
// SimpleLexer: query lexer for simple (no-keyword) queries
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: SimpleLexer.h,v 1.1.2.1 2000/09/12 14:58:55 qss Exp $
//

#include "QueryLexer.h"
 
class SimpleLexer : public QueryLexer
{
public:
	SimpleLexer() : QueryLexer() {}

	// everything is a word
	bool IsWord() const { return !IsEnd(); }
};

#endif
