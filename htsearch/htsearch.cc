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
static char RCSid[] = "$Id: htsearch.cc,v 1.24.2.17 2001/11/01 20:45:07 grdetil Exp $";
#endif

#include "htsearch.h"
#include "WeightWord.h"
#include "parser.h"
#include "Display.h"
#include "../htfuzzy/Fuzzy.h"
#include "cgi.h"
#include "WordRecord.h"
#include "WordList.h"
#include "StringList.h"
#include "IntObject.h"
#include <time.h>
#include <ctype.h>
#include <signal.h>
#include "HtURLCodec.h"
#include "HtURLRewriter.h"
#include "HtWordType.h"

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
StringList		boolean_keywords;


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
    StringMatch		limit_to;
    StringMatch		exclude_these;
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
	      // The default is obviously to do this securely
	      // but if people want to shoot themselves in the foot...
#ifndef ALLOW_INSECURE_CGI_CONFIG
	      if (!getenv("REQUEST_METHOD"))
		{
#endif
		  configFile = optarg;
		  override_config=1;
#ifndef ALLOW_INSECURE_CGI_CONFIG
		}
#endif
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
    int		filenameok = (debug && getenv("REQUEST_METHOD") == 0);
    String	filenamemsg;

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
    if (access(configFile, R_OK) < 0)
    {
	if (filenameok) filenamemsg << " '" << configFile.get() << "'";
	reportError(form("Unable to read configuration file%s",
			 filenamemsg.get()));
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
	config.Add("keywords", input["keywords"]);
    requiredWords.Create(config["keywords"], " \t\r\n\001");
    if (input.exists("sort"))
	config.Add("sort", input["sort"]);

    // Changes added 3-31-99, by Mike Grommet
    // Check form entries for starting date, and ending date
    // Each date consists of a month, day, and year

    if (input.exists("startmonth"))
	config.Add("startmonth", input["startmonth"]);
    if (input.exists("startday"))
	config.Add("startday", input["startday"]);
    if (input.exists("startyear"))
	config.Add("startyear", input["startyear"]);

    if (input.exists("endmonth"))
	config.Add("endmonth", input["endmonth"]);
    if (input.exists("endday"))
	config.Add("endday", input["endday"]);
    if (input.exists("endyear"))
	config.Add("endyear", input["endyear"]);

    // END OF CHANGES BY MIKE GROMMET    


    minimum_word_length = config.Value("minimum_word_length", minimum_word_length);

    StringList form_vars(config["allow_in_form"], " \t\r\n");
    for (i= 0; i < form_vars.Count(); i++)
    {
      if (input.exists(form_vars[i]))
	config.Add(form_vars[i], input[form_vars[i]]);
    }
 
    //
    // Compile the URL limit pattern.
    //

    StringList urllist;
    String urlpat;

    if (strlen(config["restrict"]))
    {
        // Create a temporary list from either the configuration
        // file or the input parameter
        urllist.Create(config["restrict"], "| \t\r\n\001");
        urlpat = urllist.Join('|');
        urllist.Release();  // release the temporary list of URLs
        config.Add("restrict", urlpat);  // re-create the config attribute
	limit_to.Pattern(urlpat);  // Set the new limit pattern
    }

    if (strlen(config["exclude"]))
    {
        // Create a temporary list from either the configuration
        // file or the input parameter
        urllist.Create(config["exclude"], "| \t\r\n\001");
        urlpat = urllist.Join('|');
        urllist.Release();  // release the temporary list of URLs
        config.Add("exclude", urlpat);  // re-create the config attribute
        exclude_these.Pattern(urlpat);
    }

    // Ctype-like functions for what constitutes a word.
    HtWordType::Initialize(config);

    //
    // Check url_part_aliases and common_url_parts for
    // errors.
    String url_part_errors = HtURLCodec::instance()->ErrMsg();

    if (url_part_errors.length() != 0)
      reportError(form("Invalid url_part_aliases or common_url_parts: %s",
                       url_part_errors.get()));

    // for htsearch, we use search_rewrite_rules attribute for HtURLRewriter.
    config.AddParsed("url_rewrite_rules", "${search_rewrite_rules}");
    url_part_errors = HtURLRewriter::instance()->ErrMsg();
    if (url_part_errors.length() != 0)
      reportError(form("Invalid url_rewrite_rules: %s",
                       url_part_errors.get()));

    // Load boolean_keywords from configuration
    // they should be placed in this order:
    //    0       1       2
    //    and     or      not
    boolean_keywords.Create(config["boolean_keywords"], "| \t\r\n\001");
    if (boolean_keywords.Count() != 3)
	reportError("boolean_keywords attribute is not correctly specified");

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
    String	word_db = config["word_db"];
    if (access(word_db, R_OK) < 0)
    {
	if (filenameok) filenamemsg << " '" << word_db.get() << "'";
	reportError(form("Unable to read word database file%s\nDid you run htmerge?",
			 filenamemsg.get()));
    }
    ResultList	*results = htsearch(word_db, searchWords, parser);

    String	index = config["doc_index"];
    if (access(index, R_OK) < 0)
    {
	if (filenameok) filenamemsg << " '" << index.get() << "'";
	reportError(form("Unable to read document index file%s\nDid you run htmerge?",
			 filenamemsg.get()));
    }
    String	doc_db = config["doc_db"];
    if (access(doc_db, R_OK) < 0)
    {
	if (filenameok) filenamemsg << " '" << doc_db.get() << "'";
	reportError(form("Unable to read document database file%s\nDid you run htmerge?",
			 filenamemsg.get()));
    }

    Display	display(index, doc_db);
    if (display.hasTemplateError())
      {
	if (filenameok) filenamemsg << " '" << config["template_name"] << "'";
	reportError(form("Unable to read template file%s\nDoes it exist?",
			 filenamemsg.get()));
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

    for (i = 0; i < searchWords.Count(); i++)
    {
	WeightWord	*ww = (WeightWord *) searchWords[i];
	if (!ww->isHidden)
	{
	    if (strcmp(ww->word, "&") == 0 && wasHidden == 0)
		logicalWords << " " << boolean_keywords[0] << " ";
	    else if (strcmp(ww->word, "|") == 0 && wasHidden == 0)
		logicalWords << " " << boolean_keywords[1] << " ";
	    else if (strcmp(ww->word, "!") == 0 && wasHidden == 0)
		logicalWords << " " << boolean_keywords[2] << " ";
	    else if (wasHidden == 0)
	    {
		logicalWords << ww->word;
	    }
	    wasHidden = 0;
	}
	else
	    wasHidden = 1;
	if (ww->weight > 0			// Ignore boolean syntax stuff
	    && !ww->isIgnore)			// Ignore short or bad words
	{
	    if (pattern.length())
		pattern << '|';
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
    WordList	badWords;		// Just used to check for valid words.
    badWords.BadWordFile(config["bad_word_list"]);

    //
    // Convert the string to a list of WeightWord objects.  The special
    // characters '(' and ')' will be put into their own WeightWord objects.
    //
    unsigned char	*pos = (unsigned char*) allWords;
    unsigned char	t;
    String		word;
    // Why use a char type if String is the new char type!!!
    char		*prefix_suffix = config["prefix_match_character"];
    while (*pos)
    {
	while (1)
	{
	    t = *pos++;
	    if (isspace(t))
	    {
		continue;
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
	  	word.lowercase();
		if (boolean && mystrcasecmp(word.get(), boolean_keywords[0]) == 0)
		{
		    tempWords.Add(new WeightWord("&", -1.0));
		}
		else if (boolean && mystrcasecmp(word.get(), boolean_keywords[1]) == 0)
		{
		    tempWords.Add(new WeightWord("|", -1.0));
		}
		else if (boolean && mystrcasecmp(word.get(), boolean_keywords[2]) == 0)
		{
		    tempWords.Add(new WeightWord("!", -1.0));
		}
		else
		{
		    // Add word to excerpt matching list
		    originalPattern << word << "|";
	  	    HtStripPunctuation(word);
		    WeightWord	*ww = new WeightWord(word, 1.0);
		    if (!badWords.IsValid(word) ||
			word.length() < minimum_word_length)
		    {
			ww->isIgnore = 1;
			tempWords.Add(ww);
		    }
		    else
		    {
			tempWords.Add(ww);
		    }
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
    StringList	algs(config["search_algorithm"], " \t");
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
	fweight = atof(weight);

	fuzzy = Fuzzy::getFuzzyByName(name);
	if (fuzzy)
	{
	    fuzzy->setWeight(fweight);
	    fuzzy->openIndex(config);
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
    // Does the next thing work??
//    algorithms.Start_Get();
//    while ((fuzzy = (Fuzzy *) algorithms.Get_Next()))
//	delete fuzzy;
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

    if (words.Count() == 0)
	return;
    list.Add(words[0]);
    for (i = 1; i < words.Count(); i++)
    {
	if (do_and)
	    list.Add(new WeightWord("&", -1.0));
	else
	    list.Add(new WeightWord("|", -1.0));
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
//   List of WordReference objects.
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
	Database	*dbf = Database::getDatabaseInstance();

	dbf->OpenRead(wordfile);

	parser->setDatabase(dbf);
	parser->parse(&searchWords, *matches);
	dbf->Close();
	delete dbf;
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
    static int	any_keywords = config.Boolean("any_keywords", 0);
    if (requiredWords.Count() == 0)
	return;
    if (searchWords.Count() > 0)
    {
	searchWords.Insert(new WeightWord("(", -1.0), 0);
	searchWords.Add(new WeightWord(")", -1.0));
	searchWords.Add(new WeightWord("&", -1.0));
    }
    if (requiredWords.Count() == 1)
    {
	searchWords.Add(new WeightWord(requiredWords[0], 1.0));
    }
    else
    {
	searchWords.Add(new WeightWord("(", -1.0));
	searchWords.Add(new WeightWord(requiredWords[0], 1.0));
	for (int i = 1; i < requiredWords.Count(); i++)
	{
	    if (any_keywords)
		searchWords.Add(new WeightWord("|", -1.0));
	    else
		searchWords.Add(new WeightWord("&", -1.0));
	    searchWords.Add(new WeightWord(requiredWords[i], 1.0));
	}
	searchWords.Add(new WeightWord(")", -1.0));
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
