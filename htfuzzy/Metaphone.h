//
// Metaphone.h
//
// $Id: Metaphone.h,v 1.1.1.1 1997/02/03 17:11:12 turtle Exp $
//
// $Log: Metaphone.h,v $
// Revision 1.1.1.1  1997/02/03 17:11:12  turtle
// Initial CVS
//
//
#ifndef _Metaphone_h_
#define _Metaphone_h_

#include "Fuzzy.h"

class Metaphone : public Fuzzy
{
public:
	//
	// Construction/Destruction
	//
					Metaphone();
	virtual			~Metaphone();

	virtual void	generateKey(char *word, String &key);

	virtual void	addWord(char *word);
	
private:
};

#endif


