//
// htsearch.cc
//
// htsearch: The main search CGI. Parses the CGI input, reads the config files
//           and calls the necessary code to put together the result lists
//           and the final display.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: htsearch.cc,v 1.53 1999/10/08 12:05:21 loic Exp $
//

#include "htsearch.h"
#include "WeightWord.h"
#include "parser.h"
#include "Display.h"
#include "../htfuzzy/Fuzzy.h"
#include "cgi.h"
#include "WordRecord.h"
#include "HtWordList.h"
#include "StringList.h"
#include "IntObject.h"
#include "HtURLCodec.h"
#include "WordType.h"
#include "HtRegex.h"

#include <time.h>
#include <ctype.h>
#include <signal.h>


// If we have this, we probably want it.
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

typedef void (*SIGNAL_HANDLER) (...);

ResultList *htsearch(char *, List &, Parser *);

void setupWords(char *, List &, int, Parser *, String &);
void createLogicalWords(List &, String &, String &);
void reportError(char *);
void convertToBoolean(List &words);
void doFuzzy(WeightWord *, List &, List &);
void addRequiredWords(List &, StringList &);
void usage();

int			debug = 0;
int			minimum_word_length = 3;


//*****************************************************************************
// int main()
//
int
main(int ac, char **av)
{
    int			c;
    extern char		*optarg;
    int		        override_config=0;
    List		searchWords;
    String		configFile = DEFAULT_CONFIG_FILE;
    int			pageNumber = 1;
    HtRegex		limit_to;
    HtRegex		exclude_these;
    String		logicalWords;
    String              origPattern;
    String              logicalPattern;
    StringMatch		searchWordsPattern;
    StringList		requiredWords;
    int                 i;

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

    //
    // Parse the CGI parameters.
    //
    char	none[] = "";
    cgi		input(optind < ac ? av[optind] : none);

    //
    // Compile the URL limit pattern.
    //
    if (input.exists("restrict"))
    {
	StringList l(input["restrict"], " \t\001|");
	limit_to.setEscaped(l);
	l.Release();
    }
    if (input.exists("exclude"))
    {
	StringList l(input["exclude"], " \t\001|");
	exclude_these.setEscaped(l);
	l.Release();
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
	char	*configDir = getenv("CONFIG_DIR");
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
    if (access((char*)configFile, R_OK) < 0)
    {
	reportError(form("Unable to read configuration file '%s'",
			 configFile.get()));
    }
    config.Read(configFile);

    if (input.exists("method"))
	config.Add("match_method", input["method"]);
    if (input.exists("format"))
	config.Add("template_name", input["format"]);

    if (input.exists("matchesperpage"))
    {
	// minimum check for a valid int value of "matchesperpage" cgi variable
	if (atoi(input["matchesperpage"]) > 0)
	    config.Add("matches_per_page", input["matchesperpage"]);
    }

    if (input.exists("page"))
	pageNumber = atoi(input["page"]);
    if (input.exists("config"))
	config.Add("config", input["config"]);
    if (input.exists("restrict"))
	config.Add("restrict", input["restrict"]);
    if (input.exists("exclude"))
	config.Add("exclude", input["exclude"]);
    if (input.exists("keywords"))
	requiredWords.Create(input["keywords"], " \t\r\n\001");
    if (input.exists("sort"))
	config.Add("sort", input["sort"]);

    minimum_word_length = config.Value("minimum_word_length", minimum_word_length);

    StringList form_vars(config["allow_in_form"], " \t\r\n");
    for (i= 0; i < form_vars.Count(); i++)
    {
      if (input.exists(form_vars[i]))
	config.Add(form_vars[i], input[form_vars[i]]);
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

    Parser	*parser = new Parser();
	
    //
    // Parse the words to search for from the argument list.
    // This will produce a list of WeightWord objects.
    //
    String	 originalWords = input["words"];
    originalWords.chop(" \t\r\n");
    setupWords(originalWords, searchWords,
	       strcmp(config["match_method"], "boolean") == 0,
	       parser, origPattern);

    //
    // Convert the list of WeightWord objects to a pattern string
    // that we can compile.
    //
    createLogicalWords(searchWords, logicalWords, logicalPattern);

    // 
    // Assemble the full pattern for excerpt matching and highlighting
    //
    origPattern += logicalPattern;
    searchWordsPattern.IgnoreCase();
    searchWordsPattern.IgnorePunct();
    searchWordsPattern.Pattern(logicalPattern);	// this should now be enough
    //searchWordsPattern.Pattern(origPattern);
    //if (debug > 2)
    //  cout << "Excerpt pattern: " << origPattern << "\n";

    //
    // If required keywords were given in the search form, we will
    // modify the current searchWords list to include the required
    // words.
    //
    if (requiredWords.Count() > 0)
    {
	addRequiredWords(searchWords, requiredWords);
    }
    
    //
    // Perform the actual search.  The function htsearch() is used for this.
    // The Dictionary it returns is then passed on to the Display object to
    // actually render the results in HTML.
    //
    const String	word_db = config["word_db"];
    if (access(word_db, R_OK) < 0)
    {
	reportError(form("Unable to read word database file '%s'\nDid you run htmerge?",
			 word_db.get()));
    }
    ResultList	*results = htsearch(word_db, searchWords, parser);

    const String	doc_db = config["doc_db"];
    if (access(doc_db, R_OK) < 0)
    {
	reportError(form("Unable to read document database file '%s'\nDid you run htmerge?",
			 doc_db.get()));
    }

    const String	doc_excerpt = config ["doc_excerpt"];
    if (access(doc_excerpt, R_OK) < 0)
    {
	reportError(form("Unable to read document excerpts '%s'\nDid you run htmerge?",
			 doc_excerpt.get()));
    }

    Display	display(doc_db, 0, doc_excerpt);
    if (display.hasTemplateError())
      {
	reportError(form("Unable to read template file '%s'\nDoes it exist?",
                         (const char*)config["template_name"]));
	return 0;
      }
    display.setOriginalWords(originalWords);
    display.setResults(results);
    display.setSearchWords(&searchWords);
    display.setLimit(&limit_to);
    display.setExclude(&exclude_these);
    display.setAllWordsPattern(&searchWordsPattern);
    display.setCGI(&input);
    display.setLogicalWords(logicalWords);
    if (parser->hadError())
	display.displaySyntaxError(parser->getErrorMessage());
    else
	display.display(pageNumber);

    delete results;
    delete parser;
    return 0;
}

//*****************************************************************************
void
createLogicalWords(List &searchWords, String &logicalWords, String &wm)
{
    String		pattern;
    int			i;
    int			wasHidden = 0;
    int			inPhrase = 0;

    for (i = 0; i < searchWords.Count(); i++)
    {
	WeightWord	*ww = (WeightWord *) searchWords[i];
	if (!ww->isHidden)
	{
	    if (strcmp((char*)ww->word, "&") == 0 && wasHidden == 0)
		logicalWords << " and ";
	    else if (strcmp((char*)ww->word, "|") == 0 && wasHidden == 0)
		logicalWords << " or ";
	    else if (strcmp((char*)ww->word, "!") == 0 && wasHidden == 0)
		logicalWords << " not ";
	    else if (strcmp((char*)ww->word, "\"") == 0 && wasHidden == 0)
	      {
		if (inPhrase)
		  logicalWords.chop(' ');
		inPhrase = !inPhrase;
		logicalWords << "\"";
	      }
	    else if (wasHidden == 0)
	    {
	      logicalWords << ww->word;
	      if (inPhrase)
		logicalWords << " ";
	    }
	    wasHidden = 0;
	}
	else
	    wasHidden = 1;
	if (ww->weight > 0			// Ignore boolean syntax stuff
	    && !ww->isIgnore)			// Ignore short or bad words
	{
	    if (pattern.length() && !inPhrase)
		pattern << '|';
	    else if (pattern.length() && inPhrase)
	      pattern << ' ';
	    pattern << ww->word;
	}
    }
    wm = pattern;

    if (debug)
    {
	cerr << "LogicalWords: " << logicalWords << endl;
	cerr << "Pattern: " << pattern << endl;
    }
}

void
dumpWords(List &words, char *msg = "")
{
    if (debug)
    {
	cerr << msg << ": '";
	for (int i = 0; i < words.Count(); i++)
	{
	    WeightWord	*ww = (WeightWord *) words[i];
	    cerr << ww->word << ':' << ww->isHidden << ' ';
	}
	cerr << "'\n";
    }
}

//*****************************************************************************
// void setupWords(char *allWords, List &searchWords,
//		   int boolean, Parser *parser, String &originalPattern)
//
void
setupWords(char *allWords, List &searchWords, int boolean, Parser *parser,
	   String &originalPattern)
{
    List	tempWords;
    int		i;

    //
    // Parse the words we need to search for.  It should be a list of words
    // with optional 'and' and 'or' between them.  The list of words
    // will be put in the searchWords list and at the same time in the
    // String pattern separated with '|'.
    //

    //
    // Convert the string to a list of WeightWord objects.  The special
    // characters '(' and ')' will be put into their own WeightWord objects.
    //
    unsigned char	*pos = (unsigned char*) allWords;
    unsigned char	t;
    String		word;
    // Why use a char type if String is the new char type!!!
    const String	prefix_suffix = config["prefix_match_character"];
    while (*pos)
    {
	while (1)
	{
	    t = *pos++;
	    if (isspace(t))
	    {
		continue;
	    }
	    else if (t == '"')
	      {
		tempWords.Add(new WeightWord("\"", -1.0));
		break;
	      }
	    else if (boolean && (t == '(' || t == ')'))
	    {
		char	s[2];
		s[0] = t;
		s[1] = '\0';
		tempWords.Add(new WeightWord(s, -1.0));
		break;
	    }
	    else if (HtIsWordChar(t) || t == ':' ||
			 (strchr(prefix_suffix, t) != NULL) || (t >= 161 && t <= 255))
	    {
		word = 0;
		while (t && (HtIsWordChar(t) ||
			     t == ':' || (strchr(prefix_suffix, t) != NULL) || (t >= 161 && t <= 255)))
		{
		    word << (char) t;
		    t = *pos++;
		}

		pos--;
		if (boolean && mystrcasecmp(word.get(), "and") == 0)
		{
		    tempWords.Add(new WeightWord("&", -1.0));
		}
		else if (boolean && mystrcasecmp(word.get(), "or") == 0)
		{
		    tempWords.Add(new WeightWord("|", -1.0));
		}
		else if (boolean && mystrcasecmp(word.get(), "not") == 0)
		{
		    tempWords.Add(new WeightWord("!", -1.0));
		}
		else if (boolean && mystrcasecmp(word.get(), "+") == 0)
		  tempWords.Add(new WeightWord("&", -1.0));
		else if (boolean && mystrcasecmp(word.get(), "-") == 0)
		  tempWords.Add(new WeightWord("!", -1.0));
		else
		{
		    // Add word to excerpt matching list
		    originalPattern << word << "|";
		    WeightWord	*ww = new WeightWord(word, 1.0);
		    if(HtWordNormalize(word) & WORD_NORMALIZE_NOTOK)
			ww->isIgnore = 1;
		    tempWords.Add(ww);
		}
		break;
	    }
	}
    }

    dumpWords(tempWords, "tempWords");
	
    //
    // If the user specified boolean expression operators, the whole
    // expression has to be syntactically correct.  If not, we need
    // to report a syntax error.
    //
    if (boolean)
    {
	if (!parser->checkSyntax(&tempWords))
	{
	    for (i = 0; i < tempWords.Count(); i++)
	    {
		searchWords.Add(tempWords[i]);
	    }
	    tempWords.Release();
	    return;
//			reportError("Syntax error");
	}
    }
    else
    {
	convertToBoolean(tempWords);
    }
	
    dumpWords(tempWords, "Boolean");
	
    //
    // We need to assign weights to the words according to the search_algorithm
    // configuration attribute.
    // For algorithms other than exact, we need to also do word lookups.
    //
    StringList	algs(config["search_algorithm"], " \t,");
    List		algorithms;
    String		name, weight;
    double		fweight;
    Fuzzy		*fuzzy = 0;

    //
    // Generate the list of algorithms to use and associate the given
    // weights with them.
    //
    for (i = 0; i < algs.Count(); i++)
    {
	name = strtok(algs[i], ":");
	weight = strtok(0, ":");
	if (name.length() == 0)
	    name = "exact";
	if (weight.length() == 0)
	    weight = "1";
	fweight = atof((char*)weight);

	fuzzy = Fuzzy::getFuzzyByName(name, config);
	if (fuzzy)
	{
	    fuzzy->setWeight(fweight);
	    fuzzy->openIndex();
	    algorithms.Add(fuzzy);
	}
    }

    dumpWords(searchWords, "initial");
	
    //
    // For each of the words, apply all the algorithms.
    //
    for (i = 0; i < tempWords.Count(); i++)
    {
	WeightWord	*ww = (WeightWord *) tempWords[i];
	if (ww->weight > 0 && !ww->isIgnore)
	{
	    //
	    // Apply all the algorithms to the word.
	    //
	    if (debug)
	      cerr << "Fuzzy on: " << ww->word << endl;
	    doFuzzy(ww, searchWords, algorithms);
	    delete ww;
	}
	else
	{
	    //
	    // This is '(', ')', '&', or '|'.  These will be automatically
	    // transfered to the searchWords list.
	    //
	    if (debug)
		cerr << "Add: " << ww->word << endl;
	    searchWords.Add(ww);
	}
	dumpWords(searchWords, "searchWords");
    }
    tempWords.Release();
}


//*****************************************************************************
void
doFuzzy(WeightWord *ww, List &searchWords, List &algorithms)
{
    List		fuzzyWords;
    List		weightWords;
    Fuzzy		*fuzzy;
    WeightWord	*newWw;
    String		*word;

    algorithms.Start_Get();
    while ((fuzzy = (Fuzzy *) algorithms.Get_Next()))
    {
        if (debug > 1)
	  cout << "   " << fuzzy->getName();
	fuzzy->getWords(ww->word, fuzzyWords);
	fuzzyWords.Start_Get();
	while ((word = (String *) fuzzyWords.Get_Next()))
	{
	    if (debug > 1)
	      cout << " " << word->get();
	    newWw = new WeightWord(word->get(), fuzzy->getWeight());
	    newWw->isExact = ww->isExact;
	    newWw->isHidden = ww->isHidden;
	    weightWords.Add(newWw);
	}
	if (debug > 1)
	  cout << endl;
	fuzzyWords.Destroy();
    }

    //
    // We now have a list of substitute words.  They need to be added
    // to the searchWords.
    //
    if (weightWords.Count())
    {
	if (weightWords.Count() > 1)
	    searchWords.Add(new WeightWord("(", -1.0));
	for (int i = 0; i < weightWords.Count(); i++)
	{
	    if (i > 0)
		searchWords.Add(new WeightWord("|", -1.0));
	    searchWords.Add(weightWords[i]);
	}
	if (weightWords.Count() > 1)
	    searchWords.Add(new WeightWord(")", -1.0));
    }
    weightWords.Release();
}


//*****************************************************************************
// void convertToBoolean(List &words)
//
void
convertToBoolean(List &words)
{
    List	list;
    int		i;
    int		do_and = strcmp(config["match_method"], "and") == 0;
    int		in_phrase = 0;

    String	quote = "\"";

    if (words.Count() == 0)
	return;
    list.Add(words[0]);

    // We might start off with a phrase match
    if (((WeightWord *) words[0])->word == quote)
	in_phrase = 1;

    for (i = 1; i < words.Count(); i++)
    {
	if (do_and && !in_phrase)
	    list.Add(new WeightWord("&", -1.0));
	else if (!in_phrase)
	    list.Add(new WeightWord("|", -1.0));
	
	if (((WeightWord *) words[i])->word == quote)
	    in_phrase = !in_phrase;
  
	list.Add(words[i]);
    }
    words.Release();

    for (i = 0; i < list.Count(); i++)
    {
	words.Add(list[i]);
    }
    list.Release();
}


//*****************************************************************************
// Dictionary *htsearch(char *wordfile, List &searchWords, Parser *parser)
//   This returns a dictionary indexed by document ID and containing a
//   List of HtWordReference objects.
//
ResultList *
htsearch(char *wordfile, List &searchWords, Parser *parser)
{
    //
    // Pick the database type we are going to use
    //
    ResultList	*matches = new ResultList;
    if (searchWords.Count() > 0)
    {
	parser->setDatabase(wordfile);
	parser->parse(&searchWords, *matches);
    }
	
    return matches;
}


//*****************************************************************************
// Modify the search words list to include the required words as well.
// This is done by putting the existing search words in parenthesis and
// appending the required words separated with "and".
void
addRequiredWords(List &searchWords, StringList &requiredWords)
{
    searchWords.Insert(new WeightWord("(", -1.0), 0);
    searchWords.Add(new WeightWord(")", -1.0));

    for (int i = 0; i < requiredWords.Count(); i++)
    {
	searchWords.Add(new WeightWord("&", -1.0));
	searchWords.Add(new WeightWord(requiredWords[i], 1.0));
    }
}


//*****************************************************************************
// Report an error.  Since we don' know if we are running as a CGI or not,
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
    cout << "webmaster of this site by sending an e-mail to:\n";
    cout << "<a href=\"mailto:" << config["maintainer"] << "\">";
    cout << config["maintainer"] << "</a>\n";
    cout << "The error message is:</p>\n";
    cout << "<pre>\n" << msg << "\n</pre>\n</body></html>\n";
    exit(1);
}

//*****************************************************************************
// void usage()
//   Display program usage information--assumes we're running from a cmd line
//
void usage()
{
  cout << "usage: htsearch [-v][-d][-c configfile] [query_string]\n";
  cout << "This program is part of ht://Dig " << VERSION << "\n\n";
  cout << "Options:\n";
  cout << "\t-v -d\tVerbose mode.  This increases the verbosity of the\n";
  cout << "\t\tprogram.  Using more than 2 is probably only useful\n";
  cout << "\t\tfor debugging purposes.  The default verbose mode\n";
  cout << "\t\tgives a progress on what it is doing and where it is.\n\n";
  cout << "\t-c configfile\n";
  cout << "\t\tUse the specified configuration file instead on the\n";
  cout << "\t\tdefault.\n\n";
  cout << "\tquery_string\tA CGI-style query string can be given as a single\n";
  cout << "\t\targument, and is only used if the REQUEST_METHOD environment\n";
  cout << "\t\tvariable is not set.  If no query_string is given, and\n";
  cout << "\t\tREQUEST_METHOD is not set, htsearch will prompt for the query.\n\n";
  exit(0);
}
