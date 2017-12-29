//
// htsearch.cc
//
// htsearch: The main search CGI. Parses the CGI input, reads the config files
//           and calls the necessary code to put together the result lists
//           and the final display.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: htsearch.cc,v 1.72 2004/05/28 13:15:24 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "hlsearch.h"
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
#include "HtURLRewriter.h"
#include "WordContext.h"
#include "HtRegex.h"
#include "Collection.h"

#include <time.h>
#include <ctype.h>
#include <signal.h>


// If we have this, we probably want it.
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#elif HAVE_GETOPT_LOCAL
#include <getopt_local.h>
#endif

typedef void (*SIGNAL_HANDLER) (...);

// ResultList *htsearch(const String&, List &, Parser *);
void htsearch (Collection *, List &, Parser *);

void setupWords (char *, List &, int, Parser *, String &);
void createLogicalWords (List &, String &, String &);
void reportError (char *);
void convertToBoolean (List & words);
void doFuzzy (WeightWord *, List &, List &);
void addRequiredWords (List &, StringList &);
void usage ();

int debug = 0;
int minimum_word_length = 3;
StringList boolean_keywords;

StringList collectionList;      // List of databases to search on

// reconised word prefixes (for field-restricted search and per-word fuzzy
// algorithms) in *descending* alphabetical order.
// Don't use a dictionary structure, as setup time outweights saving.
struct
{
  char *name;
  unsigned int flag;
} colonPrefix[] =
{
  {
  "url", FLAG_URL},
  {
  "title", FLAG_TITLE},
  {
  "text", FLAG_PLAIN},          // FLAG_TEXT is 0, i.e. *no* flag...
  {
  "link", FLAG_LINK_TEXT},
  {
  "keyword", FLAG_KEYWORDS},
  {
  "hidden", FLAG_HIDDEN},
  {
  "heading", FLAG_HEADING},
  {
  "exact", FLAG_EXACT},
  {
  "descr", FLAG_DESCRIPTION},
//    { "cap",     FLAG_CAPITAL },
  {
  "author", FLAG_AUTHOR},
  {
"", 0},};

