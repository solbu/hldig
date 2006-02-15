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

void TidyParser::initialize()
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
    // kill the URLlist
    // 
    URLlist.clear();
}


void TidyParser::nodeTraverse( TidyNode tnod )
{
    //
    // Loop through every node under tnod
    //
    for( TidyNode child = tidyGetChild(tnod); child; child = tidyGetNext(child) )
    {
        /*
         * This isn't really necessary
         * 
        ctmbstr name = tidyNodeGetName( child );
        if ( !name )
        {
            switch ( tidyNodeGetType(child) )
            {
                case TidyNode_Text:
                    //
                    // This is the important one. This means there's text
                    // to insert into the CLucene document
                    //
                    name = "Text";
                    break;
                case TidyNode_Root:       name = "Root";                    break;
                case TidyNode_DocType:    name = "DOCTYPE";                 break;
                case TidyNode_Comment:    name = "Comment";                 break;
                case TidyNode_ProcIns:    name = "Processing Instruction";  break;
                case TidyNode_CDATA:      name = "CDATA";                   break;
                case TidyNode_Section:    name = "XML Section";             break;
                case TidyNode_Asp:        name = "ASP";                     break;
                case TidyNode_Jste:       name = "JSTE";                    break;
                case TidyNode_Php:        name = "PHP";                     break;
                case TidyNode_XmlDecl:    name = "XML Declaration";         break;
    
                case TidyNode_Start:
                case TidyNode_End:
                case TidyNode_StartEnd:
                default:
                    assert( name != NULL ); // Shouldn't get here
                    break;
            }
        }
         *
         *
         */

        //
        // Change states if applicable
        //
        switch ( tidyNodeGetId(child) )
        {
            case TidyTag_A:
                inAnchor = true;
                break;
            case TidyTag_H1:
                inH1 = true;
                break;
            case TidyTag_H2:
                inH2 = true;
                break;
            case TidyTag_HTML:
                inHTML = true;
                break;
            case TidyTag_HEAD:
                inHead = true;
                break;
            case TidyTag_BODY:
                inBody = true;
                break;
            case TidyTag_TITLE:
                inTitle = true;
                break;
            default:
                break;
        }
    
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
        // HTMLtidy will not nest these states (supposedly)
        // so we can just use toggles, instead of nesting
        // depth counts. ( eg: <h1><h2><h1> ... </h1></h2></h1>
        // will not occur in the HTMLtidy document tree )
        //
        switch ( tidyNodeGetId(child) )
        {
            case TidyTag_A:
                inAnchor = false;
                break;
            case TidyTag_H1:
                inH1 = false;
                break;
            case TidyTag_H2:
                inH2 = false;
                break;
            case TidyTag_HTML:
                inHTML = false;
                break;
            case TidyTag_HEAD:
                inHead = false;
                break;
            case TidyTag_BODY:
                inBody = false;
                break;
            case TidyTag_TITLE:
                inTitle = false;
                break;
            default:
                break;
        }
    }
}



