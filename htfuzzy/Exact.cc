//
// Exact.cc
//
// Implementation of Exact
//
// $Log: Exact.cc,v $
// Revision 1.3.2.1  2001/06/07 18:42:22  grdetil
// * htfuzzy/Exact.cc (Exact), htfuzzy/Prefix.cc (Prefix): Set the
//   name field to the class name, as suggested by Jesse.
//
// Revision 1.3  1998/08/03 16:50:37  ghutchis
//
// Fixed compiler warnings under -Wall
//
// Revision 1.2  1997/03/24 04:33:18  turtle
// Renamed the String.h file to htString.h to help compiling under win32
//
// Revision 1.1.1.1  1997/02/03 17:11:12  turtle
// Initial CVS
//
//
#if RELEASE
static char RCSid[] = "$Id: Exact.cc,v 1.3.2.1 2001/06/07 18:42:22 grdetil Exp $";
#endif

#include "Exact.h"
#include <htString.h>
#include <List.h>


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




