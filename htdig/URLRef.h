//
// URLRef.h
//
// $Id: URLRef.h,v 1.2 1997/03/24 04:33:18 turtle Exp $
//
// $Log: URLRef.h,v $
// Revision 1.2  1997/03/24 04:33:18  turtle
// Renamed the String.h file to htString.h to help compiling under win32
//
// Revision 1.1.1.1  1997/02/03 17:11:06  turtle
// Initial CVS
//
//
#ifndef _URLRef_h_
#define _URLRef_h_

#include <Object.h>
#include <htString.h>


class URLRef : public Object
{
public:
	//
	// Construction/Destruction
	//
	                URLRef();
	                ~URLRef();

	char			*URL()					{return url;}
	int				HopCount()				{return hopcount;}
	char			*Referer()				{return referer;}
	
	void			URL(char *s)			{url = s;}
	void			HopCount(int h)			{hopcount = h;}
	void			Referer(char *ref)		{referer = ref;}
	
private:
	String			url;
	String			referer;
	int				hopcount;
};

#endif


