//
// WordReference.h
//
// $Id: WordReference.h,v 1.1.1.1 1997/02/03 17:11:07 turtle Exp $
//
// $Log: WordReference.h,v $
// Revision 1.1.1.1  1997/02/03 17:11:07  turtle
// Initial CVS
//
//
#ifndef _WordReference_h_
#define _WordReference_h_

#include <String.h>

class WordReference : public Object
{
public:
	//
	// Construction/Destruction
	//
					WordReference()			{}
					~WordReference()		{}

	char			Word[MAX_WORD_LENGTH + 1];
	int				WordCount;
	int				Weight;
	int				Location;
	int				DocumentID;
	int				Anchor;
private:
};


#endif


