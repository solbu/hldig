//
// WordReference.h
//
// $Id: WordReference.h,v 1.4 1999/09/05 18:25:39 ghutchis Exp $
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

	int			compare(Object *to) {return Word.nocase_compare( ((WordReference *) to)->Word );}
private:
};


#endif


