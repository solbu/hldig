//
// NAME
// 
// dump the dictionnary of an inverted index.
//
// SYNOPSIS
//
// mifluzdict file
//
// DESCRIPTION
//
// mifluzdict writes on <b>stdout</b> a complete ascii description
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
// Copyright (c) 1999-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <unistd.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif /* HAVE_GETOPT_H */

#include <htString.h>
#include <WordContext.h>
#include <WordList.h>
#include <WordDict.h>
#include <Configuration.h>

typedef struct
{
    String prefix;
} params_t;

static void action(WordContext* context, const String& file, params_t* params)
{
  WordList *words = context->List();
  if(words->Open(file, O_RDONLY) != OK) exit(1);
  if(params->prefix.empty()) {
    if(words->WriteDict(stdout) != OK) exit(1);
  } else {
    WordDict *dict = words->Dict();
    WordDictCursor *cursor = dict->CursorPrefix(params->prefix);
    String word;
    WordDictRecord record;
    while(dict->NextPrefix(cursor, word, record) == 0) {
      printf("%s %d %d\n", (char*)word.get(), record.Id(), record.Count());
    }
  }
  if(words->Close() != OK) exit(1);
  delete words;
}

static void usage()
{
  fprintf(stderr, "usage: mifluzdict [-p prefix] file\n");
  exit(1);
}

int main(int argc, char *argv[])
{
  params_t params;
  extern char *optarg;
  extern int optind;
  int ch;
  while ((ch = getopt(argc, argv, "p:")) != EOF) {
    switch (ch) {
    case 'p':
      params.prefix = optarg;
      break;
    default:
      usage();
      break;
    }
  }

  if(optind != argc - 1) usage();

  //
  // Mandatory to create global data needed for the library.
  //
  WordContext *context = new WordContext();
  if(!context) exit(1);
  action(context, argv[optind], &params);
  delete context;
}

