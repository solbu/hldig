	//
// WordRecord.h
//
// $Id: WordRecord.h,v 1.2 1998/12/06 18:46:22 ghutchis Exp $
//
// $Log: WordRecord.h,v $
// Revision 1.2  1998/12/06 18:46:22  ghutchis
// Ensure blank WordRecords have a default count of 1 since a word has to exist
// to have a WordRecord!
//
// Revision 1.1.1.1  1997/02/03 17:11:07  turtle
// Initial CVS
//
//
#ifndef _WordRecord_h_
#define _WordRecord_h_

struct WordRecord
{
	int		count;
	int		id;
	int		weight;
	int		anchor;
	int		location;

	void	Clear()
			{
				id = weight = anchor = location = 0;
				count = 1;
			}
};

#endif


