#ifndef _AndQuery_h_
#define _AndQuery_h_

//
// AndQuery.h
//
// AndQuery: an operator query that does 'and' combination
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: AndQuery.h,v 1.3 2003/06/24 19:58:07 nealr Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif

#include "OperatorQuery.h"

//
// and query
// 
class AndQuery : public OperatorQuery
{
public:

private:
	// evaluate operands and intersect results
	ResultList *Evaluate();

	// create an intersection of the operand results
	ResultList *Intersection(const ResultList &shorter, const List &longer);

	// used by GetLogicalWords
	String OperatorString() const { return String("and"); }
};

#endif
