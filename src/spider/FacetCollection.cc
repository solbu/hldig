//
// FacetCollection.cc
//
// FacetCollection: provides an object to represent a URL entry in
//               a SitemapPlus XML file
// 
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
#include "FacetCollection.h"



FacetCollection::FacetCollection()
{
    _time = 0;
}


FacetCollection::FacetCollection(char * u)
{
    _URL = u;
    _time = 0;
}


void FacetCollection::clear()
{
    _time = 0;
    _URL.clear();
    _referer.clear();
    _facets.clear();
}


//
// assumes W3C Datetime format (http://www.w3.org/TR/NOTE-datetime),
// which is the expected format of Sitemap timestamps
//
void FacetCollection::setTime(char* t)
{
    struct tm tm;

    memset(&tm, 0, sizeof(struct tm));
    strptime(t, "%Y-%m-%dT%H:%M:%S %Z", &tm);
    _time = mktime(&tm);
}


void FacetCollection::addFacet(char* fieldName, char* fieldValue)
{
    facet_entry newFacet;

    newFacet.first = fieldName;
    newFacet.second = fieldValue;

    _facets.push_back(newFacet);
}


void FacetCollection::addFacet(string fieldName, string fieldValue)
{
    facet_entry newFacet;

    newFacet.first = fieldName;
    newFacet.second = fieldValue;

    _facets.push_back(newFacet);
}


