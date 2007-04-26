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
#include "HtDebug.h"




TidyParser::TidyParser()
{
    tdoc = NULL;
    config= HtConfiguration::config();
}


TidyParser::~TidyParser()
{
    HtDebug * debug = HtDebug::Instance();
    debug->outlog(10, "TidyParser destructor start\n");

    release();

    debug->outlog(10, "TidyParser destructor done\n");
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
    inScript = false;
    inStyle = false;

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
    // set up various options that will be nice to use
    //
    tidyOptSetBool(tdoc, TidyQuoteNbsp, no);

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
    singleNoIndex = false;
}


std::set<std::string> TidyParser::parseDoc(char* input)
{
    HtDebug * debug = HtDebug::Instance();
    //
    // have HTMLtidy parse the contents
    //
    debug->outlog(4, "TidyParser: about to parse the document contents using HTMLTidy (losing debug output)");
    tidyParseString( tdoc, input );
    debug->outlog(4, " ... success\n");

    //
    // now that the tree has been built, traverse it and insert text into the CLucene doc
    //
    debug->outlog(4, "TidyParser: about to walk the DOM tree\n");
    nodeTraverse( tidyGetRoot(tdoc), 0 );
    debug->outlog(4, "TidyParser: DOM tree walk successful\n");

    //
    // the easiest way to implement no follow is to simply erase the outgoing links
    //
    if (noFollow)
        URLlist.clear();

    //
    // return the URLs seen during tree traversal
    //
    return URLlist;
}


