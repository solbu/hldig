//
// Images.h
//
// Images: Issue an HTTP request to retrieve the size of an image from
//         the content-length field.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Images.h,v 1.3 1999/09/11 05:03:50 ghutchis Exp $
//

#ifndef _Images_h_
#define _Images_h_

#include "Dictionary.h"

class Images
{
public:
	//
	// Construction/Destruction
	//
	Images();
	~Images();

	int	Sizeof(char *url);

private:
	Dictionary		images;
};

#endif


