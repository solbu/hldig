//
// hldig.cc
//
// hldig: Indexes the web sites specified in the config file
//        generating several databases to be used by htmerge
//
// Part of the ht://Dig package   <https://andy5995.github.io/hldig/>
// Copyright (c) 2017 The hl://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "Document.h"
#include "Retriever.h"
#include "StringList.h"
#include "hldig.h"
#include "defaults.h"
#include "HtURLCodec.h"
#include "WordContext.h"
#include "HtDateTime.h"
#include "HtURLRewriter.h"

////////////////////////////
// For cookie jar
////////////////////////////
#include "HtCookieJar.h"
#include "HtCookieMemJar.h"
#include "HtCookieInFileJar.h"
#include "HtHTTP.h"
////////////////////////////

// If we have this, we probably want it.
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#elif HAVE_GETOPT_LOCAL
#include <getopt_local.h>
#endif

#ifdef HAVE_STD
#include <iostream>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif
#else
#include <iostream.h>
#endif /* HAVE_STD */

//
// Global variables
//
int debug = 0;
int report_statistics = 0;
DocumentDB docs;
HtRegexList limits;
HtRegexList limitsn;
FILE *urls_seen = NULL;
FILE *images_seen = NULL;
String configFile = DEFAULT_CONFIG_FILE;
String minimalFile = 0;
HtDateTime StartTime;
HtDateTime EndTime;

void usage ();
void reportError (String msg);


