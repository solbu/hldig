//
// Parsable.h
//
// Parsable: Base class for file parsers (HTML, PDF, ExternalParser ...)
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Parsable.h,v 1.6 1999/09/11 05:03:50 ghutchis Exp $
//

#ifndef _Parsable_h_
#define _Parsable_h_

#include "htString.h"
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
    int			max_head_length;
    int			max_description_length;
    int			max_meta_description_length;
};

#endif


