//
// Exact.cc
//
// Implementation of Exact
//
// $Log: Exact.cc,v $
// Revision 1.1  1997/02/03 17:11:12  turtle
// Initial revision
//
//
#if RELEASE
static char RCSid[] = "$Id: Exact.cc,v 1.1 1997/02/03 17:11:12 turtle Exp $";
#endif

#include "Exact.h"
#include <String.h>
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




