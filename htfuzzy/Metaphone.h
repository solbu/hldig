//
// Metaphone.h
//
// Metaphone: A fuzzy matching algorithm used to match words that
//            sound alike in the English language. Probably not so 
//            good for foreign languages.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Metaphone.h,v 1.3 1999/09/24 10:29:01 loic Exp $
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
			Metaphone(const Configuration& config_arg);
	virtual		~Metaphone();

	virtual void	generateKey(char *word, String &key);

	virtual void	addWord(char *word);
	
private:
};

#endif


