//
// TidyParser.cc
//
// TidyParser: interface to HTMLtidy
//
// 
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//

#include "TidyParser.h"




TidyParser::TidyParser()
{
    tdoc = NULL;
}


TidyParser::~TidyParser()
{
    release();
}


void TidyParser::release()
{
    tidyRelease(tdoc);
    tdoc = NULL;
}


void TidyParser::initialize(DocumentRef * CluceneDocPointer, char* encoding)
{
    //
    // reset all flags
    // 
    inHead = false;
    inTitle = false;
    inBody = false;
    inHeading = false;

    //
    // kill the TidyDoc
    // 
    release();

    //
    // create a new TidyDoc
    // 
    tdoc = tidyCreate();

    //
    // set up the encoding for this doc
    //
    if (encoding)
    {
        tidySetCharEncoding( tdoc, encoding );
    }
    else
    {
        tidySetCharEncoding( tdoc, "utf8" );
    }

    //
    // keep the document nice and silent
    //
    tidyOptSetBool( tdoc, TidyShowWarnings, no );
    tidyOptSetInt( tdoc, TidyShowErrors, 0 );
    
    //
    // kill the URLlist
    // 
    URLlist.clear();

    //
    // set the pointer and clean the Clucene doc
    //
    CLuceneDoc = CluceneDocPointer; 

    //
    // the robots control variables
    //
    noIndex = false;
    noFollow = false;
}


std::set<std::string> TidyParser::parseDoc(char* input)
{
    //
    // have HTMLtidy parse the contents
    //
    tidyParseString( tdoc, input );

    //
    // now that the tree has been built, traverse
    // it and insert text into the CLucene doc
    //
    nodeTraverse( tidyGetRoot(tdoc) );

    //
    // the easiest way to implement no follow is to
    // simply erase the outgoing links
    //
    if (noFollow)
        URLlist.clear();

    //
    // return the URLs seen during tree traversal
    //
    return URLlist;
}


void TidyParser::nodeTraverse( TidyNode tnod )
{
    //
    // Loop through every node under tnod
    //
    for( TidyNode child = tidyGetChild(tnod); child; child = tidyGetNext(child) )
    {
        //
        // Change states if applicable
        //
        stateChanger(child, true);
    
        //
        // If this is a text node, insert the text into the CLucene document
        //
        if ( tidyNodeIsText( child ))
        {
            TidyBuffer buf;
            
            tidyBufInit(&buf);
            tidyNodeGetText(tdoc, child, &buf);
            if (buf.bp)
            {
                if (inTitle)
                {
                    CLuceneDoc->appendField("doc-title", (char*)buf.bp);
                    CLuceneDoc->appendField("title", (char*)buf.bp);
                }
                if (inHeading)
                {
                    CLuceneDoc->appendField("heading", (char*)buf.bp);
                }
                else
                {
                    CLuceneDoc->appendField("contents", (char*)buf.bp);
                }

                CLuceneDoc->appendField("stemmed", (char*)buf.bp);
                CLuceneDoc->appendField("synonym", (char*)buf.bp);
            }
            tidyBufFree(&buf);
        }
        else if (tidyNodeIsMETA(child))
        {
            //
            // find the meta name
            //
            TidyAttr metaAttr = tidyAttrGetNAME( child );
            if (tidyAttrIsNAME(metaAttr))
            {
                char * metaVal = (char*)tidyAttrValue(metaAttr);
                if (metaVal)
                {
                    if (!strcmp(metaVal, "htdig-noindex"))
                    {
                        //
                        // the content does not matter, so only check the name
                        //
                        noIndex = true;
                        break;
                    }

                    //
                    // find the meta content
                    //
                    TidyAttr contentAttr = tidyAttrGetCONTENT( child );
                    if (tidyAttrIsCONTENT(contentAttr))
                    {
                        char * contentVal = (char*)tidyAttrValue(contentAttr);
                        if (contentVal)
                        {

                            if (!strcmp(metaVal, "htdig-keywords") || 
                                    !strcmp(metaVal, "keywords"))
                            {
                                //
                                // keywords
                                //
                                CLuceneDoc->appendField("keywords", contentVal);

                                CLuceneDoc->appendField("stemmed", contentVal);
                                CLuceneDoc->appendField("synonym", contentVal);
                            }
                            else if (!strcmp(metaVal, "description"))
                            {
                                //
                                // meta description
                                // 
                                CLuceneDoc->appendField("doc-meta-desc", contentVal);
                                CLuceneDoc->appendField("meta-desc", contentVal);

                                CLuceneDoc->appendField("stemmed", contentVal);
                                CLuceneDoc->appendField("synonym", contentVal);
                            }
                            else if (!strcmp(metaVal, "author"))
                            {
                                //
                                // author
                                //
                                CLuceneDoc->appendField("doc-author", contentVal);
                                CLuceneDoc->appendField("author", contentVal);
                            }
                            else if (!strcmp(metaVal, "htdig-email"))
                            {
                                //
                                // notification email address
                                //
                                CLuceneDoc->appendField("doc-email", contentVal);
                            }
                            else if (!strcmp(metaVal, "htdig-email-subject"))
                            {
                                //
                                // notification email subject
                                //
                                CLuceneDoc->appendField("doc-email-subject", contentVal);
                            }
                            else if (!strcmp(metaVal, "htdig-notification-date"))
                            {
                                //
                                // notification email date
                                //
                                CLuceneDoc->appendField("doc-email-date", contentVal);
                            }
                            else if (!strcmp(metaVal, "robots"))
                            {
                                //
                                // robot options
                                //
                            }
                        }
                    }
                }
            }
        }
        else if (tidyNodeIsA(child))
        {
            //
            // An anchor tag. Put the URL (if it exists) into
            // the URLlist for return later. Also, put the alt 
            // text (if any) into the doc contents
            //
            TidyAttr anchorAttr = tidyAttrGetHREF( child );
            
            if (tidyAttrIsHREF(anchorAttr))
            {
                URLlist.insert(tidyAttrValue(anchorAttr));
            }
            else
            {
                //printf("found anchor, can't get HREF attr");
            }

            anchorAttr = tidyAttrGetALT( child );
            if (tidyAttrIsALT(anchorAttr))
            {
                char * altVal = (char*)tidyAttrValue(anchorAttr);
                if (altVal)
                {
                    CLuceneDoc->appendField("contents", altVal);
                    CLuceneDoc->appendField("stemmed", altVal);
                    CLuceneDoc->appendField("synonym", altVal);
                }
            }
        }

        //
        // Recurse down from here
        //
        nodeTraverse( child );

        //
        // Leaving the node, so the state can be reset.
        // 
        stateChanger(child, false);
    }
}

void TidyParser::stateChanger(TidyNode tnod, bool newState)
{
    //
    // HTMLtidy will not nest these states (supposedly)
    // so we can just use toggles, instead of a count of
    // nesting depth. ( eg: <h1><h2><h1> ... </h1></h2></h1>
    // will not occur in the HTMLtidy document tree )
    //
    switch ( tidyNodeGetId(tnod) )
    {
        case TidyTag_HEAD:
            inHead = newState;
            break;
        case TidyTag_BODY:
            inBody = newState;
            break;
        case TidyTag_TITLE:
            inTitle = newState;
            break;
        case (TidyTag_H1 || TidyTag_H2 || TidyTag_H3 ||
                TidyTag_H4 || TidyTag_H5 || TidyTag_H6 ||
                TidyTag_B):
            inHeading = newState;
            break;
        default:
            break;
    }
}


