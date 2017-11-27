//----------------------------------------------------------------
//
// libhtdig_htsearch.cc
//
// 1/25/2002 created from htsearch.cc
//
// Neal Richter nealr@rightnow.com
//
//
// htsearch: The main search CGI. Parses the CGI input, reads the config files
//           and calls the necessary code to put together the result lists
//           and the final display.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: libhtdig_htsearch.cc,v 1.4 2004/05/28 13:15:29 lha Exp $
//
//----------------------------------------------------------------

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

extern "C"
{
#include "libhtdig_api.h"
}

#include "libhtdig_log.h"


#include "htsearch.h"
#include "defaults.h"
#include "WeightWord.h"
#include "parser.h"
#include "ResultFetch.h"
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

//define _XOPEN_SOURCE
//#define _GNU_SOURCE
#include <time.h>
#include <ctype.h>
#include <signal.h>

#ifndef _WIN32
#include <unistd.h>
#endif


// If we have this, we probably want it.
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

typedef void (*SIGNAL_HANDLER) (...);

// ResultList *htsearch(const String&, List &, Parser *);
int htsearch (Collection *, List &, Parser *);

void setupWords (char *, List &, int, Parser *, String &);
void createLogicalWords (List &, String &, String &);
void reportError (char *);
void convertToBoolean (List & words);
void doFuzzy (WeightWord *, List &, List &);
void addRequiredWords (List &, StringList &);

int minimum_word_length = 3;

StringList boolean_keywords;

Parser *parser = NULL;

extern String configFile;
extern int debug;

static HtConfiguration *config = NULL;
Dictionary selected_collections;        // Multiple database support
Collection *collection = NULL;
String errorMsg;

String originalWords;
String origPattern;
String logicalWords;
String logicalPattern;
StringMatch *searchWordsPattern = NULL;
StringList requiredWords;       //TODO add this

HtRegex limit_to;
HtRegex exclude_these;

// List         searchWords;
List *searchWords = NULL;

StringList collectionList;      // List of databases to search on


static int total_matches = 0;
static List *matches_list = 0;
static ResultFetch *resultfetch = 0;


//*****************************************************************************
// int main()
//
//int main(int ac, char **av)
int
htsearch_open (htsearch_parameters_struct * htsearch_parms)
{
  int ret = -1;
  int override_config = 0;

  String logicalWords;
  String logicalPattern;
  // StringMatch       searchWordsPattern;
  StringMatch *searchWordsPattern = NULL;
  StringList requiredWords;
  //int i;
  //int c;
  int cInd = 0;

  //load 'comand-line' parameters

  if (htsearch_parms->configFile[0] != 0)
    configFile = htsearch_parms->configFile;

  debug = htsearch_parms->debug;
  if (debug != 0)
  {
    ret = logOpen (htsearch_parms->logFile);

    if (ret == FALSE)
    {
      reportError (form
                   ("[HTDIG] Error opening log file [%s] . Error:[%d], %s\n",
                    htsearch_parms->logFile, errno, strerror (errno)));
      return (HTSEARCH_ERROR_LOGFILE_OPEN);
    }
  }


  //case 'c':
  // The default is obviously to do this securely
  // but if people want to shoot themselves in the foot...
  //    configFile = optarg;
  //    override_config = 1;

  //
  // The total search can NEVER take more than 5 minutes.
  //
  //alarm(5 * 60);

  errorMsg = "";

  config = HtConfiguration::config ();

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
    config_name = NULL;         // use default config

  //
  // Setup the configuration database.  First we read the compiled defaults.
  // Then we override those with defaults read in from the configuration
  // file, and finally we override some attributes with information we
  // got from the HTML form.
  //
  config->Defaults (&defaults[0]);
  // To allow . in filename while still being 'secure',
  // e.g. htdig-f.q.d.n.conf
  if (!override_config && config_name && (strstr (config_name, "./") == NULL))
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
    reportError (form
                 ("Unable to read configuration file '%s'",
                  configFile.get ()));
    return (HTSEARCH_ERROR_CONFIG_READ);
  }
  config->Read (configFile);


  //---------- Now override config settings -----------------

  //------- override database path ------------
  if (strlen (htsearch_parms->DBpath) > 0)
  {
    config->Add ("database_dir", htsearch_parms->DBpath);
  }

  //------- custom filters from htsearch_parms ----------

  //resrict,exclude,urlrewrite


  if (strlen (htsearch_parms->meta_description_factor) > 0)
  {
    config->Add ("meta_description_factor",
                 htsearch_parms->meta_description_factor);
  }

  if (strlen (htsearch_parms->title_factor) > 0)
  {
    config->Add ("title_factor", htsearch_parms->title_factor);
  }

  if (strlen (htsearch_parms->text_factor) > 0)
  {
    config->Add ("text_factor", htsearch_parms->text_factor);
  }

  if (strlen (htsearch_parms->locale) > 0)
  {
    config->Add ("locale", htsearch_parms->locale);
  }

  //-------------------------------------------------------------------


  // Initialize htword library (key description + wordtype...)
  WordContext::Initialize (*config);

