//
// ResultMatch.h
//
// $Id: ResultMatch.h,v 1.4 1999/04/19 01:21:51 hp Exp $
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
	void			setAnchor(int a)			{anchor = a;}
	void			setID(int i)			{id = i;}
	void			setScore(float s)	{score = s;}
	
	int				getAnchor()					{return anchor;}
	int				getScore()	{ return (int) score; }
	int			getID()						{return id;}

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


