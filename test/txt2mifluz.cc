//
// txt2mifluz.cc
//
// txt2mifluz: stress test the Berkeley DB database and WordList interface.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: txt2mifluz.cc,v 1.4 2004/05/28 13:15:30 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include <htconfig.h>
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

// If we have this, we probably want it.
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif /* HAVE_GETOPT_H */
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif /* HAVE_MALLOC_H */
#include <stdlib.h>

#include <htString.h>
#include <WordList.h>
#include <WordContext.h>

/*
 * Store all options from the command line
 */
class params_t
{
public:
  char* dbfile;
  int compress;
};

/*
 * Explain options
 */
static void usage();
/*
 * Verbosity level set with -v (++)
 */
static int verbose = 0;

class WordSearch {
public:
  WordSearch();

  WordKey *Search(const String& expr);
  WordKey *Search(const StringList& terms);
  WordKey *Search(WordKey* keys);

  WordKey* Terms2WordKey(const StringList& terms);

  void DocumentSet(const WordKey& from, WordKey& to);
  void DocumentCompare(const WordKey& a, const WordKey& b);

  int limit_bottom;
  int limit_count;
  WordList* words;
};

WordSearch::WordSearch()
{
  limit_bottom = 0;
  limit_count = 0;
  words = 0;
}

WordKey *WordSearch::Search(const String& expr)
{
  return Search(StringList(expr, " \t"));
}

WordKey *WordSearch::Search(const StringList& terms)
{
  return Search(Terms2WordKey(terms));
}

WordKey *WordSearch::Search(WordKey* keys)
{
  //  WordKey* AscendingFrequency(ter);
  return 0;
}

WordKey* WordSearch::Terms2WordKey(const StringList& terms)
{
  WordKey* keys = new WordKey[terms.Count() + 1];

  int i;
  String* term;
  ListCursor cursor;
  terms.Start_Get(cursor);
  for(i = 0; (term = (String*)terms.Get_Next(cursor)); i++) {
    keys[i].SetWord(*term);
  }

  for(i = 0; !keys[i].Empty(); i++) {
    fprintf(stderr, "%s\n", (char*)keys[i].Get());
  }
  return 0;
}


// *****************************************************************************
// Entry point
//
int main(int ac, char **av)
{
  int			c;
  extern char		*optarg;
  params_t		params;

  params.compress = 0;
  params.dbfile = strdup("test");

  while ((c = getopt(ac, av, "vB:f:z")) != -1)
    {
      switch (c)
	{
	case 'v':
	  verbose++;
	  break;
	case 'B':
	  free(params.dbfile);
	  params.dbfile = strdup(optarg);
	  break;
	case 'z':
	  params.compress = 1;
	  break;
	case '?':
	  usage();
	  break;
	}
    }

  Configuration* config = WordContext::Initialize();
  if(!config) {
    fprintf(stderr, "txt2mifluz: no config file found\n");
    exit(1);
  }

  if(params.compress) {
    config->Add("wordlist_compress", "true");
  }

  WordList words(*config);
  words.Open(params.dbfile, O_RDWR|O_TRUNC);
  int inserted = words.Read(stdin);
  if(verbose)
    printf("inserted %d WordReferences\n", inserted);
  words.Close();

  delete config;
  free(params.dbfile);
}

// *****************************************************************************
// void usage()
//   Display program usage information
//
static void usage()
{
    printf("usage: txt2mifluz [options] < txtfile\n");
    printf("Options:\n");
    printf("\t-v\t\tIncreases the verbosity\n");
    printf("\t-B dbfile\tuse <dbfile> as a db file name (default test).\n");
    exit(0);
}