//NON-CGI Usage  libhtdig
/*

        config->Add("match_method", input["method"]);
        config->Add("template_name", input["format"]);

        // minimum check for a valid int value of "matchesperpage" cgi variable
        if (atoi(input["matchesperpage"]) > 0)
            config->Add("matches_per_page", input["matchesperpage"]);

        pageNumber = atoi(input["page"]);
        config->Add("config", input["config"]);
        config->Add("restrict", input["restrict"]);
        config->Add("exclude", input["exclude"]);
        config->Add("keywords", input["keywords"]);
        requiredWords.Create(config->Find("keywords"), " \t\r\n\001");
        config->Add("sort", input["sort"]);

        config->Add("startmonth", input["startmonth"]);
        config->Add("startday", input["startday"]);
        config->Add("startyear", input["startyear"]);

        config->Add("endmonth", input["endmonth"]);
        config->Add("endday", input["endday"]);
        config->Add("endyear", input["endyear"]);


        StringList form_vars(config->Find("allow_in_form"), " \t\r\n");
        for (i = 0; i < form_vars.Count(); i++)
        {
            if (input.exists(form_vars[i]))
                config->Add(form_vars[i], input[form_vars[i]]);
        }

*/
//NON-CGI Usage  libhtdig


  minimum_word_length =
    config->Value ("minimum_word_length", minimum_word_length);

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
    config->Add ("restrict", u);        // re-create the config attribute
  }
  if (config->Find ("exclude").length ())
  {
    // Create a temporary list from either the configuration
    // file or the input parameter
    StringList l (config->Find ("exclude"), " \t\r\n\001|");
    exclude_these.setEscaped (l);
    String u = l.Join ('|');
    config->Add ("exclude", u); // re-create the config attribute
  }

  //
  // Check url_part_aliases and common_url_parts for
  // errors.
  String url_part_errors = HtURLCodec::instance ()->ErrMsg ();

  if (url_part_errors.length () != 0)
  {
    reportError (form
                 ("Invalid url_part_aliases or common_url_parts: %s",
                  url_part_errors.get ()));
    return (HTSEARCH_ERROR_URL_PART);

  }

  // for htsearch, use search_rewrite_rules attribute for HtURLRewriter.
  config->AddParsed ("url_rewrite_rules", "${search_rewrite_rules}");
  url_part_errors = HtURLRewriter::instance ()->ErrMsg ();
  if (url_part_errors.length () != 0)
    reportError (form
                 ("Invalid url_rewrite_rules: %s", url_part_errors.get ()));

  // Load boolean_keywords from configuration
  // they should be placed in this order:
  //    0       1       2
  //    and     or      not
  boolean_keywords.Create (config->Find ("boolean_keywords"), "| \t\r\n\001");
  if (boolean_keywords.Count () != 3)
    reportError ("boolean_keywords attribute should have three entries");



  parser = new Parser ();

  return (TRUE);
}

//---------------------------------------------------------------------------------------
//
//
//  RETURN:  Number of Documents resulted from search
//
//---------------------------------------------------------------------------------------