//
// Start of the program.
//
int
main (int ac, char **av)
{
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  int c;
  extern char *optarg;
  String credentials;
  int initial = 0;
  int alt_work_area = 0;
  int create_text_database = 0;
  const char *max_hops = 0;

  // Cookie jar dynamic creation.
  HtCookieJar *_cookie_jar = new HtCookieMemJar ();     // new cookie jar
  if (_cookie_jar)
    HtHTTP::SetCookieJar (_cookie_jar);

//extern int yydebug;
//yydebug=1;

  //
  // Parse command line arguments
  //
  while ((c = getopt (ac, av, "lsm:c:vith:u:a")) != -1)
  {
    unsigned int pos;
    switch (c)
    {
    case 'c':
      configFile = optarg;
      break;
    case 'v':
      debug++;
      break;
    case 'i':
      initial++;
      break;
    case 't':
      create_text_database++;
      break;
    case 'h':
      max_hops = optarg;
      break;
    case 's':
      report_statistics++;
      break;
    case 'u':
      credentials = optarg;
      for (pos = 0; pos < strlen (optarg); pos++)
        optarg[pos] = '*';
      break;
    case 'a':
      alt_work_area++;
      break;
    case 'm':
      minimalFile = optarg;
      max_hops = "0";
      break;
    case '?':
      usage ();
    default:
      break;
    }
  }

  // Shows Start Time
  if (debug > 0)
    cout << _("hldig Start Time: ") << StartTime.GetAscTime () << endl;

  //
  // First set all the defaults and then read the specified config
  // file to override the defaults.
  //
  HtConfiguration *const config = HtConfiguration::config ();
  config->Defaults (&defaults[0]);
  if (access ((char *) configFile, R_OK) < 0)
  {
    reportError (form (_("Unable to find configuration file '%s'"),
                       configFile.get ()));
  }
  config->Read (configFile);

  // Warn user if any obsolete options are found in config file
  // For efficiency, check all fields here.  If different config
  // files are used for searching, obsolete options may remain
  const char *deprecatedOptions[] = {
    "heading_factor_1", "heading_factor_2", "heading_factor_3",
    "heading_factor_4", "heading_factor_5", "heading_factor_6",
    "modification_time_is_now", "pdf_parser", "translate_amp",
    "translate_lt_gt", "translate_quot", "uncoded_db_compatible",
    ""                          // empty terminator
  };
  const char **option;
  for (option = deprecatedOptions; **option; option++)
  {
    if (!config->Find (*option).empty ())
      cout << _("Warning: Configuration option ") << *option <<
        _(" is no longer supported\n");
  }

  if (config->Find (_("locale")).empty () && debug > 0)
    cout << _("Warning: unknown locale!\n");

  if (max_hops)
  {
    config->Add (_("max_hop_count"), max_hops);
  }

  // Set up credentials for this run
  if (credentials.length ())
    config->Add (_("authorization"), credentials);

  //
  // Check url_part_aliases and common_url_parts for
  // errors.
  String url_part_errors = HtURLCodec::instance ()->ErrMsg ();

  if (url_part_errors.length () != 0)
    reportError (form (_("Invalid url_part_aliases or common_url_parts: %s"),
                       url_part_errors.get ()));

  //
  // Check url_rewrite_rules for errors.
  String url_rewrite_rules = HtURLRewriter::instance ()->ErrMsg ();

  if (url_rewrite_rules.length () != 0)
    reportError (form (_("Invalid url_rewrite_rules: %s"),
                       url_rewrite_rules.get ()));

  //
  // If indicated, change the database file names to have the .work
  // extension
  //
  if (alt_work_area != 0)
  {
    String configValue = config->Find ("doc_db");

    if (configValue.length () != 0)
    {
      configValue << ".work";
      config->Add ("doc_db", configValue);
    }

    configValue = config->Find ("word_db");
    if (configValue.length () != 0)
    {
      configValue << ".work";
      config->Add ("word_db", configValue);
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

    configValue = config->Find ("md5_db");
    if (configValue.length () != 0)
    {
      configValue << ".work";
      config->Add ("md5_db", configValue);
    }
  }

  // Imports the cookies file
  const String CookiesInputFile = config->Find ("cookies_input_file");
  if (CookiesInputFile.length ())
  {
    if (debug > 0)
      cout << _("Importing Cookies input file ") << CookiesInputFile << endl;
    int result;
    HtCookieJar::SetDebugLevel (debug); // Set the debug level
    HtCookieInFileJar *cookie_file =
      new HtCookieInFileJar (CookiesInputFile, result);
    if (cookie_file)
    {
      if (!result)
      {
        if (debug > 0)
          cookie_file->ShowSummary ();
        delete _cookie_jar;     // Deletes previous cookie jar
        _cookie_jar = (HtCookieJar *) cookie_file;      // set the imported one
        HtHTTP::SetCookieJar (_cookie_jar);     // and set the new HTTP jar
      }
      else if (debug > 0)
        cout << _("Warning: Import failed! (") << CookiesInputFile << ")" <<
          endl;
    }
    else
      reportError (form (_("Unable to load cookies file '%s' in memory"),
                         CookiesInputFile.get ()));
  }

  //
  // If needed, we will create a list of every URL we come across.
  //
  if (config->Boolean ("create_url_list"))
  {
    const String filename = config->Find ("url_list");
    urls_seen = fopen (filename, initial ? "w" : "a");
    if (urls_seen == 0)
    {
      reportError (form ("Unable to create URL file '%s'", filename.get ()));
    }
  }

  //
  // If needed, we will create a list of every image we come across.
  //
  if (config->Boolean ("create_image_list"))
  {
    const String filename = config->Find ("image_list");
    images_seen = fopen (filename, initial ? "w" : "a");
    if (images_seen == 0)
    {
      reportError (form ("Unable to create images file '%s'",
                         filename.get ()));
    }
  }

  //
  // Set up the limits list
  //
  StringList l (config->Find ("limit_urls_to"), " \t");
  limits.setEscaped (l, config->Boolean ("case_sensitive"));
  l.Destroy ();

  l.Create (config->Find ("limit_normalized"), " \t");
  limitsn.setEscaped (l, config->Boolean ("case_sensitive"));
  l.Destroy ();

  //
  // Open the document database
  //
  const String filename = config->Find ("doc_db");
  if (initial)
    unlink (filename);

  const String index_filename = config->Find ("doc_index");
  if (initial)
    unlink (index_filename);

  const String head_filename = config->Find ("doc_excerpt");
  if (initial)
    unlink (head_filename);

  if (docs.Open (filename, index_filename, head_filename) < 0)
  {
    reportError (form ("Unable to open/create document database '%s'",
                       filename.get ()));
  }

  const String word_filename = config->Find ("word_db");
  if (initial)
  {
    unlink (word_filename);
    unlink ((word_filename + "_weakcmpr").get ());

    // Remove "duplicate detection" database
    unlink (config->Find ("md5_db"));

    // using  -i,  also ignore seen-but-not-processed URLs from last pass
    unlink (config->Find ("url_log"));
  }

  // Initialize htword
  WordContext::Initialize (*config);

  // Create the Retriever object which we will use to parse all the
  // HTML files.
  // In case this is just an update dig, we will add all existing
  // URLs?
  //
  Retriever retriever (Retriever_logUrl);
  if (minimalFile.length () == 0)
  {
    List *list = docs.URLs ();
    retriever.Initial (*list);
    delete list;

    // Add start_url to the initial list of the retriever.
    // Don't check a URL twice!
    // Beware order is important, if this bugs you could change
    // previous line retriever.Initial(*list, 0) to Initial(*list,1)
    retriever.Initial (config->Find ("start_url"), 1);
  }

  // Handle list of URLs given in a file (stdin, if "-") specified as
  // argument to -m or as an optional trailing argument.
  if (optind < ac)
  {
    if (debug)
      if (minimalFile.length () != 0)
        cout << "Warning: argument " << av[optind]
          << " overrides -m " << minimalFile << endl;
    minimalFile = av[optind];
  }
  if (strcmp (minimalFile.get (), "-") == 0)
  {
    String str;
    // Why not combine this with the code below, with  input = stdin ?
    while (!cin.eof ())
    {
      cin >> str;
      str.chop ("\r\n");        // (Why "\r\n" here and "\r\n\t " below?)
      if (str.length () > 0)
        retriever.Initial (str, 1);
    }
  }
  else if (minimalFile.length () != 0)
  {
    FILE *input = fopen (minimalFile.get (), "r");
    char buffer[1000];

    if (input)
    {
      while (fgets (buffer, sizeof (buffer), input))
      {
        String str (buffer);
        str.chop ("\r\n\t ");
        if (str.length () > 0)
          retriever.Initial (str, 1);
      }
      fclose (input);
    }
    else
    {
      cerr << "Could not open argument '" << minimalFile << "' of flag -m\n";
      exit (1);
    }
  }

  //
  // Go do it!
  //
  retriever.Start ();

  //
  // All done with parsing.
  //

  //
  // If the user so wants, create a text version of the document database.
  //

  if (create_text_database)
  {
    const String doc_list = config->Find ("doc_list");
    if (initial)
      unlink (doc_list);
    docs.DumpDB (doc_list);
    const String word_dump = config->Find ("word_dump");
    if (initial)
      unlink (word_dump);
    HtWordList words (*config);
    if (words.Open (config->Find ("word_db"), O_RDONLY) == OK)
    {
      words.Dump (word_dump);
    }
  }

  //
  // Cleanup
  //
  if (urls_seen)
    fclose (urls_seen);
  if (images_seen)
    fclose (images_seen);

  //
  // If needed, report some statistics
  //
  if (report_statistics)
  {
    retriever.ReportStatistics ("hldig");
  }

  // Shows End Time
  if (debug > 0)
  {
    EndTime.SettoNow ();
    cout << _("hldig End Time: ") << EndTime.GetAscTime () << endl;
  }

  if (_cookie_jar)
    delete _cookie_jar;
}


//
// Display usage information for the hldig program
//
void
usage ()
{
  printf (_("usage: hldig [-v][-i][-c configfile][-t][-m minimalfile]\n\
This program is part of hldig %s\n\n\
Options:\n"), VERSION);

  printf (_("\t-v\tVerbose mode.  This increases the verbosity of the\n\
\t\tprogram.  Using more than 2 is probably only useful\n\
\t\tfor debugging purposes.  The default verbose mode\n\
\t\tgives a nice progress report while digging.\n\n"));

  printf (_("\t-i\tInitial.  Do not use any old databases.  This is\n\
\t\taccomplished by first erasing the databases.\n\n"));

  printf (_("\t-c configfile\n\
\t\tUse the specified configuration file instead of the\n\
\t\tdefault.\n\n"));

  printf (_("\t-t\tCreate an ASCII version of the document database.\n\
\t\tThis database is easy to parse with other programs so\n\
\t\tthat information can be extracted from it.\n\n"));

  printf (_("\t-h hop count\n\
\t\tLimit the stored documents to those which are at\n\
\t\tmost hopcount links away from the start URL.\n\n"));

  printf (_("\t-s\tReport statistics after completion.\n\n"));

  printf (_("\t-u username:password\n\
\t\tTells hldig to send the supplied username and\n\
\t\tpassword with each HTTP request.  The credentials\n\
\t\twill be encoded using the 'Basic' authentication scheme.\n\
\t\tThere *HAS* to be a colon (:) between the username\n\
\t\tand password.\n\n"));

  printf (_("\t-a\tUse alternate work files.\n\
\t\tTells hldig to append .work to database files, causing\n\
\t\ta second copy of the database to be built.  This allows\n\
\t\tthe original files to be used by htsearch during the\n\
\t\tindexing run.\n\n"));

  printf (_("\t-m minimalfile  (or just a file name at end of arguments)\n\
\t\tTells hldig to read URLs from the supplied file and index\n\
\t\tthem in place of (or in addition to) the existing URLs in\n\
\t\tthe database and the start_url.  With the -m, only the\n\
\t\tURLs specified are added to the database.  A file name of\n\
\t\t'-' indicates the standard input.\n\n"));



  exit (0);
}

//
// Report an error and die
//
void
reportError (String msg)
{
  cout << "hldig: " << msg << "\n\n";
  exit (1);
}
