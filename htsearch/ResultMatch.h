//
// ResultMatch.h
//
// ResultMatch: Contains information related to a given
//              document that was matched by a search. For instance, the
//              score of the document for this search. Similar to the
//              DocMatch class but designed for result display purposes.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: ResultMatch.h,v 1.6 1999/09/10 17:22:25 ghutchis Exp $
//

#ifndef _ResultMatch_h_
#define _ResultMatch_h_

#include "Object.h"
#include "htString.h"

class DocumentRef;

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
	void			setScore(float s)	{score = s;}
	
	int				getAnchor()	{return anchor;}
	int				getScore()	{return (int) score;}
	int			getID()			{return id;}

	static int		setSortType(char *);

	// A method for each type of data Display wants to cram in.
	// Will only store the pieces necessary for the
	// search-type as defined in setSortType, the others are dummies.
	virtual char *getTitle();
	virtual time_t getTime();

	virtual void setTitle(char *);
	virtual void setTime(time_t);

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

	float			score;
	int				anchor;
	int				id;

	static SortType		mySortType;
};

#endif


