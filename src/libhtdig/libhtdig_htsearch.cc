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
// $Id: libhtdig_htsearch.cc,v 1.1.2.1 2006/09/25 23:50:49 aarnone Exp $
//
//----------------------------------------------------------------

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

extern "C" {
#include "libhtdig_api.h"
}

#include <time.h>
#include "HtStdHeader.h"

#include "CLucene.h"
#include "CLucene-contrib.h"

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

#include "defaults.h"
#include "cgi.h"
#include "StringList.h"
#include "IntObject.h"
#include "HtURLCodec.h"
#include "HtURLRewriter.h"
#include "HtRegex.h"
#include "WordType.h"
#include "HtDebug.h"

//
//  ????????
//
//typedef void (*SIGNAL_HANDLER) (...);

    
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


//
// CLucene static variables used throughout searching
//
static int64_t cl_time_str = lucene::util::Misc::currentTimeMillis();
static IndexSearcher* cl_searcher = NULL;
static PerFieldAnalyzerWrapper* cl_analyzer = NULL;
static StandardAnalyzer* standardAnalyzer = NULL;
static SnowballAnalyzer* snowballAnalyzer = NULL;
static Hits* cl_hits = NULL;
static QueryParser * cl_parser = NULL;


//
// more static variables (non-clucene) that are used across calls
//
static int htsearchapi_total_matches = 0;
static string htsearchapi_initial_query;



