//
// ExternalParser.h
//
// $Id: ExternalParser.h,v 1.2 1997/03/24 04:33:16 turtle Exp $
//
// $Log: ExternalParser.h,v $
// Revision 1.2  1997/03/24 04:33:16  turtle
// Renamed the String.h file to htString.h to help compiling under win32
//
// Revision 1.1.1.1  1997/02/03 17:11:06  turtle
// Initial CVS
//
//
#ifndef _ExternalParser_h_
#define _ExternalParser_h_

#include "Parsable.h"
#include <htString.h>
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


