//
// Soundex.h
//
// Soundex: A fuzzy matching algorithm on the principal of the 
//          Soundex method for last names used by the U.S. INS
//          and described by Knuth and others.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Soundex.h,v 1.3 1999/09/24 10:29:01 loic Exp $
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
        Soundex(const Configuration& config_arg);
	virtual			~Soundex();

	virtual void	generateKey(char *word, String &key);

	virtual void	addWord(char *word);
	
private:
};

#endif


