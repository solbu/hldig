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
// $Id: libhtdig_htsearch.cc,v 1.5.2.1 2006/04/25 22:03:10 aarnone Exp $
//
//----------------------------------------------------------------

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

extern "C"
{
#include "libhtdig_api.h"
}
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string>
using namespace std;

#include "libhtdig_log.h"
#include "CLuceneAPI.h"
#include "WordType.h"


//#include "htsearch.h"
#include "defaults.h"
//#include "WeightWord.h"
//#include "parser.h"
//#include "ResultFetch.h"
#include "cgi.h"
//#include "WordRecord.h"
//#include "HtWordList.h"
#include "StringList.h"
#include "IntObject.h"
#include "HtURLCodec.h"
#include "HtURLRewriter.h"
//#include "WordContext.h"
#include "HtRegex.h"
//#include "Collection.h"

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
//int htsearch(Collection *, List &, Parser *);

//void setupWords(char *, List &, int, Parser *, String &);
//void createLogicalWords(List &, String &, String &);
//void reportError(char *);
//void convertToBoolean(List & words);
//void doFuzzy(WeightWord *, List &, List &);
//void addRequiredWords(List &, StringList &);

int minimum_word_length = 3;

StringList boolean_keywords;


extern String configFile;
extern int debug;

static HtConfiguration *config = NULL;
//Dictionary selected_collections;	// Multiple database support
//Collection *collection = NULL;
String errorMsg;

String originalWords;
String origPattern;
String logicalWords;
String logicalPattern;
StringMatch *searchWordsPattern = NULL;
StringList requiredWords;	  //TODO add this

HtRegex limit_to;
HtRegex exclude_these;
HtRegex always_return_these;

// List         searchWords;
//List *searchWords = NULL;


static int total_matches = 0;
//static List *matches_list = 0;
//static ResultFetch *resultfetch = 0;


