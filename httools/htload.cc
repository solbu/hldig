//
// htload.cc
//
// htload: A utility to read ASCII text versions of the document
// and/or word databases. These can be used by external programs,
// edited, or used as a platform and version-independent form of the DB.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: htload.cc,v 1.6 2004/05/28 13:15:25 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "WordContext.h"
#include "HtURLCodec.h"
#include "HtWordList.h"
#include "HtConfiguration.h"
#include "DocumentDB.h"
#include "defaults.h"

#include <errno.h>

#ifndef _MSC_VER                /* _WIN32 */
#include <unistd.h>
#endif

// If we have this, we probably want it.
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#elif HAVE_GETOPT_LOCAL
#include <getopt_local.h>
#endif

int verbose = 0;

void usage ();
void reportError (char *msg);

//*****************************************************************************
// int main(int ac, char **av)
//
int
main (int ac, char **av)
{
  int do_words = 1;
  int do_docs = 1;
  int alt_work_area = 0;
  String configfile = DEFAULT_CONFIG_FILE;
  int c;
  extern char *optarg;

  while ((c = getopt (ac, av, "vdwc:a")) != -1)
  {
    switch (c)
    {
    case 'c':
      configfile = optarg;
      break;
    case 'v':
      verbose++;
      break;
    case 'a':
      alt_work_area++;
      break;
    case 'w':
      do_words = 0;
      break;
    case 'd':
      do_docs = 0;
      break;
    case '?':
      usage ();
      break;
    }
  }

  HtConfiguration *config = HtConfiguration::config ();
  config->Defaults (&defaults[0]);

  if (access ((char *) configfile, R_OK) < 0)
  {
    reportError (form ("Unable to find configuration file '%s'",
                       configfile.get ()));
  }

  config->Read (configfile);

  //
  // Check url_part_aliases and common_url_parts for
  // errors.
  String url_part_errors = HtURLCodec::instance ()->ErrMsg ();

  if (url_part_errors.length () != 0)
    reportError (form ("Invalid url_part_aliases or common_url_parts: %s",
                       url_part_errors.get ()));


  // We may need these through the methods we call
  if (alt_work_area != 0)
  {
    String configValue;

    configValue = config->Find ("word_db");
    if (configValue.length () != 0)
    {
      configValue << ".work";
      config->Add ("word_db", configValue);
    }

    configValue = config->Find ("doc_db");
    if (configValue.length () != 0)
    {
      configValue << ".work";
      config->Add ("doc_db", configValue);
    }

    configValue = config->Find ("doc_index");
    if (configValue.length () != 0)
    {
      configValue << ".work";
      config->Add ("doc_index", configValue);
    }

    configValue = config->Find ("doc_excerpt");
    if (configValue.length () != 0)
    {
      configValue << ".work";
      config->Add ("doc_excerpt", configValue);
    }
  }

  if (do_docs)
  {
    const String doc_list = config->Find ("doc_list");
    DocumentDB docs;
    if (docs.Open (config->Find ("doc_db"), config->Find ("doc_index"),
                   config->Find ("doc_excerpt")) == OK)
    {
      docs.LoadDB (doc_list, verbose);
      docs.Close ();
    }
  }
  if (do_words)
  {

    // Initialize htword
    WordContext::Initialize (*config);

    const String word_dump = config->Find ("word_dump");
    HtWordList words (*config);
    if (words.Open (config->Find ("word_db"), O_RDWR) == OK)
    {
      words.Load (word_dump);
      words.Close ();
    }
  }

  return 0;
}


//*****************************************************************************
// void usage()
//   Display program usage information
//
void
usage ()
{
  cout << "usage: htload [-v][-d][-w][-a][-c configfile]\n";
  cout << "This program is part of ht://Dig " << VERSION << "\n\n";
  cout << "Options:\n";
  cout << "\t-v\tVerbose mode.  This increases the verbosity of the\n";
  cout << "\t\tprogram.  Using more than 2 is probably only useful\n";
  cout << "\t\tfor debugging purposes.  The default verbose mode\n";
  cout << "\t\tgives a progress on what it is doing and where it is.\n\n";
  cout << "\t-d\tDo NOT load the document database.\n\n";
  cout << "\t-w\tDo NOT load the word database.\n\n";
  cout << "\t-a\tUse alternate work files.\n";
  cout << "\t\tTells htload to append .work to the database files \n";
  cout << "\t\tallowing it to operate on a second set of databases.\n";
  cout << "\t-c configfile\n";
  cout << "\t\tUse the specified configuration file instead on the\n";
  cout << "\t\tdefault.\n\n";
  exit (0);
}


//*****************************************************************************
// Report an error and die
//
void
reportError (char *msg)
{
  cout << "htload: " << msg << "\n\n";
  exit (1);
}
