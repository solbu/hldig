//
// Exact.cc
//
// Implementation of Exact
//
//
#if RELEASE
static char RCSid[] = "$Id: Exact.cc,v 1.5 1999/07/10 02:10:57 ghutchis Exp $";
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
    String	stripped = w;
    HtStripPunctuation(stripped);

    words.Add(new String(stripped));
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




