//
// WordReference.h
//
// WordReference: Reference to a word. Store everything we need for internal use
//                Defined as a class to allow the comparison 
//                method (for sorting).
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordReference.h,v 1.7 1999/09/10 11:45:29 loic Exp $
//
#ifndef _WordReference_h_
#define _WordReference_h_

#include "htString.h"
#include <stdio.h>

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

	int			Dump(FILE *fl);
	static int		DumpHeader(FILE *fl);
	int			compare(Object *to) {return Word.nocase_compare( ((WordReference *) to)->Word );}
private:
};


#endif


