//
// ExternalParser.h
//
// ExternalParser: Allows external programs to parse unknown document formats.
//                 The parser is expected to return the document in a 
//                 specific format. The format is documented 
//                 in http://www.htdig.org/attrs.html#external_parser
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: ExternalParser.h,v 1.8 2004/05/28 13:15:14 lha Exp $
//

#ifndef _ExternalParser_h_
#define _ExternalParser_h_

#include "Parsable.h"
#include "htString.h"

#include <stdio.h>

class URL;


class ExternalParser : public Parsable
{
public:
    //
    // Construction/Destruction
    //
                        ExternalParser(char *contentType);
    virtual    ~ExternalParser();

    //
    // Main parser interface.
    //
    virtual void  parse(Retriever &retriever, URL &);

    //
    // Check if the given contentType has an external parser associated
    // with it
    //
    static int    canParse(char *contentType);
    
private:
    String    currentParser;
    String    contentType;

    int      readLine(FILE *, String &);
};

#endif


