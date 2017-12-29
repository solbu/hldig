//
// parsetest.cc
//
// parsetest: A program to test the ParseTree classes as replacement for the current
//            parsing code
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: parsetest.cc,v 1.4 2004/05/28 13:15:25 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "cgi.h"
#include "defaults.h"
#include "ParseTree.h"
#include "AndParseTree.h"
#include "OrParseTree.h"
#include "ExactParseTree.h"
#include "WordContext.h"

// If we have this, we probably want it.
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

void reportError (char *msg);
void usage ();

int debug = 0;

//*****************************************************************************
// int main()
//
int
main (int ac, char **av)
{
  int c;
  extern char *optarg;
  int override_config = 0;
  List *searchWords = NULL;
  String configFile = DEFAULT_CONFIG_FILE;
  String logicalWords;

  //
  // Parse command line arguments
  //
  while ((c = getopt (ac, av, "c:dv")) != -1)
  {
    switch (c)
    {
    case 'c':
      configFile = optarg;
      override_config = 1;
      break;
    case 'v':
      debug++;
      break;
    case 'd':
      debug++;
      break;
    case '?':
      usage ();
      break;
    }
  }

  //
  // Parse the CGI parameters.
  //
  char none[] = "";
  cgi input (optind < ac ? av[optind] : none);

  String originalWords = input["words"];
  originalWords.chop (" \t\r\n");

  // Set up the config
  config.Defaults (&defaults[0]);

  if (access ((char *) configFile, R_OK) < 0)
  {
    reportError (form ("Unable to find configuration file '%s'",
                       configFile.get ()));
  }

  config.Read (configFile);

  // Initialize htword library (key description + wordtype...)
  WordContext::Initialize (config);

  ParseTree *testParse;

  testParse = new ParseTree;
  if (testParse->Parse (originalWords) != NOTOK)
  {
    cout << "Parsing as a boolean query... " << endl;
    cout << "Initial Query:" << testParse->GetQuery () << endl;
    cout << "Logical Words:" << testParse->GetLogicalWords () << endl;
  }
  else
    cout << "Parsing as a boolean query FAILED" << endl;
  delete testParse;

  testParse = new AndParseTree;
  if (testParse->Parse (originalWords) != NOTOK)
  {
    cout << "Parsing as an AND query... " << endl;
    cout << "Initial Query:" << testParse->GetQuery () << endl;
    cout << "Logical Words:" << testParse->GetLogicalWords () << endl;
  }
  else
    cout << "Parsing as an AND query FAILED" << endl;
  delete testParse;

  testParse = new OrParseTree;
  if (testParse->Parse (originalWords) != NOTOK)
  {
    cout << "Parsing as an OR query... " << endl;
    cout << "Initial Query:" << testParse->GetQuery () << endl;
    cout << "Logical Words:" << testParse->GetLogicalWords () << endl;
  }
  else
    cout << "Parsing as an OR query FAILED" << endl;
  delete testParse;

  testParse = new ExactParseTree;
  if (testParse->Parse (originalWords) != NOTOK)
  {
    cout << "Parsing as an EXACT query... " << endl;
    cout << "Initial Query:" << testParse->GetQuery () << endl;
    cout << "Logical Words:" << testParse->GetLogicalWords () << endl;
  }
  else
    cout << "Parsing as an EXACT query FAILED" << endl;
  delete testParse;

}

//*****************************************************************************
// void usage()
//   Display program usage information--assumes we're running from a cmd line
//
void
usage ()
{
  cout << "usage: parsetest [-v][-d][-c configfile]\n";
  cout << "This program is part of ht://Dig " << VERSION << "\n\n";
  cout << "Options:\n";
  cout << "\t-v -d\tVerbose mode.  This increases the verbosity of the\n";
  cout << "\t\tprogram.  Using more than 2 is probably only useful\n";
  cout << "\t\tfor debugging purposes.  The default verbose mode\n";
  cout << "\t\tgives a progress on what it is doing and where it is.\n\n";
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
  cout << "parsetest: " << msg << "\n\n";
  exit (1);
}
