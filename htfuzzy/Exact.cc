//
// Exact.cc
//
// Implementation of Exact
//
// $Log: Exact.cc,v $
// Revision 1.2  1997/03/24 04:33:18  turtle
// Renamed the String.h file to htString.h to help compiling under win32
//
// Revision 1.1.1.1  1997/02/03 17:11:12  turtle
// Initial CVS
//
//
#if RELEASE
static char RCSid[] = "$Id: Exact.cc,v 1.2 1997/03/24 04:33:18 turtle Exp $";
#endif

#include "Exact.h"
#include <htString.h>
#include <List.h>


//*****************************************************************************
// Exact::Exact()
//
Exact::Exact()
{
}


//*****************************************************************************
// Exact::~Exact()
//
Exact::~Exact()
{
}


//*****************************************************************************
void
Exact::getWords(char *w, List &words)
{
    words.Add(new String(w));
}


//*****************************************************************************
int
Exact::openIndex(Configuration &)
{
}


//*****************************************************************************
void
Exact::generateKey(char *, String &)
{
}


//*****************************************************************************
void
Exact::addWord(char *)
{
}