//*****************************************************************************
// int main()
//
int
main (int ac, char **av)
{
  int c;
  extern char *optarg;
  int override_config = 0;
  // List    searchWords;
  List *searchWords = NULL;
  String configFile = DEFAULT_CONFIG_FILE;
  int pageNumber = 1;
  HtRegex limit_to;
  HtRegex exclude_these;
  String logicalWords;
  String origPattern;
  String logicalPattern;
  // StringMatch    searchWordsPattern;
  StringMatch *searchWordsPattern = NULL;
  StringList requiredWords;
  int i;
  Dictionary selected_collections;      // Multiple database support

  //
  // Parse command line arguments
  //
  while ((c = getopt (ac, av, "c:dv")) != -1)
  {
    switch (c)
    {
    case 'c':
      // The default is obviously to do this securely
      // but if people want to shoot themselves in the foot...
#ifndef ALLOW_INSECURE_CGI_CONFIG
      if (!getenv ("REQUEST_METHOD"))
      {
#endif
        configFile = optarg;
        override_config = 1;
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
      usage ();
      break;
    }
  }

  //
  // The total search can NEVER take more than 5 minutes.
  //
#ifndef _MSC_VER                /* _WIN32 */
  alarm (5 * 60);
#endif

  //
  // Parse the CGI parameters.
  //
  char none[] = "";
  cgi input (optind < ac ? av[optind] : none);

  // Multiple databases may be specified for search.
  // Identify all databases specified with the "config=" parameter.
  if (input.exists ("config"))
  {
    collectionList.Create (input["config"], " \t\001|");
  }
  if (collectionList.Count () == 0)
    collectionList.Add ("");    // use default config
  String errorMsg = "";
  String originalWords = input["words"];
  originalWords.chop (" \t\r\n");

  HtConfiguration *config = HtConfiguration::config ();

  // Iterate over all specified collections (databases)
  for (int cInd = 0; errorMsg.empty () && cInd < collectionList.Count ();
       cInd++)
  {
    // Each collection is handled in an iteration. Reset the following so
    // that we start with a clean slate.
    //
    logicalWords = 0;
    origPattern = 0;
    logicalPattern = 0;
    searchWords = new List;
    searchWordsPattern = new StringMatch;

    char *config_name = collectionList[cInd];
    if (config_name && config_name[0] == '\0')
      config_name = NULL;       // use default config

    //
    // Setup the configuration database.  First we read the compiled defaults.
    // Then we override those with defaults read in from the configuration
    // file, and finally we override some attributes with information we
    // got from the HTML form.
    //
    config->Defaults (&defaults[0]);
    // To allow . in filename while still being 'secure',
    // e.g. htdig-f.q.d.n.conf
    if (!override_config && config_name
        && (strstr (config_name, "./") == NULL))
    {
      char *configDir = getenv ("CONFIG_DIR");
      if (configDir)
      {
        configFile = configDir;
      }
      else
      {
        configFile = CONFIG_DIR;
      }
      if (strlen (config_name) == 0)
        configFile = DEFAULT_CONFIG_FILE;
      else
        configFile << '/' << config_name << ".conf";
    }
    if (access ((char *) configFile, R_OK) < 0)
    {
      reportError ("Unable to read configuration file");
    }
    config->Read (configFile);

    // Initialize htword library (key description + wordtype...)
    WordContext::Initialize (*config);

    if (input.exists ("method"))
      config->Add ("match_method", input["method"]);
    if (input.exists ("format"))
      config->Add ("template_name", input["format"]);

    if (input.exists ("matchesperpage"))
    {
      // minimum check for a valid int value of "matchesperpage" cgi variable
      if (atoi (input["matchesperpage"]) > 0)
        config->Add ("matches_per_page", input["matchesperpage"]);
    }

    if (input.exists ("page"))
      pageNumber = atoi (input["page"]);
    if (input.exists ("config"))
      config->Add ("config", input["config"]);
    if (input.exists ("restrict"))
      config->Add ("restrict", input["restrict"]);
    if (input.exists ("exclude"))
      config->Add ("exclude", input["exclude"]);
    if (input.exists ("keywords"))
      config->Add ("keywords", input["keywords"]);
    requiredWords.Create (config->Find ("keywords"), " \t\r\n\001");
    if (input.exists ("sort"))
      config->Add ("sort", input["sort"]);

    // Changes added 3-31-99, by Mike Grommet
    // Check form entries for starting date, and ending date
    // Each date consists of a month, day, and year

    if (input.exists ("startmonth"))
      config->Add ("startmonth", input["startmonth"]);
    if (input.exists ("startday"))
      config->Add ("startday", input["startday"]);
    if (input.exists ("startyear"))
      config->Add ("startyear", input["startyear"]);

    if (input.exists ("endmonth"))
      config->Add ("endmonth", input["endmonth"]);
    if (input.exists ("endday"))
      config->Add ("endday", input["endday"]);
    if (input.exists ("endyear"))
      config->Add ("endyear", input["endyear"]);

    // END OF CHANGES BY MIKE GROMMET


    minimum_word_length =
      config->Value ("minimum_word_length", minimum_word_length);

    StringList form_vars (config->Find ("allow_in_form"), " \t\r\n");
    for (i = 0; i < form_vars.Count (); i++)
    {
      if (input.exists (form_vars[i]))
        config->Add (form_vars[i], input[form_vars[i]]);
    }

    //
    // Compile the URL limit patterns.
    //

    if (config->Find ("restrict").length ())
    {
      // Create a temporary list from either the configuration
      // file or the input parameter
      StringList l (config->Find ("restrict"), " \t\r\n\001|");
      limit_to.setEscaped (l);
      String u = l.Join ('|');
      config->Add ("restrict", u);      // re-create the config attribute
    }
    if (config->Find ("exclude").length ())
    {
      // Create a temporary list from either the configuration
      // file or the input parameter
      StringList l (config->Find ("exclude"), " \t\r\n\001|");
      exclude_these.setEscaped (l);
      String u = l.Join ('|');
      config->Add ("exclude", u);       // re-create the config attribute
    }

    //
    // Check url_part_aliases and common_url_parts for
    // errors.
    String url_part_errors = HtURLCodec::instance ()->ErrMsg ();

    if (url_part_errors.length () != 0)
      reportError (form ("Invalid url_part_aliases or common_url_parts: %s",
                         url_part_errors.get ()));

    // for htsearch, use search_rewrite_rules attribute for HtURLRewriter.
    config->AddParsed ("url_rewrite_rules", "${search_rewrite_rules}");
    url_part_errors = HtURLRewriter::instance ()->ErrMsg ();
    if (url_part_errors.length () != 0)
      reportError (form ("Invalid url_rewrite_rules: %s",
                         url_part_errors.get ()));

    // Load boolean_keywords from configuration
    // they should be placed in this order:
    //    0       1       2
    //    and     or      not
    boolean_keywords.Destroy ();
    boolean_keywords.Create (config->Find ("boolean_keywords"),
                             "| \t\r\n\001");
    if (boolean_keywords.Count () != 3)
      reportError ("boolean_keywords attribute should have three entries");

    Parser *parser = new Parser ();

    //
    // Parse the words to search for from the argument list.
    // This will produce a list of WeightWord objects.
    //
    setupWords (originalWords, *searchWords,
                strcmp (config->Find ("match_method"), "boolean") == 0,
                parser, origPattern);

    //
    // Convert the list of WeightWord objects to a pattern string
    // that we can compile.
    //
    createLogicalWords (*searchWords, logicalWords, logicalPattern);

    //
    // Assemble the full pattern for excerpt matching and highlighting
    //
    origPattern += logicalPattern;
    searchWordsPattern->IgnoreCase ();
    searchWordsPattern->IgnorePunct ();
    searchWordsPattern->Pattern (logicalPattern);       // this should now be enough
    //searchWordsPattern.Pattern(origPattern);
    //if (debug > 2)
    //  cout << "Excerpt pattern: " << origPattern << "\n";

    //
    // If required keywords were given in the search form, we will
    // modify the current searchWords list to include the required
    // words.
    //
    if (requiredWords.Count () > 0)
    {
      addRequiredWords (*searchWords, requiredWords);
    }

    //
    // Perform the actual search.  The function htsearch() is used for this.
    // The Dictionary it returns is then passed on to the Display object to
    // actually render the results in HTML.
    //
    const String word_db = config->Find ("word_db");
    if (access (word_db, R_OK) < 0)
    {
      reportError (form
                   ("Unable to read word database file '%s'\nDid you run htdig?",
                    word_db.get ()));
    }
    // ResultList  *results = htsearch((char*)word_db, searchWords, parser);

    String doc_index = config->Find ("doc_index");
    if (access ((char *) doc_index, R_OK) < 0)
    {
      reportError (form
                   ("Unable to read document index file '%s'\nDid you run htdig?",
                    doc_index.get ()));
    }

    const String doc_db = config->Find ("doc_db");
    if (access (doc_db, R_OK) < 0)
    {
      reportError (form
                   ("Unable to read document database file '%s'\nDid you run htdig?",
                    doc_db.get ()));
    }

    const String doc_excerpt = config->Find ("doc_excerpt");
    if (access (doc_excerpt, R_OK) < 0)
    {
      reportError (form
                   ("Unable to read document excerpts '%s'\nDid you run htdig?",
                    doc_excerpt.get ()));
    }

    // Multiple database support
    Collection *collection = new Collection ((char *) configFile,
                                             word_db.get (), doc_index.get (),
                                             doc_db.get (),
                                             doc_excerpt.get ());

    // Perform search within the collection. Each collection stores its
    // own result list.
    htsearch (collection, *searchWords, parser);
    collection->setSearchWords (searchWords);
    collection->setSearchWordsPattern (searchWordsPattern);
    selected_collections.Add (configFile, collection);

    if (parser->hadError ())
      errorMsg = parser->getErrorMessage ();

    delete parser;
  }

  // Display  display(doc_db, 0, doc_excerpt);
  Display display (&selected_collections);
  if (display.hasTemplateError ())
  {
    reportError (form ("Unable to read template file '%s'\nDoes it exist?",
                       (const char *) config->Find ("template_name")));
    return 0;
  }
  display.setOriginalWords (originalWords);
  // display.setResults(results);
  // display.setSearchWords(&searchWords);
  display.setLimit (&limit_to);
  display.setExclude (&exclude_these);
  // display.setAllWordsPattern(searchWordsPattern);
  display.setCGI (&input);
  display.setLogicalWords (logicalWords);
  if (!errorMsg.empty ())
    display.displaySyntaxError (errorMsg);
  else
    display.display (pageNumber);

  // delete results;
  // delete parser;
  return 0;
}

//*****************************************************************************
void
createLogicalWords (List & searchWords, String & logicalWords, String & wm)
{
  String pattern;
  int i;
  int wasHidden = 0;
  int inPhrase = 0;

  for (i = 0; i < searchWords.Count (); i++)
  {
    WeightWord *ww = (WeightWord *) searchWords[i];
    if (!ww->isHidden)
    {
      if (strcmp ((char *) ww->word, "&") == 0 && wasHidden == 0)
        logicalWords << ' ' << boolean_keywords[AND] << ' ';
      else if (strcmp ((char *) ww->word, "|") == 0 && wasHidden == 0)
        logicalWords << ' ' << boolean_keywords[OR] << ' ';
      else if (strcmp ((char *) ww->word, "!") == 0 && wasHidden == 0)
        logicalWords << ' ' << boolean_keywords[NOT] << ' ';
      else if (strcmp ((char *) ww->word, "\"") == 0 && wasHidden == 0)
      {
        if (inPhrase)
          logicalWords.chop (' ');
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
    // generate patterns to search for and highlight in excerpt
    if (ww->weight > 0          // Ignore boolean syntax stuff
        && (!ww->isIgnore || inPhrase)) // Ignore bad/short words
    {                           // but highlight them in phrases
      char spacer = inPhrase ? ' ' : '|';
      if (wm.length ())
        wm << spacer;
      wm << ww->word;
      if (!ww->isIgnore)        // ignore bad/short words for searching
      {
        if (pattern.length ())
          pattern << spacer;
        pattern << ww->word;
      }
    }
  }

  if (debug)
  {
    cerr << "LogicalWords: " << logicalWords << endl;
    cerr << "Pattern: " << pattern << endl;
    cerr << "Highlight Pattern: " << wm << endl;
  }
}

void
dumpWords (List & words, char *msg = "")
{
  if (debug)
  {
    cerr << msg << ": '";
    for (int i = 0; i < words.Count (); i++)
    {
      WeightWord *ww = (WeightWord *) words[i];
      cerr << ww->word << ':' << ww->isHidden << ' ';
    }
    cerr << "'\n";
  }
}

//*****************************************************************************
// void setupWords(char *allWords, List &searchWords,
//       int boolean, Parser *parser, String &originalPattern)
//
void
setupWords (char *allWords, List & searchWords, int boolean, Parser * parser,
            String & originalPattern)
{
  HtConfiguration *config = HtConfiguration::config ();
  List tempWords;
  int i;

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
  unsigned char *pos = (unsigned char *) allWords;
  unsigned char t;
  String word;
  const String prefix_suffix = config->Find ("prefix_match_character");

  while (*pos)
  {
    while (1)
    {
      if (debug > 3)
        cerr << "setupWords: " << pos << endl;
      t = *pos++;
      if (isspace (t))
      {
        continue;
      }
      else if (t == '"')
      {
        tempWords.Add (new WeightWord ("\"", -1.0));
        break;
      }
      else if (boolean && (t == '(' || t == ')'))
      {
        char s[2];
        s[0] = t;
        s[1] = '\0';
        tempWords.Add (new WeightWord (s, -1.0));
        break;
      }
      else if (HtIsWordChar (t) ||
               (strchr (prefix_suffix, t) != NULL) || (t >= 161 && t <= 255))
      {
        unsigned int fieldFlag = 0;
        word = 0;
        do                      // while recognised prefix, followed by ':'
        {
          while (t && (HtIsWordChar (t) ||
                       (strchr (prefix_suffix, t) != NULL) ||
                       (t >= 161 && t <= 255)))
          {
            word << (char) t;
            t = *pos++;
          }
          if (debug > 2)
            cerr << "word: " << word << endl;
          if (t == ':')         // e.g. "author:word" to search
          {                     // only in author
            word.lowercase ();
            t = *pos++;
            if (t && (HtIsWordChar (t) ||
                      (strchr (prefix_suffix, t) != NULL) ||
                      (t >= 161 && t <= 255)))
            {
              int i, cmp;
              const char *w = word.get ();
              // linear search of known prefixes, with "" flag.
              for (i = 0; (cmp = mystrcasecmp (w, colonPrefix[i].name)) < 0;
                   i++)
                ;
              if (debug > 2)
                cerr << "field: " << colonPrefix[i].name << endl;
              if (cmp == 0)     // if prefix found...
              {
                fieldFlag |= colonPrefix[i].flag;
                word = 0;
              }
            }
          }
        }
        while (!word.length () && t);
        pos--;
        if (!t && !word.length ())      // query ended with junk chars
          break;

        if (boolean && (mystrcasecmp (word.get (), "+") == 0
                        || mystrcasecmp (word.get (),
                                         boolean_keywords[AND]) == 0))
        {
          tempWords.Add (new WeightWord ("&", -1.0));
        }
        else if (boolean &&
                 mystrcasecmp (word.get (), boolean_keywords[OR]) == 0)
        {
          tempWords.Add (new WeightWord ("|", -1.0));
        }
        else if (boolean && (mystrcasecmp (word.get (), "-") == 0
                             || mystrcasecmp (word.get (),
                                              boolean_keywords[NOT]) == 0))
        {
          tempWords.Add (new WeightWord ("!", -1.0));
        }
        else
        {
          // Add word to excerpt matching list
          originalPattern << word << "|";
          WeightWord *ww = new WeightWord (word, 1.0, fieldFlag);
          if (HtWordNormalize (word) & WORD_NORMALIZE_NOTOK)
            ww->isIgnore = 1;
          tempWords.Add (ww);
        }
        break;
      }
    }
  }

  dumpWords (tempWords, "tempWords");

  //
  // If the user specified boolean expression operators, the whole
  // expression has to be syntactically correct.  If not, we need
  // to report a syntax error.
  //
  if (boolean)
  {
    if (!parser->checkSyntax (&tempWords))
    {
      for (i = 0; i < tempWords.Count (); i++)
      {
        searchWords.Add (tempWords[i]);
      }
      tempWords.Release ();
      return;
//      reportError("Syntax error");
    }
  }
  else
  {
    convertToBoolean (tempWords);
  }

  dumpWords (tempWords, "Boolean");

  //
  // We need to assign weights to the words according to the search_algorithm
  // configuration attribute.
  // For algorithms other than exact, we need to also do word lookups.
  //
  StringList algs (config->Find ("search_algorithm"), " \t");
  List algorithms;
  String name, weight;
  double fweight;
  Fuzzy *fuzzy = 0;

  //
  // Generate the list of algorithms to use and associate the given
  // weights with them.
  //
  for (i = 0; i < algs.Count (); i++)
  {
    name = strtok (algs[i], ":");
    weight = strtok (0, ":");
    if (name.length () == 0)
      name = "exact";
    if (weight.length () == 0)
      weight = "1";
    fweight = atof ((char *) weight);

    fuzzy = Fuzzy::getFuzzyByName (name, *config);
    if (fuzzy)
    {
      if (debug > 1)
        cerr << "Adding algorithm " << name.get () << endl;
      fuzzy->setWeight (fweight);
      fuzzy->openIndex ();
      algorithms.Add (fuzzy);
    }
    else if (debug)
      cerr << "Unknown fuzzy search algorithm " << name.get () << endl;
  }

  dumpWords (searchWords, "initial");

  //
  // For each of the words, apply all the algorithms.
  //
  int in_phrase = 0;            // If we get into a phrase, we don't want to fuzz.
  for (i = 0; i < tempWords.Count (); i++)
  {
    WeightWord *ww = (WeightWord *) tempWords[i];
    if (ww->weight > 0 && !ww->isIgnore && !in_phrase)
//    I think that should be:
//  if (ww->weight > 0 && !ww->isIgnore && !in_phrase && !ww->isExact)
    {
      //
      // Apply all the algorithms to the word.
      //
      if (debug)
        cerr << "Fuzzy on: " << ww->word << endl;
      doFuzzy (ww, searchWords, algorithms);
      delete ww;
    }
    else if (ww->word.length () == 1 && ww->word[0] == '"')
    {
      in_phrase = !in_phrase;
      if (debug)
        cerr << "Add: " << ww->word << endl;
      searchWords.Add (ww);
    }
    else
    {
      //
      // This is '(', ')', '&', or '|'.  These will be automatically
      // transfered to the searchWords list.
      //
      if (debug)
        cerr << "Add: " << ww->word << endl;
      searchWords.Add (ww);
    }
    dumpWords (searchWords, "searchWords");
  }
  tempWords.Release ();
}


//*****************************************************************************
void
doFuzzy (WeightWord * ww, List & searchWords, List & algorithms)
{
  List fuzzyWords;
  List weightWords;
  Fuzzy *fuzzy = 0;
  WeightWord *newWw = 0;
  String *word = 0;

  algorithms.Start_Get ();
  while ((fuzzy = (Fuzzy *) algorithms.Get_Next ()))
  {
    if (debug > 1)
      cerr << "   " << fuzzy->getName ();
    fuzzy->getWords (ww->word, fuzzyWords);
    fuzzyWords.Start_Get ();
    while ((word = (String *) fuzzyWords.Get_Next ()))
    {
      if (debug > 1)
        cerr << " " << word->get ();
      // (should be a "copy with changed weight" constructor...)
      newWw = new WeightWord (word->get (), fuzzy->getWeight ());
      newWw->isExact = ww->isExact;
      newWw->isHidden = ww->isHidden;
      newWw->flags = ww->flags;
      weightWords.Add (newWw);
    }
    if (debug > 1)
      cerr << endl;
    fuzzyWords.Destroy ();
  }

  //
  // We now have a list of substitute words.  They need to be added
  // to the searchWords.
  //
  if (weightWords.Count ())
  {
    if (weightWords.Count () > 1)
      searchWords.Add (new WeightWord ("(", -1.0));
    for (int i = 0; i < weightWords.Count (); i++)
    {
      if (i > 0)
        searchWords.Add (new WeightWord ("|", -1.0));
      searchWords.Add (weightWords[i]);
    }
    if (weightWords.Count () > 1)
      searchWords.Add (new WeightWord (")", -1.0));
  }
  else                          // if no fuzzy matches, add exact word, but give it tiny weight
  {
    searchWords.Add (new WeightWord (ww->word.get (), 0.000001));
  }
  weightWords.Release ();
}


//*****************************************************************************
// void convertToBoolean(List &words)
//
void
convertToBoolean (List & words)
{
  HtConfiguration *config = HtConfiguration::config ();
  List list;
  int i;
  int do_and = strcmp (config->Find ("match_method"), "and") == 0;
  int in_phrase = 0;

  String quote = "\"";

  if (words.Count () == 0)
    return;
  list.Add (words[0]);

  // We might start off with a phrase match
  if (((WeightWord *) words[0])->word == quote)
    in_phrase = 1;

  for (i = 1; i < words.Count (); i++)
  {
    if (do_and && !in_phrase)
      list.Add (new WeightWord ("&", -1.0));
    else if (!in_phrase)
      list.Add (new WeightWord ("|", -1.0));

    if (((WeightWord *) words[i])->word == quote)
      in_phrase = !in_phrase;

    list.Add (words[i]);
  }
  words.Release ();

  for (i = 0; i < list.Count (); i++)
  {
    words.Add (list[i]);
  }
  list.Release ();
}


//*****************************************************************************
// Dictionary *htsearch(char *wordfile, List &searchWords, Parser *parser)
//   This returns a dictionary indexed by document ID and containing a
//   List of HtWordReference objects.
//
void
htsearch (Collection * collection, List & searchWords, Parser * parser)
{
  //
  // Pick the database type we are going to use
  //
  ResultList *matches = new ResultList;
  if (searchWords.Count () > 0)
  {
    // parser->setDatabase(wordfile);
    parser->setCollection (collection);
    parser->parse (&searchWords, *matches);
  }

  collection->setResultList (matches);
  // return matches;
}


//*****************************************************************************
// Modify the search words list to include the required words as well.
// This is done by putting the existing search words in parenthesis and
// appending the required words separated with "and".
void
addRequiredWords (List & searchWords, StringList & requiredWords)
{
  HtConfiguration *config = HtConfiguration::config ();
  static int any_keywords = config->Boolean ("any_keywords", 0);
  if (requiredWords.Count () == 0)
    return;
  if (searchWords.Count () > 0)
  {
    searchWords.Insert (new WeightWord ("(", -1.0), 0);
    searchWords.Add (new WeightWord (")", -1.0));
    searchWords.Add (new WeightWord ("&", -1.0));
  }
  if (requiredWords.Count () == 1)
  {
    searchWords.Add (new WeightWord (requiredWords[0], 1.0));
  }
  else
  {
    searchWords.Add (new WeightWord ("(", -1.0));
    searchWords.Add (new WeightWord (requiredWords[0], 1.0));
    for (int i = 1; i < requiredWords.Count (); i++)
    {
      if (any_keywords)
        searchWords.Add (new WeightWord ("|", -1.0));
      else
        searchWords.Add (new WeightWord ("&", -1.0));
      searchWords.Add (new WeightWord (requiredWords[i], 1.0));
    }
    searchWords.Add (new WeightWord (")", -1.0));
  }
}


//*****************************************************************************
// Report an error.  Since we don' know if we are running as a CGI or not,
// we will assume this is the first thing returned by a CGI program.
//
void
reportError (char *msg)
{
  HtConfiguration *config = HtConfiguration::config ();
  cout << "Content-type: text/html\r\n\r\n";
  cout << "<html><head><title>htsearch error</title></head>\n";
  cout << "<body bgcolor=\"#ffffff\">\n";
  cout << "<h1>ht://Dig error</h1>\n";
  cout << "<p>htsearch detected an error.  Please report this to the\n";
  cout << "webmaster of this site by sending an e-mail to:\n";
  cout << "<a href=\"mailto:" << config->Find ("maintainer") << "\">";
  cout << config->Find ("maintainer") << "</a>\n";
  cout << "The error message is:</p>\n";
  cout << "<pre>\n" << msg << "\n</pre>\n</body></html>\n";
  exit (1);
}

//*****************************************************************************
// void usage()
//   Display program usage information--assumes we're running from a cmd line
//
void
usage ()
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
  cout <<
    "\tquery_string\tA CGI-style query string can be given as a single\n";
  cout <<
    "\t\targument, and is only used if the REQUEST_METHOD environment\n";
  cout << "\t\tvariable is not set.  If no query_string is given, and\n";
  cout <<
    "\t\tREQUEST_METHOD is not set, htsearch will prompt for the query.\n\n";
  exit (0);
}
