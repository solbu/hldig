//
// Synonym.h
//
// Synonym: A fuzzy matching algorithm to create a database of related words
//          (or misspellings) that should be searched together.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Synonym.h,v 1.2 1999/09/10 17:22:25 ghutchis Exp $
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
			Synonym();
			~Synonym();

	//
	// Lookup routines
	//
	virtual void	getWords(char *word, List &words);
	virtual int	openIndex(Configuration &);

	//
	// Creation
	//
	virtual int	createDB(Configuration &config);
	
protected:

	Database	*db;
};

#endif


