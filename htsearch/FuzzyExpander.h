#ifndef _FuzzyExpander_h_
#define _FuzzyExpander_h_

//
// FuzzyExpander.h
//
// FuzzyExpander: (abstract) root of a family of query factories.
//                They make fuzzy queries for given words
//                and store word weights to results
//                by using the existing fuzzy algorithms
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: FuzzyExpander.h,v 1.1.2.1 2000/09/12 14:58:55 qss Exp $
//

//
// for details about the basic architectural pattern see the book:
// Design Patterns, by the infamous GoF
// Factory pattern
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif

#include "htString.h"

class Query;

// abstract
class FuzzyExpander
{
public:
	FuzzyExpander() {}
	virtual ~FuzzyExpander() {}

	// generate a query for this word
	virtual Query *MakeQuery(const String &word) = 0;
};

#endif
