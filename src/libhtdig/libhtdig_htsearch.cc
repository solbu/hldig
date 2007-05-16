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
// $Id: libhtdig_htsearch.cc,v 1.1.2.4 2007/05/16 18:11:37 aarnone Exp $
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
CL_NS_USE2(search, highlight)
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
static Hits* cl_hits = NULL;
static int* hit_map = NULL;

//
// more static variables (non-clucene) that are used across calls
//
static wstring htsearchapi_highlight_text;    // used in highlighting
static int htsearchapi_total_matches = 0;



//*****************************************************************************
// int main()
//
// int main(int ac, char **av)
DLLEXPORT int htsearch_open(htsearch_parameters_struct * params)
{
    try
    {
        //StringList requiredWords;
        //StringList boolean_keywords;

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

        //
        // WordType::Initialize(*config);
        //

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

        if (strlen(params->search_alwaysreturn) > 0)
            config->Add("alwaysreturn", params->search_alwaysreturn);

        if(strlen(params->locale) > 0)
            config->Add("locale", params->locale);

        if (strlen(params->text_factor) > 0)
            config->Add ("text_factor", params->text_factor);

        if (strlen(params->title_factor) > 0)
            config->Add ("title_factor", params->title_factor);

        if (strlen(params->meta_description_factor) > 0)
            config->Add ("meta_description_factor", params->meta_description_factor);

        if (strlen(params->keyword_factor) > 0)
            config->Add ("keywords_factor", params->keyword_factor);

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
        //boolean_keywords.Create(config->Find("boolean_keywords"), "| \t\r\n\001");
        //if (boolean_keywords.Count() != 3)
        //{
        //    htsearchapi_debug->outlog(1, "boolean_keywords attribute should have three entries\n");
        //}

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
                htsearchapi_debug->outlog(4, "Using stopword file [%s]\n", stopWordsFilename.c_str());

                htsearchapi_debug->outlog(6, "Stopwords: ");
                while (infile.good())
                {
                    char line[255];
                    int lineLength;
                    infile.getline(line, 254); // hopefully no stop words are this long

                    lineLength = strlen(line);
                    while (lineLength > 0 &&
                            (line[lineLength - 1] == '\n' ||
                             line[lineLength - 1] == '\r' ||
                             line[lineLength - 1] == ' ')
                          )
                    {
                        line[lineLength - 1] = '\0';
                        lineLength--;
                    }
                    //
                    // it might have been trimmed to zero length, or possibly a comment
                    //
                    if (lineLength <= 0 || line[0] == '#')
                    {
                        continue;
                    }
                    else
                    {
                        stopWords.insert(line);
                        htsearchapi_debug->outlog(6, "%s ", line);
                    }
                }
                htsearchapi_debug->outlog(6, "\n");
            }
            else 
            {
                htsearchapi_debug->outlog(0, "Unable to open stop word file\n");
            }
        }
        else
        {
            htsearchapi_debug->outlog(1, "Stop word file not specified, using default CLucene stop words\n");
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
            htsearchapi_debug->outlog(0, "While creating IndexSearcher, IndexReader::indexExists() says index does not exist\n");
            return (HTSEARCH_ERROR_INDEX_NOT_FOUND);
        }

        //
        // Create the analyzer. the stop words will need to be extracted into a wchar_t array.
        //
        if (stopWords.size())
        {
            wchar_t ** stopArray = convertStopWords(&stopWords);

            cl_analyzer = _CLNEW PerFieldAnalyzerWrapper(_CLNEW StandardAnalyzer(stopArray));
            cl_analyzer->addAnalyzer(_T("stemmed"), _CLNEW SnowballAnalyzer(_T("english"), stopArray));

            for (unsigned int i = 0; i<stopWords.size(); i++)
            {
                free(stopArray[i]);
            }
            free(stopArray);
        }
        else
        {
            cl_analyzer = _CLNEW PerFieldAnalyzerWrapper(_CLNEW StandardAnalyzer());
            cl_analyzer->addAnalyzer(_T("stemmed"), _CLNEW SnowballAnalyzer(_T("english")));
        }
        //
        // add analyzers for special fields
        //
        cl_analyzer->addAnalyzer(_T("url"), _CLNEW WhitespaceAnalyzer());
        cl_analyzer->addAnalyzer(_T("id"), _CLNEW WhitespaceAnalyzer());
        cl_analyzer->addAnalyzer(_T("author"), _CLNEW WhitespaceAnalyzer());

        //
        // get the start time... useful for debugging
        // 
        cl_time_str = lucene::util::Misc::currentTimeMillis();

        htsearchapi_debug->outlog(3, "created.\n");

        return (TRUE);
    }
    catch (CLuceneError& e)
    {
        HtDebug * debug = HtDebug::Instance();
        debug->outlog(1, "\nException in libhtdig_htsearch (htsearch_query): [%s]\n", e.what());
        debug->close();
        return (HTSEARCH_ERROR_OPEN_ERROR);
    }
}

