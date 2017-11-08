//
// Accents.h
//
// Accents: A fuzzy matching algorithm by Robert Marchand, to treat all
//          ISO-8859-1 accented letters as equivalent to their unaccented
//          counterparts.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 2000-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: Accents.h,v 1.4 2004/05/28 13:15:20 lha Exp $
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
	Accents(const HtConfiguration& config_arg);
	virtual		~Accents();

	virtual void	generateKey(char *word, String &key);

	virtual void	addWord(char *word);

	virtual void	getWords(char *word, List &words);

private:
};

#endif

