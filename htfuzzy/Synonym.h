//
// Synonym.h
//
// Synonym: A fuzzy matching algorithm to create a database of related words
//          (or misspellings) that should be searched together.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Synonym.h,v 1.5 2002/02/01 22:49:33 ghutchis Exp $
//
//

#ifndef _Synonym_h_
#define _Synonym_h_

#include "Fuzzy.h"

class List;

class Synonym : public Fuzzy
{
public:
	//
	// Construction/Destruction
	//
			Synonym(const HtConfiguration& config_arg);
			~Synonym();

	//
	// Lookup routines
	//
	virtual void	getWords(char *word, List &words);
	virtual int	openIndex();

	//
	// Creation
	//
	virtual int	createDB(const HtConfiguration &config);
	
protected:

	Database	*db;
};

#endif


