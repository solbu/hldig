//
// Parsable.cc
//
// Parsable: Base class for file parsers (HTML, PDF, ExternalParser ...)
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Parsable.cc,v 1.5 1999/09/11 05:03:50 ghutchis Exp $
//

#include "Parsable.h"
#include "htdig.h"


//*****************************************************************************
// Parsable::Parsable()
//
Parsable::Parsable()
{
    contents = 0;
    max_head_length = config.Value("max_head_length", 0);
    max_description_length = config.Value("max_description_length", 50);
    max_meta_description_length = config.Value("max_meta_description_length", 0);
}


//*****************************************************************************
// Parsable::~Parsable()
//
Parsable::~Parsable()
{
    delete contents;
}


//*****************************************************************************
// void Parsable::setContents(char *data, int length)
//   This will set the contents of the parsable object.
//
void
Parsable::setContents(char *data, int length)
{
    delete contents;
    contents = new String(data, length);
}