int
htsearch_query (htsearch_query_struct * htseach_query)
{
  int total_match_count = 0;

  originalWords = htseach_query->raw_query;
  originalWords.chop (" \t\r\n");

  //sort
  switch (htseach_query->sortby_flag)
  {
  case HTSEARCH_SORT_SCORE:
    config->Add ("sort", "score");
    break;
  case HTSEARCH_SORT_REV_SCORE:
    config->Add ("sort", "revscore");
    break;
  case HTSEARCH_SORT_TIME:
    config->Add ("sort", "time");
    break;
  case HTSEARCH_SORT_REV_TIME:
    config->Add ("sort", "revtime");
    break;
  case HTSEARCH_SORT_TITLE:
    config->Add ("sort", "title");
    break;
  case HTSEARCH_SORT_REV_TITLE:
    config->Add ("sort", "revtitle");
    break;
  }


  switch (htseach_query->algorithms_flag)
  {
  case HTSEARCH_ALG_BOOLEAN:
    config->Add ("match_method", "boolean");
    break;
  case HTSEARCH_ALG_OR:
    config->Add ("match_method", "or");
    break;
  case HTSEARCH_ALG_AND:
    config->Add ("match_method", "and");
    break;
  }

  //format
  switch (htseach_query->algorithms_flag)
  {
  case HTSEARCH_FORMAT_SHORT:
    config->Add ("template_name", "builtin-short");
    break;
  case HTSEARCH_FORMAT_LONG:
    config->Add ("template_name", "builtin-long");
    break;
  }


  origPattern = 0;
  logicalWords = 0;
  logicalPattern = 0;
  searchWordsPattern = new StringMatch;

  // Iterate over all specified collections (databases)
  //for (int cInd = 0; errorMsg.empty() && cInd < collectionList.Count(); cInd++)
  //{

  // Parse the words to search for from the argument list.
  // This will produce a list of WeightWord objects.
  //
  setupWords (originalWords, *searchWords,
              strcmp (config->Find ("match_method"), "boolean") == 0, parser,
              origPattern);

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
  searchWordsPattern->Pattern (logicalPattern); // this should now be enough
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
    return (HTSEARCH_ERROR_WORDDB_READ);
  }
  // ResultList   *results = htsearch((char*)word_db, searchWords, parser);

  String doc_index = config->Find ("doc_index");
  if (access ((char *) doc_index, R_OK) < 0)
  {
    reportError (form
                 ("Unable to read document index file '%s'\nDid you run htdig?",
                  doc_index.get ()));
    return (HTSEARCH_ERROR_DOCINDEX_READ);
  }

  const String doc_db = config->Find ("doc_db");
  if (access (doc_db, R_OK) < 0)
  {
    reportError (form
                 ("Unable to read document database file '%s'\nDid you run htdig?",
                  doc_db.get ()));
    return (HTSEARCH_ERROR_DOCDB_READ);
  }

  const String doc_excerpt = config->Find ("doc_excerpt");
  if (access (doc_excerpt, R_OK) < 0)
  {
    reportError (form
                 ("Unable to read document excerpts '%s'\nDid you run htdig?",
                  doc_excerpt.get ()));
    return (HTSEARCH_ERROR_EXCERPTDB_READ);
  }

  // Multiple database support
  collection = new Collection ((char *) configFile,
                               word_db.get (), doc_index.get (),
                               doc_db.get (), doc_excerpt.get ());

  // Perform search within the collection. Each collection stores its
  // own result list.
  total_match_count += htsearch (collection, *searchWords, parser);
  collection->setSearchWords (searchWords);
  collection->setSearchWordsPattern (searchWordsPattern);
  selected_collections.Add (configFile, collection);

  if (parser->hadError ())
    errorMsg = parser->getErrorMessage ();

  delete parser;
  //}


  total_matches = total_match_count;

  if (total_matches > 0)
  {

    resultfetch = new ResultFetch (&selected_collections, collectionList);

    if (resultfetch->hasTemplateError ())
    {
      reportError (form ("Unable to read template file '%s'\nDoes it exist?",
                         (const char *) config->Find ("template_name")));

      return (HTSEARCH_ERROR_TEMPLATE_ERROR);
    }
    resultfetch->setOriginalWords (originalWords);
    resultfetch->setLimit (&limit_to);
    resultfetch->setExclude (&exclude_these);
    resultfetch->setLogicalWords (logicalWords);
    if (!errorMsg.empty ())
      resultfetch->displaySyntaxError (errorMsg);
    else
    {

      matches_list = resultfetch->fetch ();

      //matches_list->Start_Get();

    }

  }                             //if ((total_matches > 0) && (desired_match_index == 0))


  return (total_match_count);
}

//------------------  htsearch_get_nth_match (...)  -------------------------------------
//
//  Parameters
//        result_desired_index   ZERO based results index.
//        query_result           structure to fill with result
//
//  htsearch_query_match_struct:
//        char title[HTDIG_DOCUMENT_TITLE_L];
//        char URL[HTDIG_MAX_FILENAME_PATH_L];
//        char excerpt[HTDIG_DOCUMENT_EXCERPT_L];
//        int  score;
//        int  match_percent;     //top result is 100%
//        time_t doc_date;
//        int  size;
//        
//---------------------------------------------------------------------------------------

