//
// Images.h
//
// Images: Issue an HTTP request to retrieve the size of an image from
//         the content-length field.
//
// $Id: Images.h,v 1.2 1999/09/08 17:11:16 loic Exp $
//
// $Log: Images.h,v $
// Revision 1.2  1999/09/08 17:11:16  loic
// update comments
//
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


