//
// WeightWord.h
//
// WeightWord: Contains the information necessary for a particular search word
//             including the resulting weight (scaling factor) and 
//             whether the word should be hidden (ignored).
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: WeightWord.h,v 1.8 2004/05/28 13:15:24 lha Exp $
//

#ifndef _WeightWord_h_
#define _WeightWord_h_

#include "htString.h"
#include "WordRecord.h"
#include "HtWordReference.h"  // for FLAG_...

class WeightWord : public Object
{
public:
    //
    // Construction/Destruction
    //
    WeightWord();
    WeightWord(char *word, double weight);
    WeightWord(char *word, double weight, unsigned int flags);
    WeightWord(WeightWord *);
    
    virtual    ~WeightWord();

    void    set(char *word);

    String    word;
    double    weight;
    WordRecord    *records;
    unsigned int  flags;
    short int    isExact;
    short int    isHidden;
    short int    isIgnore;
};

#endif


