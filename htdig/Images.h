//
// Images.h
//
// $Id: Images.h,v 1.1.1.1 1997/02/03 17:11:05 turtle Exp $
//
// $Log: Images.h,v $
// Revision 1.1.1.1  1997/02/03 17:11:05  turtle
// Initial CVS
//
//
#ifndef _Images_h_
#define _Images_h_

#include <Dictionary.h>

class Images
{
public:
	//
	// Construction/Destruction
	//
					Images();
					~Images();

	int				Sizeof(char *url);

private:
	Dictionary		images;
};

#endif


