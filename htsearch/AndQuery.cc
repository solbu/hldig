// 
// AndQuery.cc
//
// AndQuery: an operator query that does 'and' combination
//           
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html> 
// 
// $Id: AndQuery.cc,v 1.2 2002/02/02 18:18:13 ghutchis Exp $
// 


#include "AndQuery.h"
//
//	l	r	and
//	----------------------
//	0	0	0
//	0	b	0
//	0	x	0
//	a	0	0
//	a	b	intersect(a,b)
//	a	x	a
//	x	0	0
//	x	b	b
//	x	x	x
//
//	i.e. some 0 => 0
//	ignores can be left out of intersection
// the shorter of the result lists is put apart for intersection
// this optimises the intersection process
//

ResultList *
AndQuery::Evaluate()
{
	ResultList *result = 0;
	ResultList *shorter = 0;

	operands.Start_Get();
	Query *operand = (Query *) operands.Get_Next();
	while(operand && !shorter)
	{
		result = operand->GetResults();
		if(!result)
		{
			break;
		}
		if(!result->IsIgnore())
		{
			shorter = result;
		}
		operand = (Query *) operands.Get_Next();
	}
	if(shorter)
	{
		List longer;
		while(operand && result)
		{
			result = operand->GetResults();
			if(result && !result->IsIgnore())
			{
				if(result->Count() < shorter->Count())
				{
					longer.Add(shorter);
					shorter = result;
				}
				else
				{
					longer.Add(result);
				}
			}
			operand = (Query *) operands.Get_Next();
		}
		if(longer.Count())
		{
			result = Intersection(*shorter, longer);
			longer.Release();
		}
		else
		{
			result = new ResultList(*shorter);
		}
	}
	return result;
}

//
// return a result list containing only the matches common to
// all input parameters.
//
// l is iterated, matches from l are searched in all elements of rs
//
//
//	foreach match in shorter
//		confirm the match in each lists
//		if confirmed
//			copy all matches in result
//
// the shorter of the input lists is assumed to be in the first parameter
// this is a modest optimisation in order to minimise iteration
//

ResultList *
AndQuery::Intersection(const ResultList &shorter, const List &lists)
{
	ResultList *result = 0;
	DictionaryCursor c;
	shorter.Start_Get(c);
	DocMatch *match = (DocMatch *)shorter.Get_NextElement(c);
	while(match)
	{
		List confirms;
	
		ListCursor lc;	
		lists.Start_Get(lc);
		ResultList *list = (ResultList *)lists.Get_Next(lc);
		while(list)
		{
			DocMatch *confirm = list->find(match->GetId());
			if(confirm)
			{
				confirms.Add(confirm);
			}
			list = (ResultList *)lists.Get_Next(lc);
		}
		if(confirms.Count() == lists.Count())
		{
			if(!result)
			{
				result = new ResultList;
			}
			DocMatch *copy = new DocMatch(*match);
			confirms.Start_Get();
			DocMatch *confirm = (DocMatch *)confirms.Get_Next();
			while(confirm)
			{
				copy->Merge(*confirm);
				confirm = (DocMatch *)confirms.Get_Next();
			}
			result->add(copy);
		}
		confirms.Release();
		match = (DocMatch *)shorter.Get_NextElement(c);
	}
	return result;
}

