//
// Parsable.h
//
// $Id: Plaintext.h,v 1.1.1.1 1997/02/03 17:11:06 turtle Exp $
//
// $Log: Plaintext.h,v $
// Revision 1.1.1.1  1997/02/03 17:11:06  turtle
// Initial CVS
//
//
#ifndef _Plaintext_h_
#define _Plaintext_h_

#include "Parsable.h"

class URL;


class Plaintext : public Parsable
{
public:
    //
    // Construction/Destruction
    //
                        Plaintext();
    virtual		~Plaintext();

    //
    // Main parser interface.
    //
    virtual void	parse(Retriever &retriever, URL &);
	
private:
};

#endif


