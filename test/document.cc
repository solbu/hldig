//
// document.cc
//
// document: Query the document database
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: document.cc,v 1.2 2002/02/01 22:49:37 ghutchis Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <unistd.h>
#include <stdlib.h>
#include <iostream.h>
#include <stdio.h>

// If we have this, we probably want it.
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include "defaults.h"
#include "DocumentDB.h"

typedef struct {
  char* config;
  int urls;
  int docids;
} params_t;

static void usage();
static void dodoc(params_t* params);

static int verbose = 0;

//*****************************************************************************
// int main(int ac, char **av)
//
int main(int ac, char **av)
{
  int			c;
  extern char		*optarg;
  params_t		params;

  params.config = strdup("???");
  params.urls = 0;
  params.docids = 0;

  while ((c = getopt(ac, av, "vudc:")) != -1)
    {
      switch (c)
	{
	case 'v':
	  verbose++;
	  break;
	case 'u':
	  params.urls = 1;
	  break;
	case 'd':
	  params.docids = 1;
	  break;
	case 'c':
	  free(params.config);
	  params.config = strdup(optarg);
	  break;
	case '?':
	  usage();
	  break;
	}
    }

  dodoc(&params);

  free(params.config);

  return 0;
}

static void dodoc(params_t* params)
{
  HtConfiguration* const config= HtConfiguration::config();
  config->Defaults(&defaults[0]);
  config->Read(params->config);

  DocumentDB docs;
  if(docs.Read(config->Find("doc_db"), config->Find("doc_index"), config->Find("doc_excerpt")) < 0) {
    cerr << "dodoc: cannot open\n";
    exit(1);
  }

  List* docids = docs.DocIDs();
  IntObject* docid = 0;
  for(docids->Start_Get(); (docid = (IntObject*)docids->Get_Next()); ) {
    if(params->docids) cout << docid->Value();
    if(params->urls) {
      if(params->docids) cout << " ";
      DocumentRef* docref = docs[docid->Value()];
      cout << docref->DocURL();
      cout << "\n";
      delete docref;
    }
  }
  delete docids;
}

//*****************************************************************************
// void usage()
//   Display program usage information
//
static void usage()
{
    cout << "usage: word [options]\n";
    cout << "Options:\n";
    cout << "\t-v\t\tIncreases the verbosity\n";
    cout << "\t-u\t\tShow URLs\n";
    cout << "\t-dl\t\tShow DocIDs\n";
    cout << "\t-c file\tspecify the config file to load\n";
    exit(0);
}

