//
// HtWordList.h
//
// HtWordList: Specialized WordList class that can hold a list 
//	       of words waiting to be inserted in the database.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: HtWordList.h,v 1.6 2004/05/28 13:15:12 lha Exp $
//

#ifndef _HtWordList_h_
#define _HtWordList_h_

#include <fcntl.h>
#include <stdlib.h>

#include"HtConfiguration.h"
#include "WordList.h"

class HtWordList : public WordList
{
public:
    //
    // Construction/Destruction
    //
    HtWordList(const Configuration  & config_arg) : WordList(config_arg) 
	{
	    cerr << "HtWordList::HtWordList(Configuration) is not valid" << endl; 
	    abort();
	}
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

private:

    List			*words;
};

#endif


