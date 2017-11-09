// 
// NotQuery.cc
//
// NotQuery: 'not' query operator (n-ary not!)
//           i.e. not(a, b, c, d...) == a except (b or c or d or...)
// 
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html> 
// 
// $Id: NotQuery.cc,v 1.4 2004/05/28 13:15:24 lha Exp $
//


#include "NotQuery.h"
//
//	l	r	not
//	-------------------------
//	0	0	0
//	0	b	0
//	0	x	0
//	a	0	a
//	a	b	diff(a,b)
//	a	x	a
//	x	0	x
//	x	b	x
//	x	x	x
// 
// note l is the first operand, r is the rest
// i.e. l = 0 => not = 0
//      l = x => not = x
//      r = 0 => not = l
//      r = x => not = l
//      subtract otherwise
//
ResultList *
NotQuery::Evaluate()
{
	operands.Start_Get();
	Query *operand = (Query *) operands.Get_Next();
	ResultList *result = 0;
	ResultList *positive = operand->GetResults();
	if(positive)
	{
		List negative;
		if(!positive->IsIgnore())
		{
			operand = (Query *) operands.Get_Next();
			while(operand)
			{
				ResultList *next = operand->GetResults();
				if(next && !next->IsIgnore())
				{
					negative.Add(next);
				}
				operand = (Query *) operands.Get_Next();
			}
		}
		if(negative.Count())
		{
			result = Subtract(*positive, negative);
			negative.Release();
		}
		else
		{
			result = new ResultList(*positive);
		}
	}
	return result;
}

//
// make a result list containing all matches in positive
// with docId absent from negatives
//
ResultList *
NotQuery::Subtract(const ResultList &positive, const List &negatives)
{
	ResultList *result = 0;
	DictionaryCursor pc;
	positive.Start_Get(pc);
	DocMatch *match = (DocMatch *)positive.Get_NextElement(pc);
	while(match)
	{
		bool confirm = true;
		ListCursor lc;
		negatives.Start_Get(lc);
		ResultList *negative = (ResultList *)negatives.Get_Next(lc);
		while(confirm && negative)
		{
			if(negative->exists(match->GetId()))
			{
				confirm = false;
			}
			negative = (ResultList *)negatives.Get_Next(lc);
		}
		if(confirm)
		{
			if(!result)
			{
				result = new ResultList;
			}
			result->add(new DocMatch(*match));
		}
		match = (DocMatch *)positive.Get_NextElement(pc);
	}
	return result;
}
