//
// Parsable.h
//
// Parsable: Base class for file parsers (HTML, PDF, ExternalParser ...)
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Parsable.h,v 1.8 2003/02/11 09:49:37 lha Exp $
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
    void addString(Retriever& retriever, char *s, int& wordindex, int slot);
    void addKeywordString(Retriever& retriever,  char *s, int& wordindex);
	
protected:
    String		*contents;
    int			max_head_length;
    int			max_description_length;
    int			max_meta_description_length;
    int			max_keywords, keywordsCount;
    unsigned int	minimum_word_length;
};

#endif


