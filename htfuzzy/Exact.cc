//
// Exact.cc
//
// Exact: The exact-match "fuzzy" matching. Simply returns the word (minus punctuation)
//           
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Exact.cc,v 1.8 2000/02/19 05:29:02 ghutchis Exp $
//

#include "Exact.h"
#include "htString.h"
#include "List.h"


//*****************************************************************************
// Exact::Exact()
//
Exact::Exact(const HtConfiguration& config_arg) :
  Fuzzy(config_arg)
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
Exact::openIndex()
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




