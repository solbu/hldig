//
// Accents.h
//
// $Id: Accents.h,v 1.1.4.1 2001/06/15 21:53:59 grdetil Exp $
//
//
#ifndef _Accents_h_
#define _Accents_h_

#include "Fuzzy.h"

class Accents : public Fuzzy
{
public:
	//
	// Construction/Destruction
	//
					Accents();
	virtual			~Accents();

	virtual void	generateKey(char *word, String &key);

	virtual void	addWord(char *word);

	virtual void	getWords(char *word, List &words);

private:
};

#endif