void TidyParser::nodeTraverse( TidyNode tnod, int depth )
{
    HtDebug * debug = HtDebug::Instance();
    //
    // Loop through every node under tnod
    //
    debug->outlog(5, "Looping through nodes at depth = %d\n", depth);
    for( TidyNode child = tidyGetChild(tnod); child; child = tidyGetNext(child) )
    {
        //
        // Change states if applicable
        //
        debug->outlog(7, "stateChanger true\n");
        stateChanger(child, true);
    
        //
        // If this is a text node, insert the text into the CLucene document
        //
        if (tidyNodeIsText( child ))
        {
            if ( !inStyle && !inScript )
            {
                debug->outlog(6, "Text node");

                TidyBuffer buf;

                tidyBufInit(&buf);
                tidyNodeGetText(tdoc, child, &buf);
                if (buf.bp)
                {
                    if (inTitle)
                    {
                        CLuceneDoc->appendField("doc-title", (char*)buf.bp);
                        CLuceneDoc->appendField("title", (char*)buf.bp);

                        debug->outlog(6, " (inTitle)");
                    }
                    else
                    {
                        CLuceneDoc->appendField("contents", (char*)buf.bp);

                        if (inHeading)
                        {
                            CLuceneDoc->appendField("heading", (char*)buf.bp);
                            debug->outlog(5, " (inHeading)");
                        }
                    }

                    //
                    // careful! this will print LOTS of text to the outlog
                    //
                    debug->outlog(7, ": %s", (char*)buf.bp);

                    if (config->Boolean("use_stemming"))
                        CLuceneDoc->appendField("stemmed", (char*)buf.bp);
                    if (config->Boolean("use_synonyms"))
                        CLuceneDoc->appendField("synonym", (char*)buf.bp);
                }
                tidyBufFree(&buf);

                debug->outlog(6, "\n");
            }
            else
            {
                debug->outlog(6, "Text node ignored (style or script)");
                if (debug->getLevel() > 7)
                {
                    TidyBuffer buf;

                    tidyBufInit(&buf);
                    tidyNodeGetText(tdoc, child, &buf);
                    if (buf.bp)
                    {
                        debug->outlog(7, ": %s", (char*)buf.bp);
                    }
                }
                debug->outlog(6, "\n");
            }
        }
        else if (tidyNodeIsMETA(child))
        {
            debug->outlog(6, "Meta node");

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
                        debug->outlog(6, " (no-index)");
                        noIndex = true;
                    }
                    else
                    {
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

                                    if (config->Boolean("use_stemming"))
                                        CLuceneDoc->appendField("stemmed", contentVal);
                                    if (config->Boolean("use_synonyms"))
                                        CLuceneDoc->appendField("synonym", contentVal);
                                    debug->outlog(6, " - htdig-keywords: %s", contentVal);
                                }
                                else if (!strcmp(metaVal, "description"))
                                {
                                    //
                                    // meta description
                                    // 
                                    CLuceneDoc->appendField("meta-desc", contentVal);

                                    if (config->Boolean("use_stemming"))
                                        CLuceneDoc->appendField("stemmed", contentVal);
                                    if (config->Boolean("use_synonyms"))
                                        CLuceneDoc->appendField("synonym", contentVal);
                                    debug->outlog(6, " - description: %s", contentVal);
                                }
                                else if (!strcmp(metaVal, "author"))
                                {
                                    //
                                    // author
                                    //
                                    CLuceneDoc->appendField("doc-author", contentVal);
                                    CLuceneDoc->appendField("author", contentVal);
                                    debug->outlog(6, " - author: %s", contentVal);
                                }
                                else if (!strcmp(metaVal, "htdig-email"))
                                {
                                    //
                                    // notification email address
                                    //
                                    CLuceneDoc->appendField("doc-email", contentVal);
                                    debug->outlog(6, " - htdig-email: %s", contentVal);
                                }
                                else if (!strcmp(metaVal, "htdig-email-subject"))
                                {
                                    //
                                    // notification email subject
                                    //
                                    CLuceneDoc->appendField("doc-email-subject", contentVal);
                                    debug->outlog(6, " - htdig-email-subject: %s", contentVal);
                                }
                                else if (!strcmp(metaVal, "htdig-notification-date"))
                                {
                                    //
                                    // notification email date
                                    //
                                    CLuceneDoc->appendField("doc-email-date", contentVal);
                                    debug->outlog(6, " - htdig-notification-date: %s", contentVal);
                                }
                                else if (!strcmp(metaVal, "robots") && !singleNoIndex)
                                {
                                    //
                                    // robot options
                                    //
                                    debug->outlog(6, " robots directive - ");
                                    if (strstr(contentVal, "nofollow") != NULL)
                                    {
                                        noFollow = true;
                                        debug->outlog(6, " nofollow");
                                    }
                                    if (strstr(contentVal, "noindex") != NULL)
                                    {
                                        noIndex = true;
                                        debug->outlog(6, " noindex");
                                    }
                                    if (strstr(contentVal, "none") != NULL)
                                    {
                                        noIndex = true;
                                        noFollow = true;
                                        debug->outlog(6, " none");
                                    }
                                }
                            }
                        }
                    }
                }
            }
            debug->outlog(6, "\n");
        }
        else if (tidyNodeIsA(child))
        {
            debug->outlog(6, "Anchor node");

            //
            // An anchor tag. Put the URL (if it exists) into
            // the URLlist for return later. Also, put the alt 
            // text (if any) into the doc contents
            //
            TidyAttr anchorAttr = tidyAttrGetHREF( child );
            if (tidyAttrIsHREF(anchorAttr))
            {
                char * anchorVal = (char*)tidyAttrValue(anchorAttr);
                if (anchorVal)
                {
                    debug->outlog(6, ": [%s]", anchorVal);
                    URLlist.insert(anchorVal);
                }
                else
                {
                    debug->outlog(6, ": [unable to get anchor node HREF value]", anchorVal);
                }
            }

            //
            // and the alt text
            //
            anchorAttr = tidyAttrGetALT( child );
            if (tidyAttrIsALT(anchorAttr))
            {
                char * altVal = (char*)tidyAttrValue(anchorAttr);
                if (altVal)
                {
                    debug->outlog(6, " (ALT= %s)", altVal);
                    CLuceneDoc->appendField("contents", altVal);

                    if (config->Boolean("use_stemming"))
                        CLuceneDoc->appendField("stemmed", altVal);
                    if (config->Boolean("use_synonyms"))
                        CLuceneDoc->appendField("synonym", altVal);
                }
            }
            debug->outlog(6, "\n");
        }
        else
        {
            debug->outlog(6, "UNKNOWN node\n");
        }

        //
        // Recurse down from here
        //
        nodeTraverse( child, depth + 1 );

        //
        // Leaving the node, so the state can be reset.
        // 
        debug->outlog(6, "stateChanger false: \n");
        stateChanger(child, false);
        debug->outlog(6, "\n");
    }
    debug->outlog(5, "Finished looping through nodes at depth = %d\n", depth);
}

void TidyParser::stateChanger(TidyNode tnod, bool newState)
{
    HtDebug * debug = HtDebug::Instance();
    //
    // HTMLtidy will not nest these states (supposedly)
    // so we can just use toggles, instead of a count of
    // nesting depth. ( eg: <h1><h2><h1> ... </h1></h2></h1>
    // will not occur in the HTMLtidy document tree )
    //
    switch ( tidyNodeGetId(tnod) )
    {
        case TidyTag_HEAD:
            debug->outlog(6,"TidyTag_HEAD");
            inHead = newState;
            break;
        case TidyTag_BODY:
            debug->outlog(6,"TidyTag_BODY");
            inBody = newState;
            break;
        case TidyTag_SCRIPT:
            debug->outlog(6,"TidyTag_SCRIPT");
            inScript = newState;
            break;
        case TidyTag_STYLE:
            debug->outlog(6,"TidyTag_STYLE");
            inStyle = newState;
            break;
        case TidyTag_TITLE:
            debug->outlog(6,"TidyTag_TITLE");
            inTitle = newState;
            break;
        case (TidyTag_H1 || TidyTag_H2 || TidyTag_H3 ||
                TidyTag_H4 || TidyTag_H5 || TidyTag_H6 ||
                TidyTag_B):
            debug->outlog(6,"TidyTag_ heading");
            inHeading = newState;
            break;
        default:
            debug->outlog(6,"TidyTag_ notSpecified");
            break;
    }
}