//*****************************************************************************
// int main()
//
// int main(int ac, char **av)
DLLEXPORT int htsearch_open(htsearch_parameters_struct * params)
{
    StringList requiredWords;
    StringList boolean_keywords;

    HtRegex limit_to;
    HtRegex exclude_these;
    HtRegex always_return_these;

    HtConfiguration *config = NULL;

    //
    // set up the debug log. any exit conditions other than true should close the debug log
    //
    HtDebug * htsearchapi_debug = HtDebug::Instance();
    htsearchapi_debug->setStdoutLevel(params->debug);
    if (strlen(params->debugFile) > 0)
    {
        //
        // if the debugFile was specified, then set it up. also, kill stdout output.
        //
        if (htsearchapi_debug->setLogfile(params->debugFile) == false)
        {
            cout << "HtSearch: Error opening log file ["<< params->debugFile << "]" << endl;
            cout << "Error:[" << errno << "], " << strerror(errno) << endl;
            return (HTSEARCH_ERROR_LOGFILE_OPEN);
        }
        htsearchapi_debug->setFileLevel(params->debug);
        htsearchapi_debug->setStdoutLevel(0);
    }

    if (cl_searcher != NULL)
    {
        htsearchapi_debug->outlog(2, "HtSearch: tried to open cl_searcher when already open\n");
        return(TRUE);
    }

    //int override_config = 0;
    //int cInd = 0;
    //StringMatch *searchWordsPattern = NULL;

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
        String configFile = params->configFile;

        if (access((char *) configFile, R_OK) < 0)
        {
            htsearchapi_debug->outlog(0, "Unable to read configuration file '%s'\n", configFile.get());
            htsearchapi_debug->close();
            return (HTSEARCH_ERROR_CONFIG_READ);
        }
        config->Read(configFile);
    }

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
    // now that the config has been read, the debug override must be checked
    //
    if (config->Value("debug_override") > 0)
    {
        if (strlen(params->debugFile) > 0)
        {
            htsearchapi_debug->setFileLevel(config->Value("debug_override"));
        }
        else
        {
            htsearchapi_debug->setStdoutLevel(config->Value("debug_override"));
        }
    }


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
        htsearchapi_debug->outlog(0, "Invalid url_part_aliases or common_url_parts: %s\n", url_part_errors.get());
        htsearchapi_debug->close();
        return (HTSEARCH_ERROR_URL_PART);
    }

	//
    // for htsearch, use search_rewrite_rules attribute for HtURLRewriter.
    //
    config->AddParsed("url_rewrite_rules", "${search_rewrite_rules}");
    url_part_errors = HtURLRewriter::instance()->ErrMsg();
    if (url_part_errors.length() != 0)
    {
        htsearchapi_debug->outlog(1, "Invalid url_rewrite_rules: %s\n", url_part_errors.get());
    }

	// Load boolean_keywords from configuration
	// they should be placed in this order:
	//    0       1       2
	//    and     or      not
	boolean_keywords.Create(config->Find("boolean_keywords"), "| \t\r\n\001");
	if (boolean_keywords.Count() != 3)
    {
        htsearchapi_debug->outlog(1, "boolean_keywords attribute should have three entries\n");
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
            htsearchapi_debug->outlog(4, "Stopwords from %s :\n", stopWordsFilename.c_str());

            while (infile.good())
            {
                char line[255];
                int lineLength;
                
                infile.getline(line, 254); // hopefully no stop words are this long

                lineLength = strlen(line);
                while (lineLength > 0 &&
                        (line[lineLength] == '\n' ||
                         line[lineLength] == '\r' ||
                         line[lineLength] == ' ')
                      )
                {
                    line[lineLength] = '\0';
                    lineLength--;
                }
                //
                // it might have been trimmed to length zero, or be a comment
                //
                if (lineLength == 0 || line[0] == '#')
                {
                    continue;
                }
                else
                {
                    stopWords.insert(line);

                    htsearchapi_debug->outlog(4, "%s\n", line);
                }
            }
        }
        else
        {
            htsearchapi_debug->outlog(0, "HtSearch: unable to open stop word file, using default CLucene stop words\n");
        }
    }
    else
    {
        htsearchapi_debug->outlog(1, "HtSearch: stop word file not specified, using default CLucene stop words\n");
    }

    //
    // Create the searcher - this will be used for.... searching!
    //
    const String db_dir_filename = config->Find("database_dir");
    htsearchapi_debug->outlog(0, "HtSearch: Opening CLucene database here: %s\n", db_dir_filename.get());
     
    if (IndexReader::indexExists(form("%s/CLuceneDB", (char *)db_dir_filename.get())))
    {
	    cl_searcher = _CLNEW IndexSearcher(form("%s/CLuceneDB", (char *)db_dir_filename.get()));
        htsearchapi_debug->outlog(3, "IndexSearcher... ");
    }
    else
    {
        htsearchapi_debug->outlog(1, "While creating IndexSearcher, IndexReader::indexExists() says index does not exist\n");
        return (HTSEARCH_ERROR_INDEX_NOT_FOUND);
    }

    //
    // Create the analyzer. the stop words will need to be
    // extracted into a wchar_t array (maybe).
    //
    if (stopWords.size())
    {
        wchar_t ** stopArray = convertStopWords(&stopWords);

        standardAnalyzer = _CLNEW StandardAnalyzer(stopArray);
        snowballAnalyzer = _CLNEW SnowballAnalyzer(_T("english"), stopArray);
        cl_analyzer = _CLNEW PerFieldAnalyzerWrapper(standardAnalyzer);
        cl_analyzer->addAnalyzer(_T("stemmed"), snowballAnalyzer);

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
        cl_analyzer = _CLNEW PerFieldAnalyzerWrapper(standardAnalyzer);
        cl_analyzer->addAnalyzer(_T("stemmed"), snowballAnalyzer);
    }

    //
    // Create the query parser.. this needs to be declared
    // beforehand so we can set options for it
    //
    cl_parser = _CLNEW QueryParser(_T("contents"), cl_analyzer);
    htsearchapi_debug->outlog(3, "QueryParser... ");

    //
    // get the start time... useful for debugging
    // 
    cl_time_str = lucene::util::Misc::currentTimeMillis();

    htsearchapi_debug->outlog(3, "created.\n");

	return (TRUE);
}

//---------------------------------------------------------------------------------------
//
//  RETURN:  Number of Documents resulted from search
//
//---------------------------------------------------------------------------------------

