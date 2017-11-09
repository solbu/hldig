// 
// PhraseQuery.cc
//
// PhraseQuery: an operator query that filters sequenced word matches
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
// 
// $Id: PhraseQuery.cc,v 1.4 2004/05/28 13:15:24 lha Exp $
// 

#include "PhraseQuery.h"

//
// evaluate operands and make a result with matches if some.
//
ResultList *
PhraseQuery::Evaluate()
{
	ResultList *result = 0;

	operands.Start_Get();
	Query *next = (Query *)operands.Get_Next();
	if(next)
	{
		result = (ResultList *)next->GetResults();
		next = (Query *)operands.Get_Next();
	}
	if(result)
	{
		result = new ResultList(*result);
	}
	while(result && next)
	{
		ResultList *r = next->GetResults();
		if(r)
		{
			if(result->IsIgnore())
			{
				delete result;
				result = new ResultList(*r);
			}
			else if(!r->IsIgnore())
			{
				ResultList *tmp = result;
				result = Near(*tmp, *r);
				delete tmp;
			}
			next = (Query *)operands.Get_Next();
		}
		else
		{
			delete result;
			result = 0;
		}
	}
	return result;
}

String
PhraseQuery::GetLogicalWords() const
{
	ListCursor c;
	String out;
	out << "\"";
	if(operands.Count())
	{
		operands.Start_Get(c);
		out << ((Query *) operands.Get_Next(c))->GetLogicalWords();
		Query *next = (Query *) operands.Get_Next(c);
		while(next)
		{
			out << " ";
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
	out << "\"";
	return out;
}

//
// return a resultlist containing matches that are contiguous
//
ResultList *
PhraseQuery::Near(const ResultList &l, const ResultList &r)
{
	ResultList *result = 0;
	DictionaryCursor c;
	l.Start_Get(c);
	DocMatch *match = (DocMatch *)l.Get_NextElement(c);
	while(match)
	{
		DocMatch *confirm = r.find(match->GetId());
		if(confirm)
		{
			List *locations = MergeLocations(
						*match->GetLocations(),
						*confirm->GetLocations());
			if(locations)
			{
				if(!result)
				{
					result = new ResultList;
				}
				DocMatch *copy = new DocMatch(*match);
				copy->SetLocations(locations);
				result->add(copy);
			}
		}
		match = (DocMatch *)l.Get_NextElement(c);
	}
	return result;
}


//
//: merge match positions in a 'next' operation
// each position of left operand match is tested against right operand positions
// if two contiguous positions are found, they are merged into a single one
// beginning at the begin of the left operand
// and ending and the end of the right operand
// 
List *
PhraseQuery::MergeLocations(const List &p, const List &q)
{
	List *result = 0;
	ListCursor pc;
	p.Start_Get(pc);
	const Location *left = (const Location *)p.Get_Next(pc);
	while(left)
	{
		ListCursor qc;
		q.Start_Get(qc);
		const Location *right = (const Location *)q.Get_Next(qc);
		while(right)
		{
			if(left->to + 1 == right->from)
			{
				double prevsize = left->to - left->from + 1.0;
				double addsize = right->to - right->from + 1.0;
				double weight = 
					((left->weight * prevsize) +
					(right->weight * addsize)) / 
					(right->to - left->from + 1.0);

				if(!result)
				{
					result = new List;
				}

				result->Add(new Location(
						left->from,
						right->to,
						left->flags & right->flags,
						weight));
				break;
			}
			right = (const Location *)q.Get_Next(qc);
		}
		left = (const Location *)p.Get_Next(pc);
	}
	return result;
}

