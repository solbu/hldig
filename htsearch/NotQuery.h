#ifndef _NotQuery_h_
#define _NotQuery_h_

//
// NotQuery.h
//
// NotQuery: 'not' query operator (n-ary not!)
//           i.e. not(a, b, c, d...) == a except (b or c or d or...)
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: NotQuery.h,v 1.3 2003/06/24 19:58:07 nealr Exp $
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
