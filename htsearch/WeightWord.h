//
// WeightWord.h
//
// WeightWord: Contains the information necessary for a particular search word
//             including the resulting weight (scaling factor) and 
//             whether the word should be hidden (ignored).
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WeightWord.h,v 1.5 2002/02/01 22:49:35 ghutchis Exp $
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


