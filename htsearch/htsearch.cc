//
// htsearch.cc
//
// Implementation of htsearch
//
// $Log: htsearch.cc,v $
// Revision 1.1.1.1  1997/02/03 17:11:05  turtle
// Initial CVS
//
// Revision 1.1  1996/01/03 18:59:56  turtle
// Before rewrite
//
//
#if RELEASE
static char RCSid[] = "$Id: htsearch.cc,v 1.1.1.1 1997/02/03 17:11:05 turtle Exp $";
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


typedef void (*SIGNAL_HANDLER) (...);

ResultList *htsearch(char *, List &, Parser *);
void setupWords(char *, List &, String&, int, Parser *);
void createLogicalWords(List &, String &, StringMatch &);
void reportError(char *);
void convertToBoolean(List &words);
void doFuzzy(WeightWord *, List &, List &);
void addRequiredWords(List &, StringList &);

int			debug = 0;


//*****************************************************************************
// int main()
//
int
main(int ac, char **av)
{
    List		searchWords;
    String		configFile = DEFAULT_CONFIG_FILE;
    int			pageNumber = 1;
    StringMatch		limit_to;
    StringMatch		exclude_these;
    String		originalWords;
    String		logicalWords;
    StringMatch		searchWordsPattern;
    StringList		requiredWords;

    if (ac > 1 && strcmp(av[1], "-d") == 0)
	debug = 1;
    
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
	limit_to.Pattern(input["restrict"]);
    }
    if (input.exists("exclude"))
    {
	exclude_these.Pattern(input["exclude"]);
    }

    //
    // Setup the configuration database.  First we read the compiled defaults.
    // Then we override those with defaults read in from the configuration
    // file, and finally we override some attributes with information we
    // got from the HTML form.
    //
    config.Defaults(&defaults[0]);
    if (input.exists("config") && !strchr(input["config"], '.'))
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
    display.setResults(results);
    display.setSearchWords(&searchWords);
    display.setLimit(&limit_to);
    display.setExclude(&exclude_these);
    display.setAllWordsPattern(&searchWordsPattern);
    display.setCGI(&input);
    display.setLogicalWords(logicalWords);
    display.setOriginalWords(originalWords);
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
    WordList	wordlist;		// Just used to check for valid words.
    wordlist.BadWordFile(config["bad_word_list"]);

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
	parsedWords << p << ' ';
    }

    parsedWords.chop(' ');
	
    //
    // Convert the string to a list of WeightWord objects.  The special
    // characters '(' and ')' will be put into their own WeightWord objects.
    //
    unsigned char	*pos = (unsigned char*) allWords;
    int			t;
    String		word;
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
		     (t >= 161 && t <= 255))
	    {
		word = 0;
		while (t && (isalnum(t) || strchr(valid_punctuation, t) ||
			     t == ':' || (t >= 161 && t <= 255)))
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
		else
		{
		    WeightWord	*ww = new WeightWord(word, 1.0);
		    if (!wordlist.IsValid(word))
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
	if (ww->weight > 0)
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
