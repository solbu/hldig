// DocMatch.cc
//
// DocMatch: Data object only. Contains information related to a given
//           document that was matched by a search. For instance, the
//           score of the document for this search.
//           
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: DocMatch.cc,v 1.3.2.3 2000/09/12 14:58:54 qss Exp $
//

#include "DocMatch.h"
#include <iostream>


//*******************************************************************************
// DocMatch::DocMatch()
//


//*******************************************************************************
// DocMatch::~DocMatch()
//
DocMatch::~DocMatch()
{
}

//
// merge with another match
// sets anchor to the lower value
// merges location lists
//
void
DocMatch::Merge(const DocMatch &match)
{
        if(match.anchor < anchor)
        {
		anchor = match.anchor;
        }
        AddLocations(match.GetLocations());
}

//
// adds locations to an existing list
// avoiding duplicates, in location order
//
void
DocMatch::AddLocations(const List *locs)
{
	List *merge = new List;
	ListCursor c;

	locations->Start_Get();
	locs->Start_Get(c);
	Location *a = (Location *)locations->Get_Next();
	Location *b = (Location *)locs->Get_Next(c);
	while(a && b)
	{
		if(a->from < b->from)
		{
			merge->Add(a);
			a = (Location *)locations->Get_Next();
		}
		else if(a->from > b->from)
		{
			merge->Add(new Location(*b));
			b = (Location *)locs->Get_Next(c);
		}
		else // (a->from == b->from)
		{
			if(a->to < b->to)
			{
				merge->Add(new Location(*a));
				merge->Add(new Location(*b));
			}
			else if(a->to > b->to)
			{
				merge->Add(new Location(*b));
				merge->Add(new Location(*a));
			}
			else // (a->to == b->to)
			{
				merge->Add(new Location(
						a->from,
						a->to,
						a->flags,
						a->weight + b->weight));
			}
			a = (Location *)locations->Get_Next();
			b = (Location *)locs->Get_Next(c);
		}
	}
	while(a)
	{
		merge->Add(a);
		a = (Location *)locations->Get_Next();
	}
	while(b)
	{
		merge->Add(new Location(*b));
		b = (Location *)locs->Get_Next(c);
	}
	locations->Release();
	delete locations;
	locations = merge;
}

//
// set the location list
//
void
DocMatch::SetLocations(List *locs)
{
	delete locations;
	locations = locs;
}

//
// copy constructor, copies locations
//
DocMatch::DocMatch(const DocMatch &other)
{
	score = -1.0;
	//score = other.score;
	id = other.id;
	anchor = other.anchor;
	locations = new List;
	AddLocations(other.GetLocations());
}

//
// set weight of all locations
//
void
DocMatch::SetWeight(double weight)
{
	locations->Start_Get();
	for(int i = 0; i < locations->Count(); i++)
	{
		Location *loc = (Location *)locations->Get_Next();
		loc->weight = weight;
	}
}

//
// debug dump
//
void
DocMatch::Dump()
{
	double score = 0.0;
	cerr << "DocMatch id: " <<  id << " {" << endl;
	locations->Start_Get();
	for(int i = 0; i < locations->Count(); i++)
	{
		Location *loc = (Location *)locations->Get_Next();
		cerr << "location [" << loc->from;
		cerr << ", " << loc->to << "] ";
		cerr << "weight " << loc->weight;
		cerr << " flags " << loc->flags;
		cerr << endl;
		score += loc->weight;
	}
	cerr << "score: " << score << endl << "}" << endl;
}

double
DocMatch::GetScore() const
{
	double result = 0.0;
	ListCursor c;
	locations->Start_Get(c);
	Location *loc = (Location *)locations->Get_Next(c);
	while(loc)
	{
		result += loc->weight;
		loc = (Location *)locations->Get_Next(c);
	}
	return result;
}