//*****************************************************************************
// int main()
//
// int main(int ac, char **av)
DLLEXPORT int htsearch_open(htsearch_parameters_struct * params)
{
    //int override_config = 0;
    //int cInd = 0;
    //StringMatch *searchWordsPattern = NULL;

    String logicalWords;
    String logicalPattern;
    StringList requiredWords;

    debug = params->debug;


    if (params->logFile[0] != 0)
    {
        if (logOpen(params->logFile) == FALSE)
        {
            reportError(form("[HTDIG] Error opening log file [%s] . Error:[%d], %s\n", 
                        params->logFile, errno, strerror(errno)));
			return (HTSEARCH_ERROR_LOGFILE_OPEN);
		}
	}

	errorMsg = "";

	//
	// Setup the configuration database.
    //
	config = HtConfiguration::config();

    //
    // First we read the compiled defaults.
	//
	config->Defaults(&defaults[0]);

    //
    // Override defaults with configuration file
    // 
    if (params->configFile[0] != 0)
    {
        configFile = params->configFile;

        if (access((char *) configFile, R_OK) < 0)
        {
            reportError(form("Unable to read configuration file '%s'", configFile.get()));
            return (HTSEARCH_ERROR_CONFIG_READ);
        }
    }
    config->Read(configFile);

//    WordType::Initialize(*config);

    //
    //  ????????? this doesn't seem to do anything
    //
    // To allow . in filename while still being 'secure',
	// e.g. htdig-f.q.d.n.conf
    // 
    //  char *config_name = collectionList[cInd];
    //  if (config_name && config_name[0] == '\0')
    //      config_name = NULL;	  // use default config
    //
/*    if (!override_config && config_name && (strstr(config_name, "./") == NULL))
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
        if (strlen(config_name) == 0)
            configFile = DEFAULT_CONFIG_FILE;
        else
            configFile << '/' << config_name << ".conf";
    }
*/


	//
    // override config settings from parameters
    //
    if (strlen(params->DBpath) > 0)
        config->Add("database_dir", params->DBpath);

    if (strlen(params->search_restrict) > 0)
        config->Add("restrict", params->search_restrict);

    if (strlen(params->search_exclude) > 0)
        config->Add("exclude", params->search_exclude);

    if(strlen(params->locale) > 0)
        config->Add("locale", params->locale);


    minimum_word_length = config->Value("minimum_word_length", minimum_word_length);

    //  
    // Compile the URL limit patterns.
    //
    if (config->Find("restrict").length())
    {
        //
        // Create a temporary list from either the configuration
        // file or the input parameter
        //
        StringList l(config->Find("restrict"), " \t\r\n\001|");
        limit_to.setEscaped(l);
        String u = l.Join('|');
        config->Add("restrict", u);	// re-create the config attribute
    }

    if (config->Find("exclude").length())
    {
		//
        // Create a temporary list from either the configuration
        // file or the input parameter
        // 
        StringList l(config->Find("exclude"), " \t\r\n\001|");
        exclude_these.setEscaped(l);
        String u = l.Join('|');
        config->Add("exclude", u);	// re-create the config attribute
    }

    if (strlen(params->search_alwaysreturn) > 0)
    {
        StringList l(params->search_alwaysreturn, " \t\r\n\001|");
        always_return_these.setEscaped(l);
    }

	//
	// Check url_part_aliases and common_url_parts for errors.
    //
    String url_part_errors = HtURLCodec::instance()->ErrMsg();

    if (url_part_errors.length() != 0)
    {
        reportError(form("Invalid url_part_aliases or common_url_parts: %s", url_part_errors.get()));
        return (HTSEARCH_ERROR_URL_PART);
    }

	//
    // for htsearch, use search_rewrite_rules attribute for HtURLRewriter.
    //
    config->AddParsed("url_rewrite_rules", "${search_rewrite_rules}");
    url_part_errors = HtURLRewriter::instance()->ErrMsg();
    if (url_part_errors.length() != 0)
    {
        reportError(form("Invalid url_rewrite_rules: %s", url_part_errors.get()));
    }

	// Load boolean_keywords from configuration
	// they should be placed in this order:
	//    0       1       2
	//    and     or      not
	boolean_keywords.Create(config->Find("boolean_keywords"), "| \t\r\n\001");
	if (boolean_keywords.Count() != 3)
    {
		reportError("boolean_keywords attribute should have three entries");
    }

    //
    // Read in the badwords file (it's just a text file)
    //
/*
    const       String filename = config->Find("bad_word_list");
    FILE        *fl = fopen(filename, "r");
    char        buffer[1000];
    char        *word;
    String      new_word;

    while (fl && fgets(buffer, sizeof(buffer), fl))
    {
        word = strtok(buffer, "\r\n \t");
        if (word && *word)
        {
            int flags;
            new_word = word;
            if((flags = WordType::Instance()->Normalize(new_word)) & WORD_NORMALIZE_NOTOK)
            {
                fprintf(stderr, "Reading bad words from %s found %s, ignored because %s\n",
                        (const char*)filename, word, 
                        (char*)WordType::Instance()->NormalizeStatus(flags & WORD_NORMALIZE_NOTOK));
            }
            else
            {
               stopWords.insert(new_word.get());
            }
        }
    }
*/
    //
    // set up the stop word list
    //
    set<string> stopWords;
    string stopWordsFilename = (config->Find("bad_word_list")).get();
    if (stopWordsFilename.length())
    {
        ifstream infile(stopWordsFilename.c_str());
        if (infile.is_open())
        {
            if (debug > 4)
            {
                cout << "Stopwords from " << stopWordsFilename << ":" << endl;
            }
            char line[255];
            while (infile.good())
            {
                infile.getline(line, 254); // hopefully no stop words are this long
                while (line[strlen(line)-1] == '\n' ||
                       line[strlen(line)-1] == '\r' ||
                       line[strlen(line)-1] == ' ')
                {
                    line[strlen(line)-1] = '\0';
                }
                if (line[0] == '#' || strlen(line) == 0)
                {
                    continue;
                }
                else
                {
                    stopWords.insert(line);
                    if (debug > 4)
                    {
                        cout << '.' << line << '.' << endl;
                    }
                }
            }
        }
        else if (debug)
        {
            cout << "Unable to open stop word file" << endl;
        }
    }
    else if (debug > 1)
    {
        cout << "Stop word file not specified, using default CLucene stop words" << endl;
    }

    //
    // open the CLucene database 
    // 
    const String db_dir_filename = config->Find("database_dir");
    if (debug)
        cout << "Opening CLucene database here: " << db_dir_filename.get() << endl;
     
    CLuceneOpenIndex(form("%s/CLuceneDB", (char *)db_dir_filename.get()), 0, &stopWords);

	return (TRUE);
}

