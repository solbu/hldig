//
// HtWordReference.cc
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtWordReference.cc,v 1.1 1999/10/01 15:19:28 loic Exp $
//

#include "HtWordReference.h"
#include <stdio.h>

//
// Explain the format of data output of the Dump function
//
int HtWordReference::DumpHeader(FILE *fl)
{
  fprintf(fl, "#word\tdocument id\tflags\tlocation\tanchor\n");
  return OK;
}

//
// Ascii representation of a word occurence.
//
int HtWordReference::Dump(FILE *fl) const
{
  fprintf(fl, "%s\t%d\t%d\t%d\t%d\n",
	  (const char*)Word(),
	  DocID(),
	  Flags(),
	  Location(),
	  Anchor());
  return OK;
}

