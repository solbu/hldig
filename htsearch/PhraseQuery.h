#ifndef _PhraseQuery_h_
#define _PhraseQuery_h_

//
// PhraseQuery.h
//
// PhraseQuery: an operator query that filters sequenced word matches
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: PhraseQuery.h,v 1.2 2002/02/02 18:18:13 ghutchis Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif

#include "OperatorQuery.h"

class PhraseQuery : public OperatorQuery
{
public:
	PhraseQuery() {}
	~PhraseQuery() {}

private:
	// get results from operands and filter
	ResultList *Evaluate();

	// create a result with neighboring matches
	ResultList *Near(const ResultList &, const ResultList &);

	// merge neighboring location lists, constructing phrase locations
	List *MergeLocations(const List &, const List &);

	String OperatorString() const { return ""; }

	String GetLogicalWords() const;
};

#endif
