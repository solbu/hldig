//
// SuffixEntry.h
//
// SuffixEntry: Decode the suffix rules used in the ispell dictionary files
//              for the endings fuzzy DB.
//           
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: SuffixEntry.h,v 1.4 2002/02/01 22:49:33 ghutchis Exp $
//


#ifndef _SuffixEntry_h_
#define _SuffixEntry_h_

#include "Object.h"
#include "htString.h"


class SuffixEntry : public Object
{
public:
	//
	// Construction/Destruction
	//
				SuffixEntry(char *);
				~SuffixEntry();

	String			expression;
	String			rule;

	void			parse(char *str);
	
private:
};

#endif