int
htsearch_get_nth_match (int desired_match_index,
                        htsearch_query_match_struct * query_result)
{

  ResultMatch *match = 0;
  Dictionary *vars = 0;

  if (total_matches == 0)
  {
    return (HTSEARCH_ERROR_NO_MATCH);
  }
  else if (desired_match_index >= total_matches)
  {
    return (HTSEARCH_ERROR_BAD_MATCH_INDEX);
  }
  else if ((total_matches > 0) && (desired_match_index < total_matches))
  {
    match = (ResultMatch *) matches_list->Nth (desired_match_index);

    // DocumentRef  *ref = docDB[match->getID()];
    Collection *collection = match->getCollection ();
    DocumentRef *ref = collection->getDocumentRef (match->getID ());
    if (!ref || ref->DocState () != Reference_normal)
    {
      // The document isn't present or shouldn't be displayed
      return (HTSEARCH_ERROR_BAD_DOCUMENT);
    }

    ref->DocAnchor (match->getAnchor ());
    ref->DocScore (match->getScore ());
    vars = resultfetch->fetchMatch (match, ref, desired_match_index);
    delete ref;

    String *value;
    String key;

    key = "NSTARS";
    value = (String *) vars->Find (key);
    //cout << key.get() << "[" << value->get() << "]" << endl;
    query_result->score = atoi (value->get ());

    key = "PERCENT";
    value = (String *) vars->Find (key);
    //cout << key.get() << "[" << value->get() << "]" << endl;
    query_result->score_percent = atoi (value->get ());

    key = "TITLE";
    value = (String *) vars->Find (key);
    //cout << key.get() << "[" << value->get() << "]" << endl;
    snprintf (query_result->title, HTDIG_DOCUMENT_TITLE_L, "%s",
              value->get ());

    key = "EXCERPT";
    value = (String *) vars->Find (key);
    //cout << key.get() << "[" << value->get() << "]" << endl;
    snprintf (query_result->excerpt, HTDIG_DOCUMENT_EXCERPT_L, "%s",
              value->get ());

    key = "URL";
    value = (String *) vars->Find (key);
    //cout << key.get() << "[" << value->get() << "]" << endl;
    snprintf (query_result->URL, HTDIG_MAX_FILENAME_PATH_L, "%s",
              value->get ());

    String datefmt = config->Find ("date_format");
    key = "MODIFIED";
    value = (String *) vars->Find (key);
    //cout << key.get() << "[" << value->get() << "]" << endl;
    mystrptime (value->get (), datefmt.get (), &(query_result->time_tm));
    //cout << "[" << asctime(&query_result->time_tm) << "]" << endl;

    key = "SIZE";
    value = (String *) vars->Find (key);
    //cout << key.get() << "[" << value->get() << "]" << endl;
    query_result->size = atoi (value->get ());


  }

  return (TRUE);
}

//---------------------------------------------------------------------------------------
//
//
//  RETURN:  TRUE or FALSE
//
//---------------------------------------------------------------------------------------

int
htsearch_close ()
{


  // delete results;
  // delete parser;


  return (TRUE);

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
    if (ww->weight > 0          // Ignore boolean syntax stuff
        && !ww->isIgnore)       // Ignore short or bad words
    {
      if (pattern.length () && !inPhrase)
        pattern << '|';
      else if (pattern.length () && inPhrase)
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
//           int boolean, Parser *parser, String &originalPattern)
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
      else if (HtIsWordChar (t) || t == ':' ||
               (strchr (prefix_suffix, t) != NULL) || (t >= 161 && t <= 255))
      {
        word = 0;
        while (t && (HtIsWordChar (t) ||
                     t == ':' || (strchr (prefix_suffix, t) != NULL)
                     || (t >= 161 && t <= 255)))
        {
          word << (char) t;
          t = *pos++;
        }

        pos--;

        if (boolean && (mystrcasecmp (word.get (), "+") == 0
                        || mystrcasecmp (word.get (),
                                         boolean_keywords[AND]) == 0))
        {
          tempWords.Add (new WeightWord ("&", -1.0));
        }
        else if (boolean
                 && mystrcasecmp (word.get (), boolean_keywords[OR]) == 0)
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
          WeightWord *ww = new WeightWord (word, 1.0);
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
//             reportError("Syntax error");
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
      fuzzy->setWeight (fweight);
      fuzzy->openIndex ();
      algorithms.Add (fuzzy);
    }
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
  Fuzzy *fuzzy;
  WeightWord *newWw;
  String *word;

  algorithms.Start_Get ();
  while ((fuzzy = (Fuzzy *) algorithms.Get_Next ()))
  {
    if (debug > 1)
      cout << "   " << fuzzy->getName ();
    fuzzy->getWords (ww->word, fuzzyWords);
    fuzzyWords.Start_Get ();
    while ((word = (String *) fuzzyWords.Get_Next ()))
    {
      if (debug > 1)
        cout << " " << word->get ();
      newWw = new WeightWord (word->get (), fuzzy->getWeight ());
      newWw->isExact = ww->isExact;
      newWw->isHidden = ww->isHidden;
      weightWords.Add (newWw);
    }
    if (debug > 1)
      cout << endl;
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
    searchWords.Add (new WeightWord (word->get (), 0.000001));
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
int
htsearch (Collection * collection, List & searchWords, Parser * parser)
{
  int count = 0;

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

  count = matches->Count ();

  return (count);
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
reportError_html (char *msg)
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
