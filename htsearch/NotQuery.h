#ifndef _NotQuery_h_
#define _NotQuery_h_

//
// NotQuery.h
//
// NotQuery: 'not' query operator (n-ary not!)
//           i.e. not(a, b, c, d...) == a except (b or c or d or...)
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: NotQuery.h,v 1.2 2002/02/02 18:18:13 ghutchis Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif

#include "OperatorQuery.h"

//
//
class NotQuery : public OperatorQuery
{
public:

private:
	// evaluate operands and operate
	ResultList *Evaluate();

	// create a difference of the operand results
	ResultList *Subtract(const ResultList &, const List &);

	// used by GetLogicalWords
	String OperatorString() const { return String("not"); }
};

#endif
