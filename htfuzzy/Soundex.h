//
// Soundex.h
//
// $Id: Soundex.h,v 1.1.1.1 1997/02/03 17:11:12 turtle Exp $
//
// $Log: Soundex.h,v $
// Revision 1.1.1.1  1997/02/03 17:11:12  turtle
// Initial CVS
//
//
#ifndef _Soundex_h_
#define _Soundex_h_

#include "Fuzzy.h"

class Soundex : public Fuzzy
{
public:
	//
	// Construction/Destruction
	//
					Soundex();
	virtual			~Soundex();

	virtual void	generateKey(char *word, String &key);

	virtual void	addWord(char *word);
	
private:
};

#endif


