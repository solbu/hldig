//
// URLRef.h
//
// URLRef: A definition of a URL/Referer pair with associated hopcount
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: URLRef.h,v 1.6.2.2 2000/10/20 03:40:56 ghutchis Exp $
//
//
#ifndef _URLRef_h_
#define _URLRef_h_

#include "Object.h"
#include "htString.h"
#include "URL.h"

class URLRef : public Object
{
public:
	//
	// Construction/Destruction
	//
	                URLRef();
	                ~URLRef();

	const URL	&GetURL() const			{return url;}
	int		GetHopCount() const		{return hopcount;}
	const URL	&GetReferer() const		{return referer;}
	
	void		SetURL(const URL &u)	        {url = u;}
	void		SetHopCount(int h)		{hopcount = h;}
	void		SetReferer(const URL &ref)	{referer = ref;}

	int		compare(const Object& to) const	{ return compare((const URLRef&) to); }
	int		compare(const URLRef& to) const;
	
private:
	URL		url;
	URL		referer;
	int		hopcount;
};

#endif


