//
// WordReference.cc
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordReference.cc,v 1.1 1999/09/10 11:45:29 loic Exp $
//

#include "WordReference.h"

//
// Explain the format of data output of the Dump function
//
int WordReference::DumpHeader(FILE *fl)
{
  fprintf(fl, "#word\tdocument id\tflags\tlocation\tanchor\n");
  return OK;
}

//
// Ascii representation of a word occurence.
//
int WordReference::Dump(FILE *fl)
{
  fprintf(fl, "%s\t%ld\t%ld\t%d\t%d\n",
	  Word.get(),
	  DocumentID,
	  Flags,
	  Location,
	  Anchor);
  return OK;
}
