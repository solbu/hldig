#ifndef _NearQuery_h_
#define _NearQuery_h_

//
// NearQuery.h
//
// NearQuery: An operator query that filters matches by proximity.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: NearQuery.h,v 1.2 2002/02/02 18:18:13 ghutchis Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif

#include "OperatorQuery.h"

class NearQuery : public OperatorQuery
{
public:
	// binary fashion
	NearQuery(Query *left, Query *right, unsigned int dist) :
		distance(dist)
		{ Add(left); Add(right); }

	// n-ary fashion -- will ignore operands for n>2
	NearQuery(unsigned int dist = 10) :
		distance(dist) {}

private:
	// get results from operands and filter
	ResultList *Evaluate();

	// create a result with neighboring matches
	ResultList *Near(const ResultList &, const ResultList &);

	// merge neighboring location lists
	List *MergeLocations(const List &, const List &);

	String OperatorString() const;
	unsigned int distance;
};

#endif
