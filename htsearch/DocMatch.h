//
// DocMatch.h
//
// $Id: DocMatch.h,v 1.1.1.1 1997/02/03 17:11:05 turtle Exp $
//
// $Log: DocMatch.h,v $
// Revision 1.1.1.1  1997/02/03 17:11:05  turtle
// Initial CVS
//
//
#ifndef _DocMatch_h_
#define _DocMatch_h_

#include <Object.h>

class DocMatch : public Object
{
public:
					DocMatch();
					~DocMatch();

	float			score;
	int				id;
	int				anchor;
};

#endif