//---------------------------------------------------------------------------------------
//
//  RETURN:  Number of Documents resulted from search
//
//---------------------------------------------------------------------------------------

DLLEXPORT int htsearch_query(htsearch_query_struct * htsearch_query)
{
    string initial_query = htsearch_query->raw_query;
    string final_query;

    if (debug > 1)
    {
        cout << "Initial query = " << initial_query << endl;
    }
 

    //
    // result sorting options
    //
    switch (htsearch_query->sortby_flag)
    {
        case HTSEARCH_SORT_SCORE:
            config->Add("sort", "score");
            break;
        case HTSEARCH_SORT_REV_SCORE:
            config->Add("sort", "revscore");
            break;
        case HTSEARCH_SORT_TIME:
            config->Add("sort", "time");
            break;
        case HTSEARCH_SORT_REV_TIME:
            config->Add("sort", "revtime");
            break;
        case HTSEARCH_SORT_TITLE:
            config->Add("sort", "title");
            break;
        case HTSEARCH_SORT_REV_TITLE:
            config->Add("sort", "revtitle");
            break;
    }

    //
    // boolean operator
    //
    switch (htsearch_query->algorithms_flag)
    {
        case HTSEARCH_ALG_BOOLEAN:
            break;
        case HTSEARCH_ALG_OR:
            break;
        case HTSEARCH_ALG_AND:
            changeDefaultOperator();
            break;
    }

    //
    // format ??
    // 
    switch (htsearch_query->algorithms_flag)
    {
        case HTSEARCH_FORMAT_SHORT:
            config->Add("template_name", "builtin-short");
            break;
        case HTSEARCH_FORMAT_LONG:
            config->Add("template_name", "builtin-long");
            break;
    }

    if (requiredWords.Count() > 0)
	{
        // add the required words to the query

        
		//addRequiredWords(*searchWords, requiredWords);
	}

/*
 *
 * CLucene supports multiple databases, but
 * we'll worry about that at a later time
 *
 *
	// Multiple database support
	collection = new Collection((char *) configFile,
						   word_db.get(), doc_index.get(), doc_db.get(), doc_excerpt.get());

	// Perform search within the collection. Each collection stores its
	// own result list.
	match_count_acc += htsearch(collection, *searchWords, parser);
	collection->setSearchWords(searchWords);
	collection->setSearchWordsPattern(searchWordsPattern);
	selected_collections.Add(configFile, collection);
 *
 *
 *
 */
    final_query = initial_query;

    if (debug > 1)
    {
        cout << "Final query = " << final_query << endl;
    }

    total_matches = CLuceneDoQuery(&final_query);

    if (debug > 1)
    {
        cout << "CLucene found " << total_matches << " results" << endl;
    }

    return total_matches;
}

//------------------  htsearch_get_nth_match (...)  -------------------------------------
//
//  Parameters
//        n                 ZERO based results index.
//        query_result      structure to fill with result
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

DLLEXPORT int htsearch_get_nth_match(int n, htsearch_query_match_struct * query_result)
{
    if (n < total_matches)
    {
        CLuceneSearchGetNth(n, query_result);
        return (TRUE);
    }
    else
    {
        return (FALSE);
    }
}


DLLEXPORT int htsearch_close()
{
    //
    // Close the CLucene index
    //
    CLuceneCloseIndex();

    return (TRUE);
}


// *****************************************************************************
// Report an error.  Since we don' know if we are running as a CGI or not,
// we will assume this is the first thing returned by a CGI program.
//
void reportError_html(char *msg)
{
	HtConfiguration *config = HtConfiguration::config();
	cout << "Content-type: text/html\r\n\r\n";
	cout << "<html><head><title>htsearch error</title></head>\n";
	cout << "<body bgcolor=\"#ffffff\">\n";
	cout << "<h1>ht://Dig error</h1>\n";
	cout << "<p>htsearch detected an error.  Please report this to the\n";
	cout << "webmaster of this site by sending an e-mail to:\n";
	cout << "<a href=\"mailto:" << config->Find("maintainer") << "\">";
	cout << config->Find("maintainer") << "</a>\n";
	cout << "The error message is:</p>\n";
	cout << "<pre>\n" << msg << "\n</pre>\n</body></html>\n";
	exit(1);
}
