//
// WeightWord.h
//
// WeightWord: Contains the information necessary for a particular search word
//             including the resulting weight (scaling factor) and 
//             whether the word should be hidden (ignored).
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: WeightWord.h,v 1.7 2003/06/24 19:58:07 nealr Exp $
//

#ifndef _WeightWord_h_
#define _WeightWord_h_

#include "htString.h"
#include "WordRecord.h"
#include "HtWordReference.h"	// for FLAG_...

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
    
    virtual		~WeightWord();

    void		set(char *word);

    String		word;
    double		weight;
    WordRecord		*records;
    unsigned int	flags;
    short int		isExact;
    short int		isHidden;
    short int		isIgnore;
};

#endif


