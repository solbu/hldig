#ifndef _SimpleLexer_h_
#define _SimpleLexer_h_

//
// SimpleLexer.h
//
// SimpleLexer: query lexer for simple (no-keyword) queries
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: SimpleLexer.h,v 1.4 2004/05/28 13:15:24 lha Exp $
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
