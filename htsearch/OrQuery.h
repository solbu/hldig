#ifndef _OrQuery_h_
#define _OrQuery_h_

//
// OrQuery.h
//
// OrQuery: an operator query that merges all the results of its operands
//          i.e. does 'or' combination
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: OrQuery.h,v 1.2 2002/02/02 18:18:13 ghutchis Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif

#include "OperatorQuery.h"

class OrQuery : public OperatorQuery
{
public:

private:
	// evaluate operands and join results
	ResultList *Evaluate();

	// create a union of the operand results
	ResultList *Union(const ResultList &longer, const List &shorter);

	String OperatorString() const { return String("or"); }
};

#endif
