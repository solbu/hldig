//
// WeightWord.h
//
// WeightWord: Contains the information necessary for a particular search word
//             including the resulting weight (scaling factor) and 
//             whether the word should be hidden (ignored).
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WeightWord.h,v 1.4 1999/09/10 17:22:25 ghutchis Exp $
//

#ifndef _WeightWord_h_
#define _WeightWord_h_

#include "htString.h"
#include "WordRecord.h"

class WeightWord : public Object
{
public:
    //
    // Construction/Destruction
    //
    WeightWord();
    WeightWord(char *word, double weight);
    WeightWord(WeightWord *);
    
    virtual		~WeightWord();

    void		set(char *word);

    String		word;
    double		weight;
    WordRecord		*records;
    int			isExact;
    int			isHidden;
    int			isIgnore;
};

#endif