DLLEXPORT int htsearch_query(htsearch_query_struct * htsearch_query)
{
    HtDebug * htsearchapi_debug = HtDebug::Instance();
	HtConfiguration * config = HtConfiguration::config();

    if (cl_searcher == NULL)
    {
        htsearchapi_debug->outlog(2, "HtSearch: tried to perform search before htsearch_open\n");
        htsearchapi_debug->close();
        return(FALSE);
    }

    if (cl_hits != NULL)
    {
        _CLDELETE(cl_hits);
        cl_hits = NULL;
    }

    htsearchapi_initial_query = htsearch_query->raw_query;
    string final_query;

    htsearchapi_debug->outlog(2, "Initial query: %s\n", htsearchapi_initial_query.c_str());
 

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
            cl_parser->setOperator(QueryParser::OR_OPERATOR);
            break;
        case HTSEARCH_ALG_AND:
            cl_parser->setOperator(QueryParser::AND_OPERATOR);
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

//    if (requiredWords.Count() > 0)
//	{
//        // add the required words to the query
//
//        
//		//addRequiredWords(*searchWords, requiredWords);
//	}

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
    final_query += " contents:(" + htsearchapi_initial_query + ")^" + factorString;

    sprintf(factorString, "%f", config->Double("keyword_factor"));
    final_query += " keywords:(" + htsearchapi_initial_query + ")^" + factorString;

    sprintf(factorString, "%f", config->Double("heading_factor"));
    final_query += " doc-id:(" + htsearchapi_initial_query + ")^" + factorString;

    sprintf(factorString, "%f", config->Double("title_factor"));
    sprintf(factorString, "%f", config->Double("heading_factor"));
    final_query += " heading:(" + htsearchapi_initial_query + ")^" + factorString;

    sprintf(factorString, "%f", config->Double("title_factor"));
    final_query += " title:(" + htsearchapi_initial_query + ")^" + factorString;

    sprintf(factorString, "%f", config->Double("meta_description_factor"));
    final_query += " meta-desc:(" + htsearchapi_initial_query + ")^" + factorString;

    sprintf(factorString, "%f", config->Double("author_factor"));
    final_query += " author:(" + htsearchapi_initial_query + ")^" + factorString;

    final_query += " url:(" + htsearchapi_initial_query + ")"; // no factor for URLs


    if (config->Boolean("use_stemming"))
    {
        sprintf(factorString, "%f", config->Double("stemming_factor"));
        final_query += " stemmed:(" + htsearchapi_initial_query + ")^" + factorString;
    }
    if (config->Boolean("use_synonyms"))
    {
        sprintf(factorString, "%f", config->Double("synonym_factor"));
        final_query += " synonym:(" + htsearchapi_initial_query + ")^" + factorString;
    }


    htsearchapi_debug->outlog(2, "Final query: %s\n", final_query.c_str());

    wchar_t * temp = utf8_to_wchar(final_query.c_str());
    //wchar_t * temp = utf8_to_wchar(htsearchapi_initial_query.c_str());
    Query * query = cl_parser->parse(temp);
    free(temp);

    if (htsearchapi_debug->getLevel() > 3)
    {
        wchar_t * converted_query = query->toString(_T("contents"));
        char * converted_query_utf8 = wchar_to_utf8(converted_query);
        htsearchapi_debug->outlog(3, "Converted query before searching: %s\n", converted_query_utf8);
        free(converted_query);
        free(converted_query_utf8);
    }

    cl_hits = cl_searcher->search( query );
    htsearchapi_total_matches = cl_hits->length();

    htsearchapi_debug->outlog(1, "CLucene found %d results\n", htsearchapi_total_matches);

    return htsearchapi_total_matches;
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
    HtDebug * htsearchapi_debug = HtDebug::Instance();
    if (cl_searcher == NULL)
    {
        htsearchapi_debug->outlog (2, "HtSearch: tried to get hit before htsearch_open\n");
        htsearchapi_debug->close();
        return (FALSE);
    }
    else if (cl_hits == NULL)
    {
        htsearchapi_debug->outlog (2, "HtSearch: tried to get match without doing search first\n");
        htsearchapi_debug->close();
        return (FALSE);
    }
    else if (n > htsearchapi_total_matches)
    {
        htsearchapi_debug->close();
        return (FALSE);
    }
    else
    {
        lucene::document::Document doc = cl_hits->doc(n);
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
        wchar_t * wtemp = utf8_to_wchar(htsearchapi_initial_query.c_str());
        Query * query = cl_parser->parse(wtemp);
        free(wtemp);

        QueryScorer * scorer = _CLNEW QueryScorer(query);
        Highlighter * highlighter = _CLNEW Highlighter(scorer);
        wtemp = highlighter->getBestFragment(cl_analyzer, _T("contents"), doc.get(_T("contents")));

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


        hit_struct->score = (int)(cl_hits->score(n) * 10000);
        hit_struct->score_percent = hit_struct->score / 100;

        return (TRUE);
    }
}


DLLEXPORT int htsearch_close()
{
    HtDebug * htsearchapi_debug = HtDebug::Instance();

    if (cl_searcher == NULL)
    {
        return (TRUE);
    }

    //
    // Delete the CLucene objects
    //
    _CLDELETE(cl_searcher);
    _CLDELETE(cl_analyzer);
    _CLDELETE(cl_parser);
    //_CLDELETE(cl_hits);

    cl_searcher = NULL;
    cl_analyzer = NULL;
    cl_parser = NULL;
    cl_hits = NULL;

    htsearchapi_debug->outlog(1, "Searching took: %d ms.\n", (lucene::util::Misc::currentTimeMillis() - cl_time_str));
    htsearchapi_debug->close();

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
