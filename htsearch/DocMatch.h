//
// DocMatch.h
//
// DocMatch: Data object only. Contains information related to a given
//           document that was matched by a search. For instance, the
//           score of the document for this search.
//
// $Id: DocMatch.h,v 1.3 1999/09/09 10:16:07 loic Exp $
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


