// 
// OperatorQuery.cc
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
// $Id: OperatorQuery.cc,v 1.1.2.1 2000/09/12 14:58:55 qss Exp $
//

#include "OperatorQuery.h"
//
// return a string with the query as a boolean expression
// descends recursively over the operand
//
String
OperatorQuery::GetLogicalWords() const
{
	ListCursor c;
	String out;
	out << "(";
	if(operands.Count())
	{
		operands.Start_Get(c);
		out << ((Query *) operands.Get_Next(c))->GetLogicalWords();
		Query *next = (Query *) operands.Get_Next(c);
		while(next)
		{
			out << " " << OperatorString() << " ";
			if(next)
			{
				out << next->GetLogicalWords();
			}
			else
			{
				out << "*nothing*";
			}
			next = (Query *) operands.Get_Next(c);
		}
	}
	out << ")";
	return out;
}
