//
// SitemapParser.cc
//
// SitemapParser: interface to HTMLtidy
//
// 
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//

#include "SitemapParser.h"
#include "HtDebug.h"




SitemapParser::SitemapParser()
{
    tdoc = NULL;
    config= HtConfiguration::config();
}


SitemapParser::~SitemapParser()
{
    HtDebug * debug = HtDebug::Instance();
    debug->outlog(10, "SitemapParser destructor start\n");

    release();

    debug->outlog(10, "SitemapParser destructor done\n");
}


void SitemapParser::release()
{
    tidyRelease(tdoc);
    tdoc = NULL;
}


void SitemapParser::initialize()
{
    //
    // kill the TidyDoc
    // 
    release();

    //
    // create a new TidyDoc
    // 
    tdoc = tidyCreate();

    //
    // keep the document nice and silent
    //
    tidyOptSetBool( tdoc, TidyXmlTags, yes );
    tidyOptSetBool( tdoc, TidyShowWarnings, no );
    tidyOptSetInt( tdoc, TidyShowErrors, 0 );

    //
    // set the character encoding for sitemaps
    //
    tidySetCharEncoding( tdoc, "utf8" );
    
    //
    // set up various options that will be nice to use
    //
    tidyOptSetBool(tdoc, TidyQuoteNbsp, no);

    //
    // kill the URLlist
    // 
    URLlist.clear();
}


list < FacetCollection > SitemapParser::parseDoc(char* input)
{
    HtDebug * debug = HtDebug::Instance();
    //
    // have HTMLtidy parse the contents
    //
    debug->outlog(4, "SitemapParser: about to parse the document contents using HTMLTidy (losing debug output)");
    tidyParseString( tdoc, input );
    debug->outlog(4, " ... success\n");

    //
    // now that the tree has been built, traverse it and insert text into the CLucene doc
    //
    debug->outlog(4, "SitemapParser: about to walk the DOM tree\n");
    nodeTraverse( tidyGetRoot(tdoc) );
    debug->outlog(4, "SitemapParser: DOM tree walk successful\n");

    //
    // return the URLs seen during tree traversal
    //
    return URLlist;
}


void SitemapParser::nodeTraverse( TidyNode tnod )
{
    HtDebug * debug = HtDebug::Instance();
    //
    // Loop through every node under tnod
    //
    char nodeName[10];
    TidyNode XMLnode, data, url, urlset;

    XMLnode = tidyGetChild(tnod);
    if (tidyNodeGetType(XMLnode) != TidyNode_XmlDecl) {
        return;
    }

    urlset = tidyGetNext(XMLnode);
    ctmbstr urlsetName = tidyNodeGetName(urlset);

    if (!strcmp(urlsetName, "urlset")) {
        strcpy(nodeName, "url");
    } else if (!strcmp(urlsetName, "sitemapindex")) {
        strcpy(nodeName, "sitemap");
    } else {
        return;
    }

    for (url = tidyGetChild(urlset); url; url = tidyGetNext(url))
    {
        FacetCollection newFacet;
        ctmbstr urlName = tidyNodeGetName(url);

        if (strcmp(urlName, nodeName))
        {
            continue;
        }

        for (data = tidyGetChild(url); data; data = tidyGetNext(data))
        {
            ctmbstr dataName = tidyNodeGetName(data);

            if (!strcmp(dataName, "loc") || !strcmp(dataName, "lastmod"))
            {
                TidyBuffer buf;
                TidyNode textChild = tidyGetChild(data);

                tidyBufInit(&buf);
                tidyNodeGetText(tdoc, textChild, &buf);

                int length = strlen((char *)buf.bp) - 1;
                while (length >= 0 && ((buf.bp[length] == '\n') || (buf.bp[length] == '\r')))
                    buf.bp[length--] = '\0';

                if (!strcmp(dataName, "loc"))
                {
                    debug->outlog(7, "URL = [%s]\n", (char *)buf.bp);
                    newFacet.setURL((char *)buf.bp);
                }
                else if (!strcmp(dataName, "lastmod"))
                {
                    newFacet.setTime((char *)buf.bp);

                    debug->outlog(7, "Date = %s (%d)\n", (char *)buf.bp, (int)newFacet.getTime());
                }
            }
            else if (!strcmp(dataName, "textFacet"))
            {
                TidyNode facetData;
                for (facetData = tidyGetChild(data); facetData; facetData = tidyGetNext(facetData))
                {
                    TidyAttr nameAttr = tidyAttrGetNAME( facetData );
                    ctmbstr facetName = tidyNodeGetName( facetData );

                    if (tidyAttrIsNAME(nameAttr) && !strcmp(facetName, "textField"))
                    {
                        TidyBuffer buf;
                        char * nameVal = (char*)tidyAttrValue(nameAttr);

                        tidyBufInit(&buf);
                        TidyNode textChild = tidyGetChild(facetData);
                        tidyNodeGetText(tdoc, textChild, &buf);

                        int length = strlen((char *)buf.bp) - 1;
                        while (length >= 0 && ((buf.bp[length] == '\n') || (buf.bp[length] == '\r')))
                            buf.bp[length--] = '\0';

                        debug->outlog(7, "    %s = [%s] [%s]\n", facetName, nameVal, (char *)buf.bp);

                        newFacet.addFacet(nameVal, (char *)buf.bp);
                    }
                }
            }
            else
            {
                debug->outlog(7, "Not processing %s node\n", dataName);
            }
        }

        URLlist.push_back(newFacet);
    }
}

