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
// $Id: DocumentRef.cc,v 1.53.2.5 2006/02/14 22:57:16 aarnone Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "DocumentRef.h"
#include "good_strtok.h"
#include "HtConfiguration.h"
#include "HtURLCodec.h"
#include <stdlib.h>
#include <ctype.h>

#ifdef HAVE_STD
  #include <fstream>
  #ifdef HAVE_NAMESPACES
    using namespace std;
  #endif
#else
  #include <fstream.h>
#endif /* HAVE_STD */

// extern HtConfiguration config;

//*****************************************************************************
// DocumentRef::DocumentRef()
//
DocumentRef::DocumentRef()
{
    initialize();
}


//*****************************************************************************
// DocumentRef::~DocumentRef()
//
DocumentRef::~DocumentRef()
{
}


//****************************************************************
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
    indexDoc.clear();
    uniqueWords.clear();

    indexDoc["url"].second = "Keyword";
    indexDoc["title"].second = "Keyword";
    indexDoc["author"].second = "Keyword";
    indexDoc["head"].second = "Keyword";
    indexDoc["meta_desc"].second = "Keyword";
    indexDoc["meta_email"].second = "Keyword";
    indexDoc["meta_notification"].second = "Keyword";
    indexDoc["meta_subject"].second = "Keyword";

    indexDoc["contents"].second = "UnStored";
    
    indexDoc["time"].second = "UnIndexed";
}


//*****************************************************************
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
    uniqueWordsSet::iterator i;
    for (i = uniqueWords.begin(); i != uniqueWords.end(); i++) {
        indexDoc["contents"].first.insert(indexDoc["contents"].first.size(), *i);
        indexDoc["contents"].first.push_back(' ');
    }
    uniqueWords.clear();
}


//*****************************************************************
// void addUniqueWord(const char* word)
//
// add a unique word to the uniqueWords set
//
// NOTE: will need modification when unicode goes in
// 
void DocumentRef::addUniqueWord(char* word)
{
    uniqueWords.insert(word);
}


//*****************************************************************
// void DocumentRef::insertField(const char* fieldName, const char* fieldValue)
//
// Take the specified field text/value and insert into the indexDoc
// at the specified field
//
// NOTE: will need modification when unicode goes in, the field value
// will most likely be a string object
// 
void DocumentRef::insertField(const char* fieldName, const char* fieldValue)
{
    indexDoc[fieldName].first = fieldValue;
}


//*****************************************************************
// void DocumentRef::appendField(const char* fieldName, const char* fieldValue)
//
// Append the specified field text/value to the specified field
//
// NOTE: will need modificatioin when Unicode goes in (the separator
// will need to be a wide character)
//
void DocumentRef::appendField(const char* fieldName, const char* fieldValue)
{
    indexDoc[fieldName].first.insert(indexDoc[fieldName].first.size(), fieldValue);
    indexDoc[fieldName].first.push_back(' ');
}