//---------------------------------------------------------------------------------------
//
//  RETURN:  Number of Documents resulted from search
//
//---------------------------------------------------------------------------------------

DLLEXPORT int htsearch_query(htsearch_query_struct * htsearch_query)
{
    try
    {
        HtDebug * htsearchapi_debug = HtDebug::Instance();
        HtConfiguration * config = HtConfiguration::config();
        //bool useAndOperator = false;

        BooleanQuery * final_query = _CLNEW BooleanQuery();

        Sort * sorter;

        htsearchapi_debug->outlog(3, "HtSearch: htsearch_query entered\n");

        wchar_t * wtemp;

        StringReader* optional_query = NULL;
        StringReader* required_query = NULL;
        StringReader* forbidden_query = NULL;
        StringReader* prefix_query = NULL;
        StringReader* synonym_query = NULL;

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

        if (hit_map != NULL)
        {
            free(hit_map);
            hit_map = NULL;
        }

        htsearchapi_total_matches = 0;
        htsearchapi_highlight_text.clear();

        //if (requiredWords.Count() > 0)
        //{
        //    // add the required words to the query
        //}

        if (strlen(htsearch_query->optional_query) > 0)
        {
            htsearchapi_debug->outlog(4, "HtSearch: optional query: [%s]\n", htsearch_query->optional_query);
            wtemp = utf8_to_wchar(htsearch_query->optional_query);
            optional_query = _CLNEW StringReader(wtemp);

            htsearchapi_highlight_text +=  wtemp; // candidate for highlighting
            htsearchapi_highlight_text += _T(" ");

            free(wtemp);
        } 

        if (strlen(htsearch_query->required_query) > 0)
        {
            htsearchapi_debug->outlog(4, "HtSearch: required query: [%s]\n", htsearch_query->required_query);
            wtemp = utf8_to_wchar(htsearch_query->required_query);
            required_query = _CLNEW StringReader(wtemp);

            htsearchapi_highlight_text +=  wtemp; // candidate for highlighting
            htsearchapi_highlight_text += _T(" ");

            free(wtemp);
        }

        if (strlen(htsearch_query->forbidden_query) > 0)
        {
            htsearchapi_debug->outlog(4, "HtSearch: forbidden query: [%s]\n", htsearch_query->forbidden_query);
            wtemp = utf8_to_wchar(htsearch_query->forbidden_query);
            forbidden_query = _CLNEW StringReader(wtemp);
            free(wtemp);
        }

        if (strlen(htsearch_query->prefix_query) > 0)
        {
            htsearchapi_debug->outlog(4, "HtSearch: prefix query: [%s]\n", htsearch_query->prefix_query);
            wtemp = utf8_to_wchar(htsearch_query->prefix_query);
            prefix_query = _CLNEW StringReader(wtemp);
            free(wtemp);
        }

        if (strlen(htsearch_query->synonym_query) > 0)
        {
            htsearchapi_debug->outlog(4, "HtSearch: synonym query: [%s]\n", htsearch_query->synonym_query);
            wtemp = utf8_to_wchar(htsearch_query->synonym_query);
            synonym_query = _CLNEW StringReader(wtemp);
            free(wtemp);
        }

        //StringList optional_query(htsearch_query->optional_query, " \t\r\n\001|");
        //StringList required_query(htsearch_query->required_query, " \t\r\n\001|");
        //StringList forbidden_query(htsearch_query->forbidden_query, " \t\r\n\001|");
        //StringList prefix_query(htsearch_query->prefix_query, " \t\r\n\001|");
        //StringList synonym_query(htsearch_query->synonym_query, " \t\r\n\001|");

 
        //
        // boolean operator. since CLucene supports complex query types by
        // default, HTSEARCH_ALG_BOOLEAN is redundant, or at least identical
        // to using the default, namely HTSEARCH_ALG_OR
        //
        /*
         * With the new search query fields, all searches are essentially complex
         *
        switch (htsearch_query->algorithms_flag)
        {
            case HTSEARCH_ALG_BOOLEAN:
                htsearchapi_debug->outlog(3, "Boolean query type specified - doing nothing.\n");
                break;
            case HTSEARCH_ALG_OR:
                htsearchapi_debug->outlog(3, "OR query type specified - doing nothing.\n");
                break;
            case HTSEARCH_ALG_AND:
                useAndOperator = true;
                htsearchapi_debug->outlog(3, "AND query type specified - will change default QueryParser operator.\n");
                break;
        }*/

        //wchar_t* wideQuery = utf8_to_wchar(htsearchapi_initial_query.c_str());

        //
        // fieldInfo pairs are comprised of field name and corresponding field boost weight
        //
        vector<pair<wstring, double> > fieldInfo;
        vector<pair<wstring, double> >::iterator fieldInfoIterator;

        fieldInfo.push_back( pair<wstring, double>(wstring(_T("contents")), config->Double("text_factor")) );
        fieldInfo.push_back( pair<wstring, double>(wstring(_T("title")), config->Double("title_factor")) );
        fieldInfo.push_back( pair<wstring, double>(wstring(_T("meta-desc")), config->Double("meta_description_factor")) );
        fieldInfo.push_back( pair<wstring, double>(wstring(_T("keywords")), config->Double("keywords_factor")) );
        fieldInfo.push_back( pair<wstring, double>(wstring(_T("backlink")), config->Double("backlink_factor")) );

        fieldInfo.push_back( pair<wstring, double>(wstring(_T("heading")), config->Double("heading_factor")) );
        fieldInfo.push_back( pair<wstring, double>(wstring(_T("author")), config->Double("author_factor")) );

        fieldInfo.push_back( pair<wstring, double>(wstring(_T("url")), 1.0) );     // no boost for contents
        fieldInfo.push_back( pair<wstring, double>(wstring(_T("id")), 10.0) );     // boost for id should be high

        if (config->Boolean("use_stemming"))
        {
            fieldInfo.push_back( pair<wstring, double>(wstring(_T("stemmed")), config->Double("stemming_factor")) );
        }

        /*
         * eventually make this the field searched on for synonyms
         *
        if (config->Boolean("use_synonyms"))
        {
            fieldInfo.push_back( pair<wstring, double>(wstring(_T("synonym")), config->Double("synonym_factor")) );
        }
        */

        //
        // optional_query part
        //
        htsearchapi_debug->outlog(3, "Assembling optional query...");
        if (optional_query != NULL)
        {
            for (fieldInfoIterator = fieldInfo.begin(); fieldInfoIterator != fieldInfo.end(); fieldInfoIterator++)
            {
                TokenStream * optional_tokens;
                Token optional_token;

                optional_query->reset(0);
                optional_tokens = cl_analyzer->tokenStream((fieldInfoIterator->first).c_str(), optional_query);

                while (optional_tokens->next(&optional_token) != false)
                {
                    TermQuery* optionalQuery;

                    optionalQuery = _CLNEW TermQuery(_CLNEW Term((fieldInfoIterator->first).c_str(), optional_token.termText()));
                    optionalQuery->setBoost(fieldInfoIterator->second);

                    final_query->add(optionalQuery, false, false); //optional
                }
            }
        }
        htsearchapi_debug->outlog(3, " done\n");


        //
        // required_query part
        //
        // this needs to be handled a bit differently then the other queries. what should happen is when
        // a required query looks like
        // 
        //   A B C
        //
        // it should be translated into
        // 
        //   +(field1:A field2:A ... )
        //   +(field1:B field2:B ... )
        //   +(field1:C field2:C ... )
        // 
        // However, A, B and C should all be analyzed differently based upon what field they're being
        // looked for in. So really, the final query should look like
        //
        //   +(field1:(analyzer(field1, A)) field2:(analyzer(field2, A)) ... )
        //   +(field1:(analyzer(field1, B)) field2:(analyzer(field2, B)) ... )
        //   +(field1:(analyzer(field1, C)) field2:(analyzer(field2, C)) ... )
        //
        // This will require splitting the whole query on whitespace (using a whitspace analyzer), then
        // using each of the resultant terms in the token stream as individual queries that can be
        // analyzed on a per-field basis.
        //
        htsearchapi_debug->outlog(3, "Assembling required query...");
        if (required_query != NULL)
        {
            TokenStream * required_tokens;
            Token required_token;
            WhitespaceAnalyzer whitespaceAnalyzer;

            required_tokens = whitespaceAnalyzer.tokenStream(_T("doesn't matter"), required_query);
            
            while (required_tokens->next(&required_token) != false)
            {
                BooleanQuery * requiredTerm = _CLNEW BooleanQuery();

                for (fieldInfoIterator = fieldInfo.begin(); fieldInfoIterator != fieldInfo.end(); fieldInfoIterator++)
                {
                    StringReader required_reader(required_token.termText());
                    TokenStream * analyzed_tokens;
                    Token analyzed_token;

                    analyzed_tokens = cl_analyzer->tokenStream((fieldInfoIterator->first).c_str(), &required_reader);

                    //
                    // while there should only be one token, it never hurts to make sure
                    //
                    while (analyzed_tokens->next(&analyzed_token) != false)
                    {
                        TermQuery* requiredQuery;

                        requiredQuery = _CLNEW TermQuery(_CLNEW Term((fieldInfoIterator->first).c_str(), analyzed_token.termText()));
                        requiredQuery->setBoost(fieldInfoIterator->second);

                        requiredTerm->add(requiredQuery, false, false); // optional Field(i):Term(i)
                    }
                }
                final_query->add(requiredTerm, true, false); // required Field(1):Term(i) OR ... Field(n):Term(i)
            }
        }
        htsearchapi_debug->outlog(3, " done\n");


        //
        // forbidden_query part
        //
        htsearchapi_debug->outlog(3, "Assembling forbidden query...");
        if (forbidden_query != NULL)
        {
            for (fieldInfoIterator = fieldInfo.begin(); fieldInfoIterator != fieldInfo.end(); fieldInfoIterator++)
            {
                TokenStream * forbidden_tokens;
                Token forbidden_token;

                forbidden_query->reset(0);
                forbidden_tokens = cl_analyzer->tokenStream((fieldInfoIterator->first).c_str(), forbidden_query);

                while (forbidden_tokens->next(&forbidden_token) != false)
                {
                    TermQuery* forbiddenQuery;

                    forbiddenQuery = _CLNEW TermQuery(_CLNEW Term((fieldInfoIterator->first).c_str(), forbidden_token.termText()));
                    //forbiddenQuery->setBoost(fieldInfoIterator->second); // not neccessary

                    final_query->add(forbiddenQuery, false, true); // verboten
                }
            }
        }
        else
        {
            htsearchapi_debug->outlog(3, " none specified...");
        }
        htsearchapi_debug->outlog(3, " done\n");


        //
        // prefix query part
        //
        htsearchapi_debug->outlog(3, "Assembling prefix query...");
        if (prefix_query != NULL)
        {
            for (fieldInfoIterator = fieldInfo.begin(); fieldInfoIterator != fieldInfo.end(); fieldInfoIterator++)
            {
                TokenStream * prefix_tokens;
                Token prefix_token;

                prefix_query->reset(0);
                prefix_tokens = cl_analyzer->tokenStream((fieldInfoIterator->first).c_str(), prefix_query);

                while (prefix_tokens->next(&prefix_token) != false)
                {
                    PrefixQuery* prefixQuery;

                    prefixQuery = _CLNEW PrefixQuery(_CLNEW Term((fieldInfoIterator->first).c_str(), prefix_token.termText()));
                    prefixQuery->setBoost(fieldInfoIterator->second);

                    final_query->add(prefixQuery, false, false); // optional
                }
            }
        }
        htsearchapi_debug->outlog(3, " done\n");


        //
        // synonym_query part - just search for synonyms in every field (optional)
        //
        htsearchapi_debug->outlog(3, "Assembling synonym query...");
        if (synonym_query != NULL)
        {
            for (fieldInfoIterator = fieldInfo.begin(); fieldInfoIterator != fieldInfo.end(); fieldInfoIterator++)
            {
                TokenStream * synonym_tokens;
                Token synonym_token;

                synonym_query->reset(0);
                synonym_tokens = cl_analyzer->tokenStream((fieldInfoIterator->first).c_str(), synonym_query);

                while (synonym_tokens->next(&synonym_token) != false)
                {
                    TermQuery* synonymQuery;

                    synonymQuery = _CLNEW TermQuery(_CLNEW Term((fieldInfoIterator->first).c_str(), synonym_token.termText()));
                    synonymQuery->setBoost(fieldInfoIterator->second);

                    final_query->add(synonymQuery, false, false); //optional
                }
            }
        }
        htsearchapi_debug->outlog(3, " done\n");


        if (htsearchapi_debug->getLevel() > 4)
        {
            wchar_t * converted_query = final_query->toString(_T("contents"));
            char * converted_query_utf8 = wchar_to_utf8(converted_query);
            htsearchapi_debug->outlog(4, "Converted query before searching: %s\n", converted_query_utf8);
            free(converted_query);
            free(converted_query_utf8);
        }


        //
        // result sorting options
        //
        sorter = _CLNEW Sort();
        switch (htsearch_query->sortby_flag)
        {
            case HTSEARCH_SORT_REV_SCORE:
                // 
                // what the hell is this? why would anyone do this?
                //
                htsearchapi_debug->outlog(3, "Sorting by reverse score... Wait, no we're not\n");
                break;
            case HTSEARCH_SORT_TIME:
                htsearchapi_debug->outlog(3, "Sorting by time\n");
                sorter->setSort(_CLNEW SortField (_T("doc-time"), SortField::INT, false));
                break;
            case HTSEARCH_SORT_REV_TIME:
                htsearchapi_debug->outlog(3, "Sorting by reverse time\n");
                sorter->setSort(_CLNEW SortField (_T("doc-time"), SortField::INT, true));
                break;
            case HTSEARCH_SORT_TITLE:
                htsearchapi_debug->outlog(3, "Sorting by title\n");
                sorter->setSort(_CLNEW SortField (_T("doc-title"), SortField::STRING, false));
                break;
            case HTSEARCH_SORT_REV_TITLE:
                htsearchapi_debug->outlog(3, "Sorting by reverse title\n");
                sorter->setSort(_CLNEW SortField (_T("doc-title"), SortField::STRING, true));
                break;
            case HTSEARCH_SORT_SCORE:
                htsearchapi_debug->outlog(3, "Sorting by score... doing nothing\n");
                break;
        }


        //
        // perform the actual search
        //
        htsearchapi_debug->outlog(3, "Performing search...\n");
        cl_hits = cl_searcher->search( final_query, sorter);


        //
        // search result filtering
        //
        if (cl_hits->length() > 0)
        {
            htsearchapi_debug->outlog(3, "Search success\n");

            HtRegex  * alwaysReturn_pattern = NULL;
            HtRegex  * restrict_pattern = NULL;
            HtRegex  * exclude_pattern = NULL;

            if (config->Find("exclude").length())
            {
                StringList l(config->Find("exclude"), " \t\r\n\001|");

                exclude_pattern = new HtRegex();
                exclude_pattern->setEscaped(l);

                htsearchapi_debug->outlog(5, "Exclude search filter set up\n");
            }
            if (config->Find("restrict").length())
            {
                StringList l(config->Find("restrict"), " \t\r\n\001|");

                restrict_pattern = new HtRegex();
                restrict_pattern->setEscaped(l);

                htsearchapi_debug->outlog(5, "Restrict search filter set up\n");
            }
            if (config->Find("alwaysreturn").length())
            {
                StringList l(config->Find("alwaysreturn"), " \t\r\n\001|");

                alwaysReturn_pattern = new HtRegex();
                alwaysReturn_pattern->setEscaped(l);

                htsearchapi_debug->outlog(5, "AlwaysReturn search filter set up\n");
            }

            //
            // the hit map needs to be allocated, and will contain at most 
            // the same number of documents that were found by CLucene
            //
            hit_map = (int *)calloc(cl_hits->length() + 1, sizeof(int));

            for (int i = 0; i < cl_hits->length(); i++)
            { 
                String url = wchar_to_utf8( cl_hits->doc(i).get(_T("url")));;

                htsearchapi_debug->outlog(5, "CLucene result #%d: [%s] ", i, url.get());

                if (alwaysReturn_pattern && alwaysReturn_pattern->match(url, 1, 0) != 0)
                {
                    htsearchapi_debug->outlog(5, "added as #%d by alwaysReturn filter\n", htsearchapi_total_matches);
                    hit_map[htsearchapi_total_matches] = i;
                    htsearchapi_total_matches++;
                }
                else if (restrict_pattern && restrict_pattern->match(url, 1, 0) == 0)
                {
                    htsearchapi_debug->outlog(5, "rejected by restrict filter\n");
                }
                else
                {
                    if (exclude_pattern && exclude_pattern->match(url, 0, 0) != 0)
                    {
                        htsearchapi_debug->outlog(5, "rejected by exclude all filter\n");
                        continue;
                    }
                    else
                    {
                        htsearchapi_debug->outlog(5, "added as #%d by passing all filters\n", htsearchapi_total_matches);
                        hit_map[htsearchapi_total_matches] = i;
                        htsearchapi_total_matches++;
                    }
                }
            }
        }

        htsearchapi_debug->outlog(1, "CLucene found %d raw results\n", cl_hits->length());
        htsearchapi_debug->outlog(1, "HtDig approved %d results after limits were applied\n", htsearchapi_total_matches);

        return htsearchapi_total_matches;
    }
    catch (CLuceneError& e)
    {
        HtDebug * debug = HtDebug::Instance();
        debug->outlog(1, "\nException in libhtdig_htsearch (htsearch_query): [%s]\n", e.what());
        debug->close();
        return (HTSEARCH_ERROR_QUERY_ERROR);
    }
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
    HtConfiguration * config = HtConfiguration::config();
    if (cl_searcher == NULL)
    {
        htsearchapi_debug->outlog (2, "HtSearch: tried to get hit before htsearch_open\n");
        return (FALSE);
    }
    else if (cl_hits == NULL)
    {
        htsearchapi_debug->outlog (2, "HtSearch: tried to get match without doing search first\n");
        return (FALSE);
    }
    else if (n > htsearchapi_total_matches)
    {
        htsearchapi_debug->close();
        return (FALSE);
    }
    else
    {
        char * temp;

        htsearchapi_debug->outlog (4, "HtSearch: Retrieving match #%d\n", n);
        lucene::document::Document doc = cl_hits->doc(hit_map[n]);

        //
        // clear the strings in the match structure
        //
        hit_struct->URL[0] = '\0';
        hit_struct->name[0] = '\0';
        hit_struct->title[0] = '\0';
        hit_struct->excerpt[0] = '\0';


        temp = wchar_to_utf8(doc.get(_T("url")));
        strncpy (hit_struct->URL, temp, HTDIG_MAX_FILENAME_PATH_L);
        hit_struct->URL[HTDIG_MAX_FILENAME_PATH_L - 1] = '\0';
        free(temp);

        
        temp = wchar_to_utf8(doc.get(_T("doc-name")));
        strncpy (hit_struct->name, temp, HTDIG_MAX_FILENAME_PATH_L);
        hit_struct->URL[HTDIG_MAX_FILENAME_PATH_L - 1] = '\0';
        free(temp);


        temp = wchar_to_utf8(doc.get(_T("id")));
        hit_struct->id = atoi(temp);
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

        htsearchapi_debug->outlog (4, "HtSearch: Match #%d retrieved, doing highlighting\n", n);

        //
        // use the intial query for highlighting, because it hasn't been
        // changed from its initial form. this is broken on windows
        //
        if (htsearchapi_highlight_text.length() > 0)
        {
            Query * query = QueryParser::parse(htsearchapi_highlight_text.c_str(), _T("contents"), cl_analyzer);

            QueryScorer * scorer = _CLNEW QueryScorer(query);
            Highlighter * highlighter = _CLNEW Highlighter(scorer);
            wchar_t * wtemp = highlighter->getBestFragment(cl_analyzer, _T("contents"), doc.get(_T("contents")));

            if (wtemp)
            {
                temp = wchar_to_utf8(wtemp);
                free(wtemp);
            }
            else
            {
                //
                // highlighter couldn't find anything... just use the top of the contents
                //
                temp = wchar_to_utf8(doc.get(_T("contents")));
            }

            _CLDELETE(scorer);
            _CLDELETE(highlighter);
        }
        else
        {
            //
            // no highlight text available...  just use the top of the contents
            //
            temp = wchar_to_utf8(doc.get(_T("contents")));
        }

        strncpy (hit_struct->excerpt, temp, HTDIG_DOCUMENT_EXCERPT_L);
        hit_struct->excerpt[HTDIG_DOCUMENT_EXCERPT_L - 1] = '\0';
        free(temp);

        htsearchapi_debug->outlog (4, "HtSearch: Match #%d highlighted, setting score, time and size\n", n);

        temp = wchar_to_utf8(doc.get(_T("doc-size")));
        hit_struct->size = atoi(temp);
        free(temp);

        temp = wchar_to_utf8(doc.get(_T("doc-time")));
        hit_struct->time = (time_t)atoi(temp);
        free(temp);

        if (config->Boolean("use_score_smoothing"))
        {
            //
            // math magic!!!
            //
            /*
             * Log 10 smoothing with scaling
             *
            double orig_score, temp_score, log_score;
            double scale = 4.0;

            orig_score = cl_hits->score(n);                 // should be between 0 and 1
            temp_score = orig_score * (exp(scale) - M_E);   // should be between 0 and (e^scale)-e
            temp_score = temp_score + M_E;                  // should be between e and e^scale
            log_score = log(temp_score);                    // should be between 0 and scale
            log_score = log_score / scale;                  // should be between 0 and 1

            htsearchapi_debug->outlog (2, "score: %f ... %f ... %f\n", orig_score, temp_score, log_score );
            hit_struct->score = log_score > 0 ? log_score : 0;      // just in case
            */

            double orig_score, log_score;

            orig_score = cl_hits->score(n);         // should be between 0 and 1
            log_score = log(fabs(orig_score + 1));  // should be between 0 and ln(2)
            log_score = log_score / log(2.0);       // should be between 0 and 1

            hit_struct->score = log_score > 0 ? log_score : 0;      // just in case
        }
        else
        {
            hit_struct->score = cl_hits->score(n);
        }

        //
        // sanitize the return strings (since they've been truncated)
        //
        sanitize_utf8_string(hit_struct->URL);
        sanitize_utf8_string(hit_struct->name);
        sanitize_utf8_string(hit_struct->title);
        sanitize_utf8_string(hit_struct->excerpt);

        return (TRUE);
    }
}


DLLEXPORT int htsearch_close()
{
    HtDebug * htsearchapi_debug = HtDebug::Instance();

    if (hit_map != NULL)
    {
        free(hit_map);
        hit_map = NULL;
    }

    if (cl_searcher == NULL)
    {
        return (TRUE);
    }

    //
    // Delete the CLucene objects
    //
    _CLDELETE(cl_searcher);
    _CLDELETE(cl_analyzer);
    //_CLDELETE(cl_parser);
    //_CLDELETE(cl_hits);

    cl_searcher = NULL;
    cl_analyzer = NULL;
    //cl_parser = NULL;
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
