//
// Parsable.cc
//
// Implementation of Parsable
//
// $Log: Parsable.cc,v $
// Revision 1.1.1.1  1997/02/03 17:11:06  turtle
// Initial CVS
//
//
#if RELEASE
static char RCSid[] = "$Id: Parsable.cc,v 1.1.1.1 1997/02/03 17:11:06 turtle Exp $";
#endif

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
    valid_punctuation = config["valid_punctuation"];
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
