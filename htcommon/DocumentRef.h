//
// DocumentRef.h
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
// $Id: DocumentRef.h,v 1.29.2.6 2006/03/01 23:45:18 aarnone Exp $
//

#ifndef _DocumentRef_h_
#define _DocumentRef_h_

#include <map>
#include <set>
#include <time.h>

#include "CLuceneAPI.h"

typedef std::set<std::string> uniqueWordsSet;

class DocumentRef
{
public:
    //
    // Construction/Destruction
    //
    DocumentRef();
    ~DocumentRef();

    void        initialize();               // Clear out everything, and set up the hash
    
    void        dumpUniqueWords();          // insert all of the unique words into the contents
    void        addUniqueWord(char* word);  // add a unique word
    void        insertField(const char* fieldName, const char* fieldValue);
    void        appendField(const char* fieldName, const char* fieldValue);

    CL_Doc*     contents() {return &indexDoc;}  // return a pointer to the the indexDoc
    
protected:

    //
    // New hotness hash that contains everything
    // 
    CL_Doc indexDoc;

    // This will contain unique words (if that option is enabled)
    // before the document is put into the index, this needs to 
    // be flushed to the contents field
    // 
    uniqueWordsSet uniqueWords;

};

#endif


