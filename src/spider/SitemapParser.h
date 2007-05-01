//
// SitemapParser.h
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
//

#ifndef _SitemapParser_h_
#define _SitemapParser_h_

#include "HtStdHeader.h"
#include "DocumentRef.h"
#include "CLuceneAPI.h"
#include "FacetCollection.h"
#include "HtConfiguration.h"

//
// include the two HTMLtidy headers
// 
#include "tidy.h"
#include "buffio.h"

class SitemapParser
{
    public:

    //
    // Construction/Destruction
    //
    SitemapParser();
    ~SitemapParser();

    //
    // get the TidyDoc all set up and ready to parse a buffer of text. 
    // also clear the URLlist and reset all the position flags
    //
    void initialize();

    //
    // parse the buffer, return a set of URLs seen during parsing
    // 
    list<FacetCollection>  parseDoc(char*);


    private:

    //
    // parsed HTML document
    // 
    TidyDoc tdoc;

    //
    // list of sitemap entries seen during parsing
    //
    list <FacetCollection> URLlist;

    //
    // configuration for using in parsing
    //
    HtConfiguration *config;

    //
    // call to traverse XML document tree 
    //
    void nodeTraverse( TidyNode tnod );
    
    //
    // kill the TidyDoc
    //
    void release();

};

#endif


