// 
// ExactWordQuery.cc
//
// ExactWordQuery: A Query tree leaf object. Wraps a database access
//                 that generates ResultLists for word matches.
// 
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
// 
// $Id: ExactWordQuery.cc,v 1.2 2002/02/02 18:18:13 ghutchis Exp $
//

#include "ExactWordQuery.h"
#include "WordSearcher.h"

//
// the searcher object used by all instances
// of ExactWord
//
WordSearcher *
ExactWordQuery::searcher = 0;

//
// set the weight of the matches to my weight
//
void
ExactWordQuery::AdjustWeight(ResultList &results)
{
	results.SetWeight(weight);
}

//
// tell the searcher to fetch my word in the database
// return 0 if no matches
//
ResultList *
ExactWordQuery::Evaluate()
{
	ResultList *result = 0;
	if(searcher)
	{
		result = searcher->Search(word);
	}
	if(result && !result->Count() && !result->IsIgnore())
	{
		delete result;
		result = 0;
	}
	return result;
}
