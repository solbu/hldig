//
// HtWordList.h
//
// HtWordList: Specialized WordList class that can hold a list 
//	       of words waiting to be inserted in the database.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtWordList.h,v 1.2.2.5 2000/09/25 03:58:47 ghutchis Exp $
//

#ifndef _HtWordList_h_
#define _HtWordList_h_

#include <fcntl.h>

#include"HtConfiguration.h"
#include "WordList.h"

class HtWordList : public WordList
{
public:
    //
    // Construction/Destruction
    //
    HtWordList(const Configuration  & config_arg);
    HtWordList(const HtConfiguration& config_arg);
    virtual ~HtWordList();
    
    //
    // Update/add a word, perform sanity checking and
    // fill information.
    //
    void		Replace(const WordReference& wordRef);

    //
    // Skip this document -- ignore all words stored in the object
    //  from this document
    //
    void		Skip();

    //
    // Flush the words stored in the object to the database
    //
    void		Flush();

    // Write an ascii version of the word database in <filename>
    int			Dump(const String& filename);

    // Read in an ascii version of the word database in <filename>
    int			Load(const String& filename);

    // Get the WordContext for this WordList (to be used for WordRefs, etc.)
    WordContext		*GetWordContext() { return &context; }

private:

    WordContext			context;
    List			*words;
};

#endif


