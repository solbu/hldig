//
// htsearch.cc
//
// Command-line and CGI interface to search the databases
// Expects the databases are generated using htdig, htmerge, and htfuzzy
// Outputs HTML-ized results of the search based on the templates specified
//
// $Log: htsearch.cc,v $
// Revision 1.18  1998/12/19 16:55:11  bergolth
// Added allow_in_form option.
//
// Revision 1.17  1998/12/08 02:53:21  ghutchis
// Fix thinko with multiple excludes and restricts. Pointed out by Gilles.
//
// Revision 1.16  1998/12/05 00:53:53  ghutchis
// Added usage message for the command line.
//
// revision 1.15 1998/12/04 04:13:52 ghutchis
// Use configure check to only include getopt.h when it exists.
// 
// revision 1.14 1998/12/02 02:44:44 ghutchis
// Add include <getopt.h> to help compiling under Win32 with CygWinB20.
//
// revision 1.13 1998/11/30 02:28:50 ghutchis
// Fix mistake in last update so the code compiles.
//
// Revision 1.12  1998/11/30 01:50:38  ghutchis
// Improved support for multiple restrict and exclude patterns, based on code
// from Gilles Detillieux and William Rhee <willrhee@umich.edu>.
//
// Revision 1.11  1998/11/22 19:15:35  ghutchis
// Don't remove boolean operators from boolean search strings!
//
// Revision 1.10  1998/11/01 00:00:40  ghutchis
// Replaced system calls with htlib/my* functions.
//
// Revision 1.9  1998/10/26 20:34:33  ghutchis
// Added patch by Esa Ahola to fix bug with not properly ignoring bad_words
//
// Revision 1.8  1998/09/30 17:31:51  ghutchis
// Changes for 3.1.0b2
//
// Revision 1.7  1998/09/10 04:16:26  ghutchis
// More bug fixes.
//
// Revision 1.6  1998/06/21 23:20:12  turtle
// patches by Esa and Jesse to add BerkeleyDB and Prefix searching
//
// Revision 1.5  1997/04/27 14:43:30  turtle
// changes
//
// Revision 1.4  1997/04/21 15:44:39  turtle
// Added code to check the search words against the minimum_word_length attribute
//
// Revision 1.3  1997/02/24 17:52:55  turtle
// Applied patches supplied by "Jan P. Sorensen" <japs@garm.adm.ku.dk> to make
// ht://Dig run on 8-bit text without the global unsigned-char option to gcc.
//
// Revision 1.2  1997/02/11 00:38:48  turtle
// Renamed the very bad wordlist variable to badWords
//
// Revision 1.1.1.1  1997/02/03 17:11:05  turtle
// Initial CVS
//
// Revision 1.1  1996/01/03 18:59:56  turtle
// Before rewrite
//
//
#if RELEASE
static char RCSid[] = "$Id: htsearch.cc,v 1.18 1998/12/19 16:55:11 bergolth Exp $";
#endif

#include "htsearch.h"
#include "WeightWord.h"
#include "parser.h"
#include "Display.h"
#include "../htfuzzy/Fuzzy.h"
#include <cgi.h>
#include <WordRecord.h>
#include <WordList.h>
#include <StringList.h>
#include <IntObject.h>
#include <time.h>
#include <ctype.h>
#include <signal.h>

// If we have this, we probably want it.
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

typedef void (*SIGNAL_HANDLER) (...);

ResultList *htsearch(char *, List &, Parser *);

void setupWords(char *, List &, String&, int, Parser *);
void createLogicalWords(List &, String &, StringMatch &);
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
    StringMatch		limit_to;
    StringMatch		exclude_these;
    String		originalWords;
    String		logicalWords;
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
    cgi		input;

    //
    // Compile the URL limit pattern.
    //
    if (input.exists("restrict"))
      {
	char *sep = input["restrict"];
	while ((sep = strchr(sep, '\001')) != NULL)
	  *sep++ = '|';
	limit_to.Pattern(input["restrict"]);
    }
    if (input.exists("exclude"))
    {
       char *sep = input["exclude"];
       while ((sep = strchr(sep, '\001')) != NULL)
       	 *sep++ = '|';
       exclude_these.Pattern(input["exclude"]);
    }

    //
    // Setup the configuration database.  First we read the compiled defaults.
    // Then we override those with defaults read in from the configuration
    // file, and finally we override some attributes with information we
    // got from the HTML form.
    //
    config.Defaults(&defaults[0]);
    if (!override_config && input.exists("config") && !strchr(input["config"], '.'))
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
	reportError(form("Unable to read configuration file '%s'",
			 configFile.get()));
    }
    config.Read(configFile);

    if (input.exists("method"))
	config.Add("match_method", input["method"]);
    if (input.exists("format"))
	config.Add("template_name", input["format"]);
    if (input.exists("matchesperpage"))
	config.Add("matches_per_page", input["matchesperpage"]);
    if (input.exists("page"))
	pageNumber = atoi(input["page"]);
    if (input.exists("config"))
	config.Add("config", input["config"]);
    if (input.exists("restrict"))
	config.Add("restrict", input["restrict"]);
    if (input.exists("exclude"))
	config.Add("exclude", input["exclude"]);
    if (input.exists("keywords"))
	requiredWords.Create(input["keywords"], " \t\r\n");

    minimum_word_length = config.Value("minimum_word_length", minimum_word_length);

    StringList form_vars(config["allow_in_form"], " \t\r\n");
    for (i= 0; i < form_vars.Count(); i++)
    {
      if (input.exists(form_vars[i]))
	config.Add(form_vars[i], input[form_vars[i]]);
    }
 
    Parser	*parser = new Parser();
	
    //
    // Parse the words to search for from the argument list.
    // This will produce a list of WeightWord objects.
    //
    String	tmp = input["words"];
    tmp.chop(" \t\r\n");
    setupWords(tmp, searchWords, originalWords,
	       strcmp(config["match_method"], "boolean") == 0,
	       parser);

    //
    // Convert the list of WeightWord objects to a pattern string
    // that we can compile.
    //
    createLogicalWords(searchWords, logicalWords, searchWordsPattern);

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
	reportError(form("Unable to read word database file '%s'\nDid you run htmerge?",
			 word_db.get()));
    }
    ResultList	*results = htsearch(word_db, searchWords, parser);

    String	index = config["doc_index"];
    if (access(index, R_OK) < 0)
    {
	reportError(form("Unable to read document index file '%s'\nDid you run htmerge?",
			 index.get()));
    }
    String	doc_db = config["doc_db"];
    if (access(doc_db, R_OK) < 0)
    {
	reportError(form("Unable to read document database file '%s'\nDid you run htmerge?",
			 doc_db.get()));
    }

    Display	display(index, doc_db);
    if (display.hasTemplateError())
      {
	reportError(form("Unable to read template file '%s'\nDoes it exist?",
                         config["template_name"]));
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
    return 0;
}

