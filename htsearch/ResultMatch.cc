//
// ResultMatch.cc
//
// Implementation of ResultMatch
//
// $Log: ResultMatch.cc,v $
// Revision 1.2  1998/11/17 04:06:14  ghutchis
//
// Add new ranking factors backlink_factor and date_factor
//
// Revision 1.1.1.1  1997/02/03 17:11:05  turtle
// Initial CVS
//
// Revision 1.1  1996/01/03 19:02:22  turtle
// Before rewrite
//
//
#if RELEASE
static char RCSid[] = "$Id: ResultMatch.cc,v 1.2 1998/11/17 04:06:14 ghutchis Exp $";
#endif

#include "ResultMatch.h"


//*****************************************************************************
//
ResultMatch::ResultMatch()
{
	incomplete = 1;
}


//*****************************************************************************
//
ResultMatch::~ResultMatch()
{
}


//*****************************************************************************
//
int
ResultMatch::getScore()
{
	if (!incomplete)
		return (int) score;

	//
	// Score has not been computed, yet.
	// Currently we don't do any computing. :-)
	//
	incomplete = 0;
	return (int) score;
}

