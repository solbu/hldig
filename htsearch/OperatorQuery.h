#ifndef _OperatorQuery_h_
#define _OperatorQuery_h_

//
// OperatorQuery.h
//
// OperatorQuery: (abstract class) a query that combines result lists
//                returned by other queries kept in an operand list.
//                how they are combined is tbd by the concrete classes.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: OperatorQuery.h,v 1.1.2.1 2000/09/12 14:58:55 qss Exp $
//

//
// for details about the basic architectural pattern see the book:
// Design Patterns, by the infamous GoF
// Interpreter pattern
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif

#include "Query.h"
#include "List.h"

// abstract
class OperatorQuery : public Query
{
public:
	virtual ~OperatorQuery()
	{
		operands.Destroy();
	}

	// add an operand to the operation
	void Add(Query *operand)
	{
		operands.Add(operand);
	}

protected:
	OperatorQuery() {}

	// get results from operands and combine them ad-hoc
	virtual ResultList *Evaluate() = 0;

	// keyword name of the operation
	virtual String OperatorString() const = 0;

	// human-readable unparsed string
	virtual String GetLogicalWords() const;

	// cache index
	String GetSignature() const
		{ return String("Compound:")+GetLogicalWords(); }

	// children query operands
	List operands;
};

#endif
