//
// DocumentRef.cc
//
// DocumentRef: Reference to an indexed document. Keeps track of all
//              information stored on the document, either by the dig 
//              or temporary search information.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: DocumentRef.cc,v 1.1.2.3 2007/05/01 22:47:01 aarnone Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */
#include "HtDebug.h"

#include "DocumentRef.h"



// *****************************************************************************
// DocumentRef::DocumentRef()
//
DocumentRef::DocumentRef()
{
    initialize();
}


// *****************************************************************************
// DocumentRef::~DocumentRef()
//
DocumentRef::~DocumentRef()
{
    HtDebug * debug = HtDebug::Instance();
    debug->outlog(10, "DocumentRef transport destructor start\n");

    debug->outlog(10, "DocumentRef destructor done\n");
}


// ****************************************************************
// void DocumentRef::initialize()
//
// Set up the indexDocument with the required hash fields. If these
// aren't set up, the cppAPI won't accept them (hopefully). Also,
// delete anything that might still be in the document.
//
// NOTE: this can be improved... we should probably use an enum
// here instead of all these silly strings. mmm... silly string.
// 
void DocumentRef::initialize()
{
    //uniqueWords.clear();

    indexDoc["doc-name"].first.clear();
    indexDoc["doc-size"].first.clear();
    indexDoc["doc-alt-time"].first.clear();
    indexDoc["doc-expires"].first.clear();

    indexDoc["doc-title"].first.clear();
    indexDoc["doc-time"].first.clear();

    indexDoc["doc-meta-email"].first.clear();
    indexDoc["doc-meta-email-date"].first.clear();
    indexDoc["doc-meta-email-subject"].first.clear();

    indexDoc["contents"].first.clear();
    indexDoc["keywords"].first.clear();
    indexDoc["heading"].first.clear();
    indexDoc["title"].first.clear();
    indexDoc["meta-desc"].first.clear();
    indexDoc["backlink"].first.clear();

    indexDoc["stemmed"].first.clear();
    indexDoc["synonym"].first.clear();

    indexDoc["url"].first.clear();
    indexDoc["author"].first.clear();
    indexDoc["id"].first.clear();

    //
    // these fields will be returned verbatim - they are not for
    // searching. these fields should not be very large, as they
    // will bloat the index (any field that starts with 'doc-' should
    // be like this).
    //
    indexDoc["doc-name"].second = "UnIndexed";
    indexDoc["doc-size"].second = "UnIndexed";
    indexDoc["doc-alt-time"].second = "UnIndexed";
    indexDoc["doc-expires"].second = "UnIndexed";

    indexDoc["doc-title"].second = "Keyword";       // needed for sorting
    indexDoc["doc-time"].second = "Keyword";        // needed for sorting

    indexDoc["doc-meta-email"].second = "UnIndexed";
    indexDoc["doc-meta-email-date"].second = "UnIndexed";
    indexDoc["doc-meta-email-subject"].second = "UnIndexed";

    //
    // thse fields will be used in actual searching
    //
    indexDoc["contents"].second = "Text"; // must be in the index for highlighting
    indexDoc["keywords"].second = "UnStored";
    indexDoc["heading"].second = "UnStored";
    indexDoc["title"].second = "UnStored";
    indexDoc["meta-desc"].second = "UnStored";
    indexDoc["backlink"].second = "UnStored";

    //
    // optional fields (can be turned on or off from configuration)
    //
    //indexDoc["stemmed"].second = "UnStored";
    indexDoc["stemmed"].second = "UnStored";
    indexDoc["synonym"].second = "UnStored";

    //
    // these are special fields. they should be searchable, and may be returned.
    // They can be declared as text, since there is a special analyzer specified
    // for these field names (or at least there should be in the CLuceneAPI and
    // the searching code).
    //
    indexDoc["url"].second = "Text";
    indexDoc["author"].second = "Text";
    indexDoc["id"].second = "Text";
}


// *****************************************************************
// void DocumentRef::dumpUniqueWords()
//
// Take the unique words that have been stored so far and append each
// to the contents field of the indexDoc. then clear out the unique words.
// 
// NOTE: will need modification when unicode goes in (might be able to
// replace with a bunch of appendField calls)
// 
void DocumentRef::dumpUniqueWords()
{
//    uniqueWordsSet::iterator i;
//    for (i = uniqueWords.begin(); i != uniqueWords.end(); i++) {
//        indexDoc["contents"].first.insert(indexDoc["contents"].first.size(), *i);
//        indexDoc["contents"].first.push_back(' ');
//    }
//    uniqueWords.clear();
}


// *****************************************************************
// void addUniqueWord(const char* word)
//
// add a unique word to the uniqueWords set
//
// NOTE: will need modification when unicode goes in
// 
void DocumentRef::addUniqueWord(char* word)
{
//    uniqueWords.insert(word);
}


// *****************************************************************
// void DocumentRef::insertField(const char* fieldName, const char* fieldValue)
//
// Take the specified field text/value (in UTF8) and insert 
// into the indexDoc at the specified field, clearing the
// field first.
//
void DocumentRef::insertField(const char* fieldName, const char* fieldValue)
{
    indexDoc[fieldName].first.clear();

    appendField(fieldName, fieldValue);
}    


// *****************************************************************
// void DocumentRef::appendField(const char* fieldName, const char* fieldValue)
//
// Append the specified field text/value to the specified field.
// Do not use utf8_to_wchar here, since that creates a copy of the data. use
// push_back on individual wchar_t's to conserve space
//
void DocumentRef::appendField(const char* fieldName, const char* fieldValue)
{
    wchar_t ucs2_char;
    const char * counter = fieldValue;
    HtDebug * debug = HtDebug::Instance();

    debug->outlog(5, "DocumentRef: Adding to CL_Doc field \"%s\": [%s]\n", fieldName, fieldValue);

    while (counter[0])
    {
        if ((counter[0] & 0x80) == 0x00)
        {
            ucs2_char = *counter++;
            indexDoc[fieldName].first.push_back(ucs2_char);
        }
        else if ((counter[1] & 0xC0) == 0x80) 
        {
            if ((counter[0] & 0xE0) == 0xC0)
            {
                ucs2_char = ((counter[0] & 0x1F) << 6) | (counter[1] & 0x3F);
                indexDoc[fieldName].first.push_back(ucs2_char);
                counter += 2;
            }
            else if (((counter[2] & 0xC0) == 0x80) && ((counter[0] & 0xF0) == 0xE0))
            {
                ucs2_char = ((counter[0] & 0x0F) << 12) |
                    ((counter[1] & 0x3F) << 6) |
                    (counter[2] & 0x3F);
                indexDoc[fieldName].first.push_back(ucs2_char);
                counter += 3;
            }
            else
            {
                counter++; // invalid character
            }
        }
        else
        {
            counter++; // invalid character
        }
    }
    indexDoc[fieldName].first.push_back(' ');
}


// ******************************************************************
// char * DocumentRef::getField(const char* fieldName)
//
// return the contents of an already stored field, converted back to utf8
// 
char * DocumentRef::getField(const char* fieldName)
{
    //if (indexDoc[fieldName].first.length() == 0)
    //    return NULL;
        
    return wchar_to_utf8(indexDoc[fieldName].first.c_str());
}
