//
// Parsable.h
//
// $Id: Parsable.h,v 1.2 1997/03/24 04:33:16 turtle Exp $
//
// $Log: Parsable.h,v $
// Revision 1.2  1997/03/24 04:33:16  turtle
// Renamed the String.h file to htString.h to help compiling under win32
//
// Revision 1.1.1.1  1997/02/03 17:11:06  turtle
// Initial CVS
//
//
#ifndef _Parsable_h_
#define _Parsable_h_

#include <htString.h>
#include "Retriever.h"

class URL;


class Parsable
{
public:
    //
    // Construction/Destruction
    //
                        Parsable();
    virtual		~Parsable();

    //
    // Main parser interface.
    //
    virtual void	parse(Retriever &retriever, URL &) = 0;

    //
    // The rest of the members are used by the Document to provide us
    // the data that we contain.
    //
    virtual void	setContents(char *data, int length);
	
protected:
    String		*contents;
    char		*valid_punctuation;
    int			max_head_length;
    int			max_description_length;
};

#endif


