//
// WordReference.h
//
// $Id: WordReference.h,v 1.3 1999/07/19 01:49:09 ghutchis Exp $
//
//
#ifndef _WordReference_h_
#define _WordReference_h_

#include "htString.h"

class WordReference : public Object
{
public:
	//
	// Construction/Destruction
	//
        WordReference()		{}
	~WordReference()	{}

	String			Word;
	long int		DocumentID;
	long int		Flags;
	int			Location;
	int			Anchor;
private:
};


#endif


