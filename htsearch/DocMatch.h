//
// DocMatch.h
//
// $Id: DocMatch.h,v 1.2 1999/08/28 21:11:12 ghutchis Exp $
//
//
#ifndef _DocMatch_h_
#define _DocMatch_h_

#include "Object.h"

class DocMatch : public Object
{
public:
					DocMatch();
					~DocMatch();

	double				score;
	int				id;
	int				anchor;
};

#endif


