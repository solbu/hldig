//
// HtWordReference.cc
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtWordReference.cc,v 1.2.2.1 2000/05/06 20:46:37 loic Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

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
	  (char*)Word(),
	  DocID(),
	  Flags(),
	  Location(),
	  Anchor());
  return OK;
}

