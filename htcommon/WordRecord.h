	//
// WordRecord.h
//
// $Id: WordRecord.h,v 1.1 1997/02/03 17:11:07 turtle Exp $
//
// $Log: WordRecord.h,v $
// Revision 1.1  1997/02/03 17:11:07  turtle
// Initial revision
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
				count = id = weight = anchor = location = 0;
			}
};

#endif


