	/***************************************************************************
                          htsearch.cc  -  description
                             -------------------
    begin                : Fri Oct 8 1999
    copyright            : (C) 1999 by 
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
//
// htsearch.cc
//
// Command-line and CGI interface to search the databases
// Expects the databases are generated using htdig, htmerge, and htfuzzy
// Outputs HTML-ized results of the search based on the templates specified
//
//
//
#if RELEASE
static char RCSid[] = "$Id: htsearch.cc,v 1.55 1999/10/15 03:33:17 jtillman Exp $";
#endif

#include "new_htsearch.h"
#include "Display.h"
#include "cgi.h"
#include "WordRecord.h"
#include "HtWordList.h"
#include "StringList.h"
#include "IntObject.h"
#include <time.h>
#include <ctype.h>
#include <signal.h>
#include "HtURLCodec.h"
#include "WordType.h"
#include "Searcher.h"
#include "defaults.h"

// If we have this, we probably want it.
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

typedef void (*SIGNAL_HANDLER) (...);

void reportError(char *);
void usage();

int			debug = 0;
int			minimum_word_length = 3;


//*****************************************************************************
// int main()
//
int
main(int ac, char **av)
{
	int c;
	extern char *optarg;
	int override_config=0;
	List searchWords;
	String configFile = DEFAULT_CONFIG_FILE;
	StringMatch searchWordsPattern;
	int pageNumber = 1;
	int i;

	//
	// Parse command line arguments
	//
	while ((c = getopt(ac, av, "c:dv")) != -1)
	{
		switch (c)
		{
			case 'c':
				configFile = optarg;
				override_config=1;
				break;
			case 'v':
				debug++;
			 	break;
			case 'd':
				debug++;
				break;
			case '?':
				usage();
			  break;
		}
	}

	//
	// The total search can NEVER take more than 5 minutes.
	//
	alarm(5 * 60);

	//This is the new Searcher object, which actually
	// parses the query and performs the search
	Searcher searcher;

	//
	// Parse the CGI parameters.
  //
  char none[] = "";
	cgi input(optind < ac ? av[optind] : none);

	//
  // Compile the URL limit pattern.
  //
  if (input.exists("restrict"))
  {
		char *sep = input["restrict"];
		while ((sep = strchr(sep, '\001')) != NULL)
	  	*sep++ = '|';
		searcher.setRestriction(input["restrict"]);
	}
  if (input.exists("exclude"))
  {
  	char *sep = input["exclude"];
    while ((sep = strchr(sep, '\001')) != NULL)
    *sep++ = '|';
    searcher.setExclusion(input["exclude"]);
  }

	//
  // Setup the configuration database.  First we read the compiled defaults.
  // Then we override those with defaults read in from the configuration
  // file, and finally we override some attributes with information we
  // got from the HTML form.
  //
  config.Defaults(&defaults[0]);
  // To allow . in filename while still being 'secure',
  // e.g. htdig-f.q.d.n.conf
  if (!override_config && input.exists("config")
		&& (strstr(input["config"], "./") == NULL))
  {
		char *configDir = getenv("CONFIG_DIR");
		if (configDir)
		{
	    configFile = configDir;
		}
		else
		{
	    configFile = CONFIG_DIR;
		}
		if (strlen(input["config"]) == 0)
	  	configFile = DEFAULT_CONFIG_FILE;
		else
	  	configFile << '/' << input["config"] << ".conf";
  }
  if (access((char *)configFile, R_OK) < 0)
  {
		reportError(form("Unable to read configuration file '%s'",
		(char *) configFile.get()));
  }

	//Load config file
	//htsearch and Searcher now maintain their own copies of the config
  config.Read(configFile);
  searcher.setConfigFile(configFile);

  if (input.exists("method"))
		searcher.setConfigVal("match_method", input["method"]);
  if (input.exists("format"))
		searcher.setConfigVal("template_name", input["format"]);

  if (input.exists("matchesperpage"))
  {
		// minimum check for a valid int value of "matchesperpage" cgi variable
		if (atoi(input["matchesperpage"]) > 0)
			searcher.setConfigVal("matches_per_page", input["matchesperpage"]);
  }

	if (input.exists("page"))
		pageNumber = atoi(input["page"]);
  if (input.exists("config"))
		searcher.setConfigVal("config", input["config"]);
  if (input.exists("restrict"))
		searcher.setConfigVal("restrict", input["restrict"]);
  if (input.exists("exclude"))
		searcher.setConfigVal("exclude", input["exclude"]);
  if (input.exists("keywords"))
		searcher.setRequiredWords(input["keywords"]);
  if (input.exists("sort"))
		searcher.setConfigVal("sort", input["sort"]);

  minimum_word_length = config.Value("minimum_word_length", minimum_word_length);

  StringList form_vars(config["allow_in_form"], " \t\r\n");
  for (i= 0; i < form_vars.Count(); i++)
	{
		if (input.exists(form_vars[i]))
			searcher.setConfigVal(form_vars[i], input[form_vars[i]]);
	}
 
	// Ctype-like functions for what constitutes a word.
	WordType::Initialize(config);

	//
	// Check url_part_aliases and common_url_parts for
	// errors.
  String url_part_errors = HtURLCodec::instance()->ErrMsg();

  if (url_part_errors.length() != 0)
  	reportError(form("Invalid url_part_aliases or common_url_parts: %s",
			url_part_errors.get()));

	//
  // Give the search words to the Searcher
  //
  searcher.setSearchWords(input["words"]);
  searcher.setDebug(debug);

  ResultList	*results = searcher.execute();

	if (searcher.hadError())
		reportError(searcher.getErrorMessage());

	searchWordsPattern.IgnoreCase();
	searchWordsPattern.Pattern(searcher.getOriginalWords());

	
	if (debug)
	{
		cerr << "Total items returned was: ";
		cerr << results->Count();
	}

//  Display	display(doc_db, 0, doc_excerpt);
	Display display;

  String template_name = config ["template_name"];

  if (display.hasTemplateError())
  {
		String templateName = config["template_name"];
			reportError(form("Unable to read template file '%s'\nDoes it exist?",
                         template_name.get()));
		return 0;
  }
  display.setOriginalWords(input["words"]);
  display.setResults(results);
  display.setSearchWords(searcher.getSearchWords());
  display.setLimit(searcher.getRestriction());
  display.setExclude(searcher.getExclusion());
  display.setAllWordsPattern(&searchWordsPattern);
  display.setCGI(&input);
  display.setLogicalWords(searcher.getLogicalWords());
  if (searcher.hadError())
		display.displaySyntaxError(searcher.getErrorMessage());
  else
		display.display(pageNumber);

  delete results;
  return 0;
}


/*

UTILITY FUNCTIONS

*/


