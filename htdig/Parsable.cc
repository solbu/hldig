//
// Parsable.cc
//
// Implementation of Parsable
//

#if RELEASE
static char RCSid[] = "$Id: Parsable.cc,v 1.2.2.1 1999/03/23 23:22:54 grdetil Exp $";
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
