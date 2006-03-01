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


void TidyParser::initialize(char* encoding, char* docURL, char* docTime)
{
    //
    // reset all flags
    // 
    inHTML = false;
    inHead = false;
    inTitle = false;
    inBody = false;
    inAnchor = false;
    inH1 = false;
    inH2 = false;

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
    tidySetCharEncoding( tdoc, encoding );

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
    // Add the URL to the CLucene doc
    //
    CLuceneDoc->insertField("url", docURL);

    //
    // add the time to the CLucene doc
    //
    CLuceneDoc->insertField("time", docTime);

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
    // return the URLs seen during tree traversal
    //
    return URLlist;
}


void TidyParser::commitDoc()
{
    CLuceneAddDocToIndex(CLuceneDoc->contents());
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
                    //
                    // in the title
                    //
                    CLuceneDoc->insertField("title", (char*)buf.bp);
                }
                else
                {
                    //
                    // in the body (or just not the title)
                    //
                    CLuceneDoc->appendField("contents", (char*)buf.bp);
                }
            }
            tidyBufFree(&buf);
        }
        else if (tidyNodeIsA(child))
        {
            //
            // An anchor tag. Put the URL (if it exists) into
            // the URLlist for return later. If it doesn't exist,
            // just skip it.
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
        case TidyTag_A:
            inAnchor = newState;
            break;
        case TidyTag_H1:
            inH1 = newState;
            break;
        case TidyTag_H2:
            inH2 = newState;
            break;
        case TidyTag_HTML:
            inHTML = newState;
            break;
        case TidyTag_HEAD:
            inHead = newState;
            break;
        case TidyTag_BODY:
            inBody = newState;
            break;
        case TidyTag_TITLE:
            inTitle = newState;
            break;
        default:
            break;
    }
}


