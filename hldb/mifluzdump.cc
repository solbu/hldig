//
// NAME
// 
// dump the content of an inverted index.
//
// SYNOPSIS
//
// mifluzdump file
//
// DESCRIPTION
//
// mifluzdump writes on <b>stdout</b> a complete ascii description
// of the <b>file</b> inverted index using the <i>WordList::Write</i>
// method. 
//
// ENVIRONMENT
//
// <b>MIFLUZ_CONFIG</b>
// file name of configuration file read by WordContext(3). Defaults to
// <b>~/.mifluz.</b> 
// 
// 
// END
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//

#ifdef HAVE_CONFIG_H
#include "hlconfig.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <unistd.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif /* HAVE_GETOPT_H */

#include <htString.h>
#include <WordContext.h>
#include <WordList.h>
#include <Configuration.h>

static void
action (WordContext * context, const String & file)
{
  WordList *words = context->List ();
  if (words->Open (file, O_RDONLY) != OK)
    exit (1);
  if (words->Write (stdout) != OK)
    exit (1);
  if (words->Close () != OK)
    exit (1);
  delete words;
}

static void
usage ()
{
  fprintf (stderr, "usage: mifluzdump file\n");
  exit (1);
}

int
main (int argc, char *argv[])
{

  if (argc != 2)
    usage ();

  //
  // Mandatory to create global data needed for the library.
  //
  WordContext *context = new WordContext ();
  if (!context)
    exit (1);
  action (context, argv[1]);
  delete context;
}
