//
// BasicDocument.cc
//
// 2/6/2002 created for libhtdig to simplify & mimic Document.cc
//
// Neal Richter nealr@rightnow.com
//
//
// BasicDocument: This class holds everything there is to know about a document.
//           The actual contents of the document may or may not be present at
//           all times for memory conservation reasons.
//
//           This is a basic extensable container for plain text holding documents.
//
//           Uses any Parser with parse method handling this class.
//           
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library Public License version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: BasicDocument.cc,v 1.1 2003/04/09 00:51:55 nealr Exp $
//
//--------------------------------------------------------------------

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>

#include "BasicDocument.h"
#include "TextCollector.h"
#include "StringList.h"
#include "htdig.h"
#include "Plaintext.h"
#include "HTML.h"
#include "ExternalParser.h"
#include "lib.h"

#include "defaults.h"

#if 1
typedef void (*SIGNAL_HANDLER) (...);
#else
typedef SIG_PF SIGNAL_HANDLER;
#endif

//*****************************************************************************
// BasicDocument::BasicDocument(char *loc)
//   Initialize with the given loc-parameter as the location for this document.
//   If the max_size is given, use that for size, otherwise use the
//   config value.
//
BasicDocument::BasicDocument(char *loc, int suggested_size)
{
    int temp_size = 0;

    id = 0;
    location = 0;
    title = 0;
    metacontent = 0;
    contents = 0;
    document_length = 0;


    HtConfiguration *config = HtConfiguration::config();

    //We probably need to move assignment of max_doc_size, according
    //to a configuration value. 

    if (suggested_size > 0)
        temp_size = suggested_size;
    else
        temp_size = config->Value("max_doc_size");

    contents.allocate(temp_size + 100);

    contentType = "";

    if (loc)
    {
        Location(loc);
    }
}


//*****************************************************************************
// BasicDocument::~BasicDocument()
//
BasicDocument::~BasicDocument()
{
    // We delete only the derived class objects

#if MEM_DEBUG
    char *p = new char;
    cout << "==== BasicDocument deleted: " << this << " new at " << ((void *) p) << endl;
    delete p;
#endif
}


//*****************************************************************************
// void BasicDocument::Reset()
//   Restore the BasicDocument object to an initial state.
//
void
BasicDocument::Reset()
{

    id = 0;
    location = 0;
    title = 0;
    metacontent = 0;
    contents = 0;

    contentType = 0;
    document_length = 0;

}

//*****************************************************************************
// void BasicDocument::Length()
//   Return/Calc length of BasicDocument... icummulative size of the Strings
//
int
BasicDocument::Length()
{
    if (document_length < 0)
    {
        document_length = 0;
        document_length += location.length();
        document_length += title.length();
        document_length += metacontent.length();
        document_length += contents.length();
        document_length += id.length();
    }

    return (document_length);
}


//*****************************************************************************
// Parsable *BasicDocument::getParsable()
//   Given the content-type of a document, returns a document parser.
//   This will first look through the list of user supplied parsers and
//   then at our (limited) builtin list of parsers.  The user supplied
//   parsers are external programs that will be used.

Parsable *
BasicDocument::getParsable()
{
    static HTML *html = 0;
    static Plaintext *plaintext = 0;
    static ExternalParser *externalParser = 0;

    Parsable *parsable = 0;

    if (ExternalParser::canParse(contentType))
    {
        if (externalParser)
        {
            delete externalParser;
        }
        externalParser = new ExternalParser(contentType);
        parsable = externalParser;
    }
    else if (mystrncasecmp((char *) contentType, "text/html", 9) == 0)
    {
        if (!html)
            html = new HTML();
        parsable = html;
    }
    else if (mystrncasecmp((char *) contentType, "text/plain", 10) == 0)
    {
        if (!plaintext)
            plaintext = new Plaintext();
        parsable = plaintext;
    }
    else if (mystrncasecmp((char *) contentType, "text/css", 8) == 0)
    {
        return NULL;
    }
    else if (mystrncasecmp((char *) contentType, "text/", 5) == 0)
    {
        if (!plaintext)
            plaintext = new Plaintext();
        parsable = plaintext;
        if (debug > 1)
        {
            cout << '"' << contentType << "\" not a recognized type.  Assuming text/plain\n";
        }
    }
    else
    {
        if (debug > 1)
        {
            cout << '"' << contentType << "\" not a recognized type.  Ignoring\n";
        }
        return NULL;
    }

    parsable->setContents(contents.get(), contents.length());
    return parsable;
}

//*****************************************************************************
//
//  Test for self parseaable
//
int
BasicDocument::SelfParseable()
{

    if (mystrncasecmp((char *) contentType, "text/vnd.customdocument", 10) == 0)
    {
        return (TRUE);
    }
    else
        return (FALSE);

}


//*****************************************************************************
// Parsable *BasicDocument::internalParser()
int     
BasicDocument::internalParser(TextCollector & textcollector)
{
    HtConfiguration* config= HtConfiguration::config();
    char *position = NULL;
    static int minimumWordLength = config->Value("minimum_word_length", 3);
    int wordIndex = 1;
    String word;
    int letter_count = 0;

    //First Process Title
    textcollector.got_title((char *) title);

    //Next Process Contents
    position = contents;

    while (*position)
    {
        word = 0;

        if (HtIsStrictWordChar(*position))
        {
            //
            // Start of a word.  Try to find the whole thing
            //
            //TODO NEAL RICHTER  Imposed a 50-letter word length limit here
            //
            while (*position && HtIsWordChar(*position) && (letter_count < 50))
            {
                word << *position;
                position++;
                letter_count++;
            }

            letter_count = 0;
            if (word.length() >= minimumWordLength)
            {
                textcollector.got_word((char *) word, wordIndex++, 0);
            }
        }
        
        if (*position)
            position++;

    }//end while

    textcollector.got_head((char*) contents);

      //Third, Process MetaContent
    position = metacontent;
    textcollector.got_meta_dsc(metacontent);
    

    //max_meta_description_length???
    
    while (*position)
    {
        word = 0;

        if (HtIsStrictWordChar(*position))
        {
            //
            // Start of a word.  Try to find the whole thing
            //
            while (*position && HtIsWordChar(*position) && (letter_count < 50))
            {
                word << *position;
                position++;
                letter_count++;
            }

            letter_count = 0;

            if (word.length() >= minimumWordLength)
            {
                textcollector.got_word((char *) word, wordIndex++, 9);
            }
        }
        
        if (*position)
            position++;

    }//end while

    return(1);
}
