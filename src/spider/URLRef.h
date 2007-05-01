//
// URLRef.h
//
// URLRef: A definition of a URL/Referer pair with associated hopcount
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: URLRef.h,v 1.1.2.2 2007/05/01 22:52:46 aarnone Exp $
//
//
#ifndef _URLRef_h_
#define _URLRef_h_

#include "Object.h"
#include "htString.h"
#include "URL.h"
#include "HtStdHeader.h"
#include "FacetCollection.h"

class URLRef : public Object
{
public:
    //
    // Construction/Destruction
    //
    URLRef();
    ~URLRef();

    const URL   &GetURL() const             {return url;}
    int         GetHopCount() const         {return hopcount;}
    const URL   &GetReferer() const         {return referer;}
    facet_list  GetFacets()                 {return _facets;}
    time_t      GetTime()                   {return _time;}

    void        SetURL(const URL &u)        {url = u;}
    void        SetHopCount(int h)          {hopcount = h;}
    void        SetReferer(const URL &ref)	{referer = ref;}
    void        SetFacets(facet_list f)     {_facets = f;}
    void        SetTime(time_t t)           {_time = t;}

    void        AddBacklink(string text);

    int         compare(const Object& to) const	{ return compare((const URLRef&) to); }
    int         compare(const URLRef& to) const;

private:
    URL         url;
    URL         referer;
    int         hopcount;

    facet_list  _facets;
    time_t      _time;
};

#endif


