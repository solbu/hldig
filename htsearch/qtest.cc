//
// qtest.cc
//
// qtest: A program to test the Query classes as replacement for the current
//            parsing code
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: qtest.cc,v 1.1.2.2 2000/09/27 05:17:29 ghutchis Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "cgi.h"
#include "defaults.h"
#include "WordContext.h"
#include <iostream>
#include "QueryParser.h"
#include "Query.h"
#include "ResultList.h"
#include "Exact.h"
#include "Accents.h"
#include "Prefix.h"
#include "WordSearcher.h"
#include "OrFuzzyExpander.h"
#include "ExactWordQuery.h"
#include "OrQueryParser.h"
#include "AndQueryParser.h"
#include "BooleanQueryParser.h"
#include "GParser.h"

// If we have this, we probably want it.
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

void reportError(char *msg);
void usage();

int			debug = 0;

void
ParseAndGet(QueryParser &parser, const String &string);

//*****************************************************************************
// int main()
//
int
main(int ac, char **av)
{
    int			c;
    extern char		*optarg;
    int		        override_config=0;
    String		configFile = DEFAULT_CONFIG_FILE;
    String		logicalWords;
    bool		doall = true,
			doand = false,
			door = false,
			dobool = false,
			dogeoffs = false;

     //
     // Parse command line arguments
     //
     while ((c = getopt(ac, av, "c:dvkaobg")) != -1)
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
	    case 'a':
		doall = false;
		doand = true;
		break;
	    case 'o':
		doall = false;
		door = true;
		break;
	    case 'b':
		doall = false;
		dobool = true;
		break;
	    case 'g':
		doall = false;
		dogeoffs = true;
		break;
	    case '?':
	        usage();
                break;
 	}
     }

    //
    // Parse the CGI parameters.
    //
    char	none[] = "";
    cgi		input(optind < ac ? av[optind] : none);

    String	 originalWords = input["words"];
    originalWords.chop(" \t\r\n");

     // Set up the config
    config.Defaults(&defaults[0]);

    if (access((char*)configFile, R_OK) < 0)
    {
	reportError(form("Unable to find configuration file '%s'",
			 configFile.get()));
    }
	
    config.Read(configFile);

    // Initialize htword library (key description + wordtype...)
    // WordContext::Initialize(config);    

	OrFuzzyExpander exp;
	Exact exact(config);
	exact.setWeight(1.0);
	exact.openIndex();
	exp.Add(&exact);
	Accents accents(config);
	accents.setWeight(0.7);
	accents.openIndex();
	exp.Add(&accents);
	Prefix prefix(config);
	prefix.setWeight(0.7);
	prefix.openIndex();
	exp.Add(&prefix);
	QueryParser::SetFuzzyExpander(&exp);

	WordSearcher searcher(config["word_db"]);
	ExactWordQuery::SetSearcher(&searcher);

	// -- put here your prefered cache
	//QueryCache *cache = new XXX;
	//Query::SetCache(cache);

	OrQueryParser o;
	BooleanQueryParser b;
	GParser g;
	AndQueryParser a;

	if(doall || doand)
	{
		cout << "Trying and..." << endl;
		ParseAndGet(a, originalWords);
	}

	if(doall || door)
	{
		cout << "Trying or..." << endl;
		ParseAndGet(o, originalWords);
	}

	if(doall || dobool)
	{
		cout << "Trying boolean..." << endl;
		ParseAndGet(b, originalWords);
	}

	if(doall || dogeoffs)
	{
		cout << "Trying no-precedence-boolean..." << endl;
		ParseAndGet(g, originalWords);
	}
}

void
ParseAndGet(QueryParser &parser, const String &query)
{
	Query *q = parser.Parse(query);
	if(q)
	{
		cout << "Parsed: " << q->GetLogicalWords() << endl;
		ResultList *l = q->GetResults();
		if(l)
		{
			cout << "Evaluated with " << l->Count() << " matches" << endl;
			if(debug) l->Dump();
		}
		else
		{
			cout << "No matches" << endl;;
		}
	}
	else
	{
		cerr << "syntax error: " << flush << parser.Error() << endl;
	}
	delete q;
}


//*****************************************************************************
// void usage()
//   Display program usage information--assumes we're running from a cmd line
//
void usage()
{
  cout << "usage: qtest [-a][-o][-b][-g][-v][-d][-c configfile]\n";
  cout << "This program is part of ht://Dig " << VERSION << "\n\n";
  cout << "Options:\n";
  cout << "\t-v -d\tVerbose mode.  This increases the verbosity of the\n";
  cout << "\t\tprogram.  Using more than 2 is probably only useful\n";
  cout << "\t\tfor debugging purposes.  The default verbose mode\n";
  cout << "\t\tgives a progress on what it is doing and where it is.\n\n";
  cout << "\t-c configfile\n";
  cout << "\t\tUse the specified configuration file instead on the\n";
  cout << "\t\tdefault.\n\n";
  cout << "\t-a\tPerform only and/all parsing\n\n";
  cout << "\t-o\tPerform only or/any parsing\n\n";
  cout << "\t-b\tPerform only boolean parsing\n\n";
  cout << "\t-g\tPerform only no-precedence-boolean parsing\n\n";
  exit(0);
}

//*****************************************************************************
// Report an error and die
//
void reportError(char *msg)
{
    cout << "qtest: " << msg << "\n\n";
    exit(1);
}
