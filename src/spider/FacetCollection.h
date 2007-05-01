//
// FacetCollection.h
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
//
#ifndef _FacetCollection_h_
#define _FacetCollection_h_

#include "HtStdHeader.h"

typedef pair< string, string > facet_entry;
typedef list< facet_entry > facet_list;

class FacetCollection
{
    public:

        //
        // constructors / destructors
        //
        FacetCollection();
        FacetCollection(char* u);
        ~FacetCollection() {}

        void clear();

        //
        // set methods
        //
        void setURL(char* u) {_URL = u;}
        void setURL(string u) {_URL = u;}

        void setReferer(char* u) {_referer = u;}
        void setReferer(string u) {_referer = u;}

        void setTime(int t) {_time = (time_t)t;}
        void setTime(time_t t) {_time = t;}
        void setTime(string t) {setTime(t.c_str());}
        void setTime(char* t);

        void addFacet(char* fieldName, char* fieldValue);
        void addFacet(string fieldName, string fieldValue);

        //
        // get methods
        //
        string getURL() {return _URL;}

        string getReferer() {return _referer;}

        time_t getTime() {return _time;}

        facet_list getFacets() {return _facets;}


    private:

        time_t _time;
        facet_list _facets;
        string _URL;
        string _referer;
};

#endif

