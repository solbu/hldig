//
// WeightWord.h
//
// $Id: WeightWord.h,v 1.2 1997/03/24 04:33:24 turtle Exp $
//
// $Log: WeightWord.h,v $
// Revision 1.2  1997/03/24 04:33:24  turtle
// Renamed the String.h file to htString.h to help compiling under win32
//
// Revision 1.1.1.1  1997/02/03 17:11:05  turtle
// Initial CVS
//
// Revision 1.1  1996/01/03 19:02:00  turtle
// Before rewrite
//
//
#ifndef _WeightWord_h_
#define _WeightWord_h_

#include <htString.h>
#include <WordRecord.h>

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


