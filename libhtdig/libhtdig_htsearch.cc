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
// $Id: libhtdig_htsearch.cc,v 1.5.2.3 2006/09/25 22:26:48 aarnone Exp $
//
//----------------------------------------------------------------

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <time.h>
#include "HtStdHeader.h"

#include "CLucene.h"

extern "C"
{
#include "libhtdig_api.h"
}
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string>
#include <ctype.h>
#include <signal.h>

#ifndef _WIN32
#include <unistd.h>
#endif


// If we have this, we probably want it.
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

using namespace std;

#include "WordType.h"

#include "libhtdig_log.h"

#include "defaults.h"
#include "cgi.h"
#include "StringList.h"
#include "IntObject.h"
#include "HtURLCodec.h"
#include "HtURLRewriter.h"
#include "HtRegex.h"
#include "HtDebug.h"

typedef void (*SIGNAL_HANDLER) (...);


int minimum_word_length = 3;

StringList boolean_keywords;


extern String configFile;

static HtConfiguration *config = NULL;
static HtDebug *debug = NULL;
//Dictionary selected_collections;	// Multiple database support
//Collection *collection = NULL;

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
static string initial_query;

    
CL_NS_USE(index)
CL_NS_USE(util)
CL_NS_USE(store)
CL_NS_USE(search)
#ifndef _MSC_VER
CL_NS_USE2(search, highlight)
#endif
CL_NS_USE(document)
CL_NS_USE(queryParser)
CL_NS_USE(analysis)
CL_NS_USE2(analysis,standard)
CL_NS_USE2(analysis,snowball)

static int64_t str = lucene::util::Misc::currentTimeMillis();
static IndexSearcher* searcher = NULL;
static PerFieldAnalyzerWrapper* analyzer = NULL;
static StandardAnalyzer* standardAnalyzer = NULL;
static SnowballAnalyzer* snowballAnalyzer = NULL;
static Hits* hits = NULL;
static QueryParser * parser = NULL;



//*****************************************************************************
// int main()
//
// int main(int ac, char **av)
DLLEXPORT int htsearch_open(htsearch_parameters_struct * params)
{
    //
    // set up the debug log. any exit conditions other than true should close the debug log
    //
    debug = HtDebug::Instance();
    debug->setStdoutLevel(params->debug);
    if (strlen(params->logFile) > 0)
    {
        //
        // if the logFile was specified, then set it up. also, kill stdout output.
        //
        if (debug->setLogfile(params->logFile) == false)
        {
            cout << "HtSearch: Error opening log file ["<< params->logFile << "]" << endl;
            cout << "Error:[" << errno << "], " << strerror(errno) << endl;
            return (HTSEARCH_ERROR_LOGFILE_OPEN);
        }
        debug->setFileLevel(params->debug);
        debug->setStdoutLevel(0);
    }

    if (searcher != NULL)
    {
        debug->outlog(2, "HtSearch: tried to open searcher when already open\n");
        return(TRUE);
    }

    //int override_config = 0;
    //int cInd = 0;
    //StringMatch *searchWordsPattern = NULL;

    String logicalWords;
    String logicalPattern;
    StringList requiredWords;

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
            debug->outlog(0, "Unable to read configuration file '%s'\n", configFile.get());
            debug->close();
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
        debug->close();
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
    // set up the stop word list
    //
    set<string> stopWords;
    string stopWordsFilename = (config->Find("bad_word_list")).get();
    if (stopWordsFilename.length())
    {
        ifstream infile(stopWordsFilename.c_str());
        if (infile.is_open())
        {
            debug->outlog(4, "Stopwords from %s :\n", stopWordsFilename.c_str());

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

                    debug->outlog(4, "%s\n", line);
                }
            }
        }
        else
        {
            debug->outlog(0, "HtSearch: unable to open stop word file, using default CLucene stop words\n");
        }
    }
    else
    {
        debug->outlog(1, "HtSearch: stop word file not specified, using default CLucene stop words\n");
    }

    //
    // Create the searcher - this will be used for.... searching!
    //
    const String db_dir_filename = config->Find("database_dir");
    debug->outlog(0, "HtSearch: Opening CLucene database here: %s\n", db_dir_filename.get());
     
	searcher = _CLNEW IndexSearcher(form("%s/CLuceneDB", (char *)db_dir_filename.get()));
    debug->outlog(3, "IndexSearcher... ");

    //
    // Create the analyzer. the stop words will need to be
    // extracted into a wchar_t array (maybe).
    //
    if (stopWords.size())
    {
        wchar_t ** stopArray = convertStopWords(&stopWords);

        standardAnalyzer = _CLNEW StandardAnalyzer(stopArray);
        snowballAnalyzer = _CLNEW SnowballAnalyzer(_T("english"), stopArray);
        analyzer = _CLNEW PerFieldAnalyzerWrapper(standardAnalyzer);
        analyzer->addAnalyzer(_T("stemmed"), snowballAnalyzer);

        for (unsigned int i = 0; i<stopWords.size(); i++)
        {
            free(stopArray[i]);
        }
        free(stopArray);
    }
    else
    {
        standardAnalyzer = _CLNEW StandardAnalyzer();
        snowballAnalyzer = _CLNEW SnowballAnalyzer(_T("english"));
        analyzer = _CLNEW PerFieldAnalyzerWrapper(standardAnalyzer);
        analyzer->addAnalyzer(_T("stemmed"), snowballAnalyzer);
    }

    //
    // Create the query parser.. this needs to be declared
    // beforehand so we can set options for it
    //
    parser = _CLNEW QueryParser(_T("contents"), analyzer);
    debug->outlog(3, "QueryParser... ");

    //
    // get the start time... useful for debugging
    // 
    str = lucene::util::Misc::currentTimeMillis();

    debug->outlog(3, "created.\n");

	return (TRUE);
}