//*****************************************************************************
// Report an error.  Since we don't know if we are running as a CGI or not,
// we will assume this is the first thing returned by a CGI program.
//
void
reportError(char *msg)
{
	cout << "Content-type: text/html\r\n\r\n";
	cout << "<html><head><title>htsearch error</title></head>\n";
	cout << "<body bgcolor=\"#ffffff\">\n";
	cout << "<h1>ht://Dig error</h1>\n";
	cout << "<p>htsearch detected an error.  Please report this to the\n";
	cout << "webmaster of this site.  The error message is:</p>\n";
	cout << "<pre>\n" << msg << "\n</pre>\n</body></html>\n";
	exit(1);
}

//*****************************************************************************
// void usage()
//   Display program usage information--assumes we're running from a cmd line
//
void usage()
{
	cout << "usage: htsearch [-v][-d][-c configfile]\n";
	cout << "This program is part of ht://Dig " << VERSION << "\n\n";
  cout << "Options:\n";
  cout << "\t-v -d\tVerbose mode.  This increases the verbosity of the\n";
  cout << "\t\tprogram.  Using more than 2 is probably only useful\n";
  cout << "\t\tfor debugging purposes.  The default verbose mode\n";
  cout << "\t\tgives a progress on what it is doing and where it is.\n\n";
  cout << "\t-c configfile\n";
  cout << "\t\tUse the specified configuration file instead on the\n";
  cout << "\t\tdefault.\n\n";
  exit(0);
}

