//*****************************************************************************
void
createLogicalWords(List &searchWords, String &logicalWords, StringMatch &wm)
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
		logicalWords << " and ";
	    else if (strcmp(ww->word, "|") == 0 && wasHidden == 0)
		logicalWords << " or ";
	    else if (strcmp(ww->word, "!") == 0 && wasHidden == 0)
		logicalWords << " not ";
	    else if (wasHidden == 0)
	    {
		logicalWords << ww->word;
	    }
	    wasHidden = 0;
	}
	else
	    wasHidden = 1;
	if (ww->weight > 0)			// Ignore boolean syntax stuff
	{
	    if (pattern.length())
		pattern << '|';
	    pattern << ww->word;
	}
    }
    wm.IgnoreCase();
    wm.Pattern(pattern);

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
// void setupWords(char *allWords, List &searchWords, String &parsedWords,
//					int boolean, Parser *parser)
//
void
setupWords(char *allWords, List &searchWords, String &parsedWords,
	   int boolean, Parser *parser)
{
    List	tempWords;
    int		i;
    char	*valid_punctuation = 0;
	
    //
    // This is going to be used a lot.  We'll cache it.
    //
    valid_punctuation = config["valid_punctuation"];
    if (!valid_punctuation)
	valid_punctuation = "";

    //
    // Parse the words we need to search for.  It should be a list of words
    // with optional 'and' and 'or' between them.  The list of words
    // will be put in the searchWords list and at the same time in the
    // String pattern separated with '|'.
    //
    WordList	badWords;		// Just used to check for valid words.
    badWords.BadWordFile(config["bad_word_list"]);

    //
    // Create a string with the original search words minus any attributes
    // and minus hidden keywords.
    //
    StringList	origList(allWords, ' ');
    for (i = 0; i < origList.Count(); i++)
    {
	char	*p = origList[i];
	if (mystrncasecmp(p, "hidden:", 7) == 0)
	{
	    i++;
	    continue;
	}
	if (mystrncasecmp(p, "exact:", 6) == 0)
	    p += 6;
	if (mystrncasecmp(p, "hidden:", 7) == 0)
	{
	    i++;
	    continue;
	}
	if (badWords.IsValid(p))
	    parsedWords << p << ' ';
	if (boolean && ((mystrncasecmp(p, "or", 2) == 0) || 
			(mystrncasecmp(p, "and", 3) == 0) ||
			(mystrncasecmp(p, "not", 3) == 0)))
	    parsedWords << p << ' ';
    }

    parsedWords.chop(' ');
	
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
	    else if (isalnum(t) || strchr(valid_punctuation, t) || t == ':' ||
			 (strchr(prefix_suffix, t) != NULL) || (t >= 161 && t <= 255))
	    {
		word = 0;
		while (t && (isalnum(t) || strchr(valid_punctuation, t) ||
			     t == ':' || (strchr(prefix_suffix, t) != NULL) || (t >= 161 && t <= 255)))
		{
		    word << (char) t;
		    t = *pos++;
		}
		word.remove(valid_punctuation);
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
		else
		{
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
	fuzzy->getWords(ww->word, fuzzyWords);
	fuzzyWords.Start_Get();
	while ((word = (String *) fuzzyWords.Get_Next()))
	{
	    newWw = new WeightWord(word->get(), fuzzy->getWeight());
	    newWw->isExact = ww->isExact;
	    newWw->isHidden = ww->isHidden;
	    weightWords.Add(newWw);
	}
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
  cout << "usage: htsearch [-v][-d][-w][-c configfile]\n";
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
