//
// DocMatch.h
//
// DocMatch: Data object only. Contains information related to a given
//           document that was matched by a search. For instance, the
//           score of the document for this search.
//           
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2002 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: DocMatch.h,v 1.6 2002/02/10 00:07:58 ghutchis Exp $
//

#ifndef _DocMatch_h_
#define _DocMatch_h_

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif

#include "Object.h"
#include "List.h"

class Collection;

//
// an element of the DocMatch location list
//
struct Location : public Object
{
	Location(int f, int t, unsigned int l, double w = 1.0) :
		from(f), to(t), flags(l), weight(w) {}
	Location(const Location &l) :
		from(l.from), to(l.to), flags(l.flags), weight(l.weight) {}
	int from;
	int to;
	unsigned int flags;
	double weight;
};

//
// an element of a ResultList
//
class DocMatch : public Object
{
public:
	// default constructor
	DocMatch() :
		locations(new List),
		score(-1.0),
		id(0),
		anchor(0),
		collection(0) {}

	// copy constructor	
	DocMatch(const DocMatch &);

	// destructor
	~DocMatch();

	// match join
	void Merge(const DocMatch &);

	// score accessor
	double GetScore();
	void SetScore(double);

	// doc id accessors
	int GetId() const { return id; }
	void SetId(int x) { id = x; }

	// anchor accessors
	int GetAnchor() const { return anchor; }
	void SetAnchor(int x) { anchor = x; }

	// location list accessors
	const List *GetLocations() const { return locations; }
	void SetLocations(List *);
	void AddLocations(const List *);

	// add one location to the list
	// use with caution -- does not ensure {ordered}
	void AddLocation(Location *x) { locations->Add(x); }

	// set weight of all locations
	void SetWeight(double weight);

	// debug
	void Dump();
	
private:
	List			*locations;
// the rest should be private:
// but is already used by the old htsearch
public:

	double				score;
	int				id;
	int				anchor;
	Collection                      *collection; // Multiple databases
};

#endif


