//
// ExternalParser.h
//
// $Id: ExternalParser.h,v 1.1 1997/02/03 17:11:06 turtle Exp $
//
// $Log: ExternalParser.h,v $
// Revision 1.1  1997/02/03 17:11:06  turtle
// Initial revision
//
//
#ifndef _ExternalParser_h_
#define _ExternalParser_h_

#include "Parsable.h"
#include <String.h>
#include <stdio.h>
class URL;


class ExternalParser : public Parsable
{
public:
    //
    // Construction/Destruction
    //
                        ExternalParser(char *contentType);
    virtual		~ExternalParser();

    //
    // Main parser interface.
    //
    virtual void	parse(Retriever &retriever, URL &);

    //
    // Check if the given contentType has an external parser associated
    // with it
    //
    static int		canParse(char *contentType);
    
private:
    String		currentParser;
    String		contentType;

    int			readLine(FILE *, String &);
};

#endif


