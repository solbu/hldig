//
// ResultMatch.h
//
// $Id: ResultMatch.h,v 1.3 1999/03/12 00:46:57 hp Exp $
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

	//
	// Data access members
	//
	void			setAnchor(int a)			{anchor = a;}
	void			setID(int i)			{id = i;}
	void			setRef(DocumentRef *r)		{ref = r;}
	void			setIncompleteScore(float s)	{score = s;}
	
	int				getAnchor()					{return anchor;}
	int				getScore();
	int			getID()						{return id;}
	DocumentRef		*getRef()					{return ref;}

private:
	float			score;
	int				incomplete;
	int				anchor;
	int				id;
	DocumentRef		*ref;
};

#endif


