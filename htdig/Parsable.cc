//
// Parsable.cc
//
// Parsable: Base class for file parsers (HTML, PDF, ExternalParser ...)
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Parsable.cc,v 1.6 2002/02/01 22:49:29 ghutchis Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "Parsable.h"
#include "htdig.h"
#include "defaults.h"


//*****************************************************************************
// Parsable::Parsable()
//
Parsable::Parsable()
{
	HtConfiguration* config= HtConfiguration::config();
    contents = 0;
    max_head_length = config->Value("max_head_length", 0);
    max_description_length = config->Value("max_description_length", 50);
    max_meta_description_length = config->Value("max_meta_description_length", 0);
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
