//
// ResultMatch.h
//
// $Id: ResultMatch.h,v 1.2 1997/03/24 04:33:24 turtle Exp $
//
// $Log: ResultMatch.h,v $
// Revision 1.2  1997/03/24 04:33:24  turtle
// Renamed the String.h file to htString.h to help compiling under win32
//
// Revision 1.1.1.1  1997/02/03 17:11:05  turtle
// Initial CVS
//
// Revision 1.1  1996/01/03 19:02:23  turtle
// Before rewrite
//
//
#ifndef _ResultMatch_h_
#define _ResultMatch_h_

#include <Object.h>
#include <htString.h>

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
	void			setURL(String &s)			{url = s;}
	void			setRef(DocumentRef *r)		{ref = r;}
	void			setIncompleteScore(float s)	{score = s;}
	
	int				getAnchor()					{return anchor;}
	int				getScore();
	char			*getURL()					{return url;}
	DocumentRef		*getRef()					{return ref;}

private:
	float			score;
	int				incomplete;
	int				anchor;
	String			url;
	DocumentRef		*ref;
};

#endif


