//
// Exact.cc
//
// Implementation of Exact
//
//
#if RELEASE
static char RCSid[] = "$Id: Exact.cc,v 1.4 1999/05/05 00:41:02 ghutchis Exp $";
#endif

#include "Exact.h"
#include "htString.h"
#include "List.h"


//*****************************************************************************
// Exact::Exact()
//
Exact::Exact()
{
  name = "exact";
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
  return 0;
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




