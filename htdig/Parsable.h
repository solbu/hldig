//
// Parsable.h
//
// $Id: Parsable.h,v 1.1 1997/02/03 17:11:06 turtle Exp $
//
// $Log: Parsable.h,v $
// Revision 1.1  1997/02/03 17:11:06  turtle
// Initial revision
//
//
#ifndef _Parsable_h_
#define _Parsable_h_

#include <String.h>
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


