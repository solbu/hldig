//
// ResultMatch.h
//
// ResultMatch: Contains information related to a given
//              document that was matched by a search. For instance, the
//              score of the document for this search. Similar to the
//              DocMatch class but designed for result display purposes.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: ResultMatch.h,v 1.7.2.3 2000/10/20 03:40:59 ghutchis Exp $
//

#ifndef _ResultMatch_h_
#define _ResultMatch_h_

#include "Object.h"
#include "htString.h"

class DocumentRef;
class Collection;

class ResultMatch : public Object
{
public:
	//
	// Construction/Destruction
	//
					ResultMatch();
					~ResultMatch();
					static ResultMatch *create();
	//
	// Data access members
	//
	void			setAnchor(int a)	{anchor = a;}
	void			setID(int i)		{id = i;}
	void			setScore(double s)	{score = s;}
	
	int				getAnchor()	{return anchor;}
	double				getScore()	{return score;}
	int			getID()			{return id;}

        // Multiple database support
        void            setCollection(Collection *coll) { collection = coll; }
        Collection      *getCollection() { return collection; }  

	static int		setSortType(const String& sorttype);

	// A method for each type of data Display wants to cram in.
	// Will only store the pieces necessary for the
	// search-type as defined in setSortType, the others are dummies.
	virtual char *getTitle();
	virtual time_t getTime();

	virtual void setTitle(const String& title);
	virtual void setTime(time_t t);

	// This is likely to help weak compilers as well as the eye.
	typedef int (*CmpFun)(const void *, const void *);

	// The purpose of the derived classes is to define their own.
	virtual CmpFun getSortFun() = 0;

private:
	enum SortType
	{
	    SortByScore,
	    SortByTime,
	    SortByTitle,
	    SortByID
	};

	double			score;
	int				anchor;
	int				id;
        Collection              *collection;

	static SortType		mySortType;
};

#endif