//---------------------------------------------------------------------------------------
//
//  RETURN:  Number of Documents resulted from search
//
//---------------------------------------------------------------------------------------

DLLEXPORT int htsearch_query(htsearch_query_struct * htsearch_query)
{
    debug = HtDebug::Instance();
    if (searcher == NULL)
    {
        debug->outlog(2, "HtSearch: tried to perform search before htsearch_open\n");
        debug->close();
        return(FALSE);
    }

    if (hits != NULL)
    {
        _CLDELETE(hits);
        hits = NULL;
    }

    initial_query = htsearch_query->raw_query;
    string final_query;

    debug->outlog(2, "Initial query: %s\n", initial_query.c_str());
 

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
            parser->setOperator(QueryParser::OR_OPERATOR);
            break;
        case HTSEARCH_ALG_AND:
            parser->setOperator(QueryParser::AND_OPERATOR);
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
    char factorString[32];

    sprintf(factorString, "%f", config->Double("text_factor"));
    final_query += " contents:(" + initial_query + ")^" + factorString;

    sprintf(factorString, "%f", config->Double("keyword_factor"));
    final_query += " keywords:(" + initial_query + ")^" + factorString;

    sprintf(factorString, "%f", config->Double("heading_factor"));
    final_query += " doc-id:(" + initial_query + ")^" + factorString;

    sprintf(factorString, "%f", config->Double("title_factor"));
    sprintf(factorString, "%f", config->Double("heading_factor"));
    final_query += " heading:(" + initial_query + ")^" + factorString;

    sprintf(factorString, "%f", config->Double("title_factor"));
    final_query += " title:(" + initial_query + ")^" + factorString;

    sprintf(factorString, "%f", config->Double("meta_description_factor"));
    final_query += " meta-desc:(" + initial_query + ")^" + factorString;

    sprintf(factorString, "%f", config->Double("author_factor"));
    final_query += " author:(" + initial_query + ")^" + factorString;

    final_query += " url:(" + initial_query + ")"; // no factor for URLs


    if (config->Boolean("use_stemming"))
    {
        sprintf(factorString, "%f", config->Double("stemming_factor"));
        final_query += " stemmed:(" + initial_query + ")^" + factorString;
    }
    if (config->Boolean("use_synonyms"))
    {
        sprintf(factorString, "%f", config->Double("synonym_factor"));
        final_query += " synonym:(" + initial_query + ")^" + factorString;
    }


    debug->outlog(2, "Final query: %s\n", final_query.c_str());

    wchar_t * temp = utf8_to_wchar(final_query.c_str());
    //wchar_t * temp = utf8_to_wchar(initial_query.c_str());
    Query * query = parser->parse(temp);
    free(temp);

    if (debug->getLevel() > 3)
    {
        wchar_t * converted_query = query->toString(_T("contents"));
        char * converted_query_utf8 = wchar_to_utf8(converted_query);
        debug->outlog(3, "Converted query before searching: %s\n", converted_query_utf8);
        free(converted_query);
        free(converted_query_utf8);
    }

    hits = searcher->search( query );
    total_matches = hits->length();

    debug->outlog(1, "CLucene found %d results\n", total_matches);

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

DLLEXPORT int htsearch_get_nth_match(int n, htsearch_query_match_struct * hit_struct)
{
    debug = HtDebug::Instance();
    if (searcher == NULL)
    {
        debug->outlog (2, "HtSearch: tried to get hit before htsearch_open\n");
        debug->close();
        return (FALSE);
    }
    else if (hits == NULL)
    {
        debug->outlog (2, "HtSearch: tried to get match without doing search first\n");
        debug->close();
        return (FALSE);
    }
    else if (n > total_matches)
    {
        debug->close();
        return (FALSE);
    }
    else
    {
        lucene::document::Document doc = hits->doc(n);
        char * temp;

        //
        // clear the strings in the match structure
        //
        hit_struct->URL[0] = '\0';
        hit_struct->title[0] = '\0';
        hit_struct->excerpt[0] = '\0';


        temp = wchar_to_utf8(doc.get(_T("url")));
        strncpy (hit_struct->URL, temp, HTDIG_MAX_FILENAME_PATH_L);
        hit_struct->URL[HTDIG_MAX_FILENAME_PATH_L - 1] = '\0';
        free(temp);


        temp = wchar_to_utf8(doc.get(_T("doc-title")));
        //
        // if there is no title, then just use the URL
        //
        if (strlen(temp) > 0)
        {
            strncpy (hit_struct->title, temp, HTDIG_DOCUMENT_TITLE_L);
        }
        else
        {
            strncpy (hit_struct->title, hit_struct->URL, HTDIG_DOCUMENT_TITLE_L);
        }
        hit_struct->title[HTDIG_DOCUMENT_TITLE_L - 1] = '\0';
        free(temp);

        
        //
        // use the intial query for highlighting, because it hasn't been
        // changed from its initial form. this is broken on windows
        //
#ifndef _MSC_VER
        wchar_t * wtemp = utf8_to_wchar(initial_query.c_str());
        Query * query = parser->parse(wtemp);
        free(wtemp);

        QueryScorer * scorer = _CLNEW QueryScorer(query);
        Highlighter * highlighter = _CLNEW Highlighter(scorer);
        wtemp = highlighter->getBestFragment(analyzer, _T("contents"), doc.get(_T("contents")));

        if (wtemp)
        {
            temp = wchar_to_utf8(wtemp);
            free(wtemp);
        }
        else
        {
#endif
            //
            // highlighter couldn't find anything... just use the top of the contents
            //
            temp = wchar_to_utf8(doc.get(_T("contents")));
#ifndef _MSC_VER
        }

        _CLDELETE(scorer);
        _CLDELETE(highlighter);
#endif
        strncpy (hit_struct->excerpt, temp, HTDIG_DOCUMENT_EXCERPT_L);
        hit_struct->excerpt[HTDIG_DOCUMENT_EXCERPT_L - 1] = '\0';
        free(temp);


        temp = wchar_to_utf8(doc.get(_T("doc-size")));
        hit_struct->size = atoi(temp);
        free(temp);


        temp = wchar_to_utf8(doc.get(_T("doc-id")));
        hit_struct->id = atoi(temp);
        free(temp);

        
        temp = wchar_to_utf8(doc.get(_T("doc-time")));
        time_t tempTime = (time_t)atoi(temp);
        tm * ptm = gmtime(&tempTime);
        memcpy(&(hit_struct->time_tm), ptm, sizeof(tm));
        free(temp);


        hit_struct->score = (int)(hits->score(n) * 10000);
        hit_struct->score_percent = hit_struct->score / 100;

        return (TRUE);
    }
}


DLLEXPORT int htsearch_close()
{
    if (searcher == NULL)
    {
        return (TRUE);
    }

    //
    // Delete the CLucene objects
    //
    _CLDELETE(searcher);
    _CLDELETE(analyzer);
    _CLDELETE(parser);
    //_CLDELETE(hits);

    searcher = NULL;
    analyzer = NULL;
    parser = NULL;
    hits = NULL;

    debug->outlog(1, "Searching took: %d ms.\n", (lucene::util::Misc::currentTimeMillis() - str));
    debug->close();

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
