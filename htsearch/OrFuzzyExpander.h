#ifndef _OrFuzzyExpander_h_
#define _OrFuzzyExpander_h_

//
// OrFuzzyExpander.h
//
// OrFuzzyExpander: a concrete Fuzzy expander that makes a OR with
//                  all the results returned by the applicable Fuzzies.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: OrFuzzyExpander.h,v 1.4 2004/05/28 13:15:24 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif

#include "FuzzyExpander.h"
#include "List.h"
#include "Fuzzy.h"

//
// makes a Or query with all the fuzzy expansions
//
class Fuzzy;
class OrFuzzyExpander : public FuzzyExpander
{
public:
	OrFuzzyExpander() {}
	virtual ~OrFuzzyExpander() { filters.Release(); }

	// use this filter
	void Add(Fuzzy *filter) { filters.Add(filter); }


private:
	// generate a OrQuery with all fuzzies found
	Query *MakeQuery(const String &word);

	// Fuzzies to be used
	List filters;
};

#endif
