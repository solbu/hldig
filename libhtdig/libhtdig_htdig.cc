//-------------------------------------------------------------
//
// libhtdig_htdig.cc
//
// 1/25/2002 created from htdig.cc
//
// Neal Richter nealr@rightnow.com
//
// libhtdig_htdig.cc
// 
// htdig: Indexes the web sites specified in the config file
//        generating several databases to be used by htmerge
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: libhtdig_htdig.cc,v 1.5 2004/05/28 13:15:29 lha Exp $
//
//-------------------------------------------------------------

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_STD
#include <iostream>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif
#else
#include <iostream.h>
#endif /* HAVE_STD */

extern "C" {
#include "libhtdig_api.h"
}

#include "libhtdig_log.h"

#include "BasicDocument.h"
#include "Document.h"
#include "TextCollector.h"
#include "Retriever.h"
#include "StringList.h"
#include "htdig.h"
#include "defaults.h"
#include "HtURLCodec.h"
#include "WordContext.h"
#include "HtDateTime.h"
#include "HtURLRewriter.h"
#include "URL.h"
#include "Server.h"

////////////////////////////
// For cookie jar
////////////////////////////
#include "HtCookieJar.h"
#include "HtCookieMemJar.h"
#include "HtHTTP.h"
////////////////////////////

// If we have this, we probably want it.
//#ifdef HAVE_GETOPT_H
//#include <getopt.h>
//#endif



//Global Variables for Library

int debug = 0;
HtRegexList limits;
HtRegexList limitsn;
String configFile = DEFAULT_CONFIG_FILE;
FILE *urls_seen = NULL;
FILE *images_seen = NULL;
DocumentDB docs;

    
//
// Global variables for this file
//
static int report_statistics = 0;
static String minimalFile = 0;
static HtDateTime StartTime;
static HtDateTime EndTime;

//static char *max_hops = NULL;
static String credentials;
static HtCookieJar *_cookie_jar = NULL;
static HtConfiguration * config = NULL;
static WordContext * wc = NULL;

static int create_text_database = 0;
static int alt_work_area = 0;
static int initial = 0;

int htdig_index_open_flag = FALSE;


//new.  URLs from 'command-line'
#define URL_SEPCHARS    " ,"
static char *myURL = NULL;


BasicDocument *a_basicdoc;
TextCollector *Indexer;

BasicDocument the_basicdoc;
//TextCollector the_Indexer;

/*******************************************************
 *
 *   LIBHTDIG API FUNCTION
 *
 *   int htdig_index_open(...)
 * 
 * 
 *   opens/creates document indexes and initializes variables
 *   for indexing.
 *
 *
 *   see libhtdig_api.h headerfile for definition of 
 *       htdig_parameters_struct
 *
 *
 *   TODO Examine external function calls for error return
 *          codes
 *
 *******************************************************/
 
int htdig_index_open(htdig_parameters_struct * htdig_parms)
{
    int ret = -1;

    if(htdig_index_open_flag != FALSE)
        return(FALSE);
        
    //load 'comand-line' parameters
    
    if (htdig_parms->configFile[0] != 0)
        configFile = htdig_parms->configFile;
    
    if (htdig_parms->URL[0] != 0)
    {
        myURL = strdup(htdig_parms->URL);
    }
    
    debug = htdig_parms->debug;
    if(debug != 0)
    {
        ret = logOpen(htdig_parms->logFile);

        if(ret == FALSE)
        {
            reportError (form ("[HTDIG] Error opening log file [%s] . Error:[%d], %s\n",
                   htdig_parms->logFile, errno, strerror(errno)) );
            return(HTDIG_ERROR_LOGFILE_OPEN);
        }
    }

    initial = htdig_parms->initial;
    create_text_database  =  htdig_parms->create_text_database;
    //max_hops  =  strdup(htdig_parms->max_hops);
    report_statistics  =  htdig_parms->report_statistics;
    credentials = htdig_parms->credentials;
    alt_work_area  =  htdig_parms->alt_work_area;
    minimalFile = htdig_parms->minimalFile;

    
    if(htdig_parms->use_cookies == TRUE)
    {
        // Cookie jar dynamic creation.

        _cookie_jar = new HtCookieMemJar ();    // new cookie jar
        if (_cookie_jar)
            HtHTTP::SetCookieJar (_cookie_jar);
    }

    //
    // First set all the defaults and then read the specified config
    // file to override the defaults.
    //

    config = HtConfiguration::config ();

    config->Defaults (&defaults[0]);
    if (access ((char *) configFile, R_OK) < 0)
    {
        reportError (form ("[HTDIG] Unable to find configuration file '%s'",
                        configFile.get ()));
        return(HTDIG_ERROR_CONFIG_READ);
    }
    config->Read (configFile);

    //------- Now override config settings ------------

    //------- override database path ------------
    if(strlen(htdig_parms->DBpath) > 0)
    {
        config->Add("database_dir", htdig_parms->DBpath);
    }

    //------- custom filters from htdig_parms ----------

    if(strlen(htdig_parms->locale) > 0)
    {
        config->Add("locale", htdig_parms->locale);
    }

    if (config->Find ("locale").empty () && debug > 0)
        logEntry("Warning: unknown locale!\n");

    if (strlen(htdig_parms->max_hops) > 0)
    {
        config->Add ("max_hop_count", htdig_parms->max_hops);
    }

    if(strlen(htdig_parms->limit_urls_to) > 0)
    {
        config->Add("limit_urls_to", htdig_parms->limit_urls_to);
    }
    
    if(strlen(htdig_parms->limit_normalized) > 0)
    {
        config->Add("limit_normalized", htdig_parms->limit_normalized);
    }

    if(strlen(htdig_parms->exclude_urls) > 0)
    {
        config->Add("exclude_urls", htdig_parms->exclude_urls);
    }
    
    if(strlen(htdig_parms->url_rewrite_rules) > 0)
    {
        config->Add("url_rewrite_rules", htdig_parms->url_rewrite_rules);
    }
    
    if(strlen(htdig_parms->bad_querystr) > 0)
    {
        config->Add("bad_querystr", htdig_parms->bad_querystr);
    }

    if(strlen(htdig_parms->locale) > 0)
    {
        config->Add("locale", htdig_parms->locale);
    }

    if(strlen(htdig_parms->meta_description_factor) > 0)
    {
        config->Add("meta_description_factor", htdig_parms->meta_description_factor);
    }

    if(strlen(htdig_parms->title_factor) > 0)
    {
        config->Add("title_factor", htdig_parms->title_factor);
    }

    if(strlen(htdig_parms->text_factor) > 0)
    {
        config->Add("text_factor", htdig_parms->text_factor);
    }
    
    if(strlen(htdig_parms->URL) > 0)
    {
        config->Add("start_url", htdig_parms->URL);
        free(myURL);
        myURL=NULL;
    }
    
    //------- end custom filters from htdig_parms ----------

    // Set up credentials for this run
    if (credentials.length ())
        config->Add ("authorization", credentials);

    //
    // Check url_part_aliases and common_url_parts for
    // errors.
    String url_part_errors = HtURLCodec::instance ()->ErrMsg ();

    if (url_part_errors.length () != 0)
    {
        reportError (form("[HTDIG] Invalid url_part_aliases or common_url_parts: %s",
                    url_part_errors.get ()));
        return(HTDIG_ERROR_URL_PART);
    }
    //
    // Check url_rewrite_rules for errors.
    String url_rewrite_rules = HtURLRewriter::instance ()->ErrMsg ();

    if (url_rewrite_rules.length () != 0)
    {
        reportError (form ("[HTDIG] Invalid url_rewrite_rules: %s",
                        url_rewrite_rules.get ()));
        return(HTDIG_ERROR_URL_REWRITE);
    }

    //
    // If indicated, change the database file names to have the .work
    // extension
    //
    if (alt_work_area != 0)
    {
        String configValue = config->Find ("doc_db");

        if (configValue.length () != 0)
        {
            configValue << ".work";
            config->Add ("doc_db", configValue);
        }

        configValue = config->Find ("word_db");
        if (configValue.length () != 0)
        {
            configValue << ".work";
            config->Add ("word_db", configValue);
        }

        configValue = config->Find ("doc_index");
        if (configValue.length () != 0)
        {
            configValue << ".work";
            config->Add ("doc_index", configValue);
        }

        configValue = config->Find ("doc_excerpt");
        if (configValue.length () != 0)
        {
            configValue << ".work";
            config->Add ("doc_excerpt", configValue);
        }

        configValue = config->Find ("md5_db");
        if (configValue.length () != 0)
        {
            configValue << ".work";
            config->Add ("md5_db", configValue);
        }
    }

    //
    // If needed, we will create a list of every URL we come across.
    //TODO put document-index log file stuff here

    if (config->Boolean ("create_url_list"))
    {
        const String filename = config->Find ("url_list");
        urls_seen = fopen (filename, initial ? "w" : "a");
        if (urls_seen == 0)
        {
            reportError (form ("[HTDIG] Unable to create URL file '%s'",
                            filename.get ()));
            return(HTDIG_ERROR_URL_CREATE_FILE);
        }
    }

    //
    // If needed, we will create a list of every image we come across.
    //
    if (config->Boolean ("create_image_list"))
    {
        const String filename = config->Find ("image_list");
        images_seen = fopen (filename, initial ? "w" : "a");
        if (images_seen == 0)
        {
            reportError (form ("[HTDIG] Unable to create images file '%s'",
                            filename.get ()));
            return(HTDIG_ERROR_IMAGE_CREATE_FILE);
        }
    }

    //
    // Set up the limits list
    //
    StringList l (config->Find ("limit_urls_to"), " \t");
    limits.setEscaped (l, config->Boolean ("case_sensitive"));
    l.Destroy ();

    l.Create (config->Find ("limit_normalized"), " \t");
    limitsn.setEscaped (l, config->Boolean ("case_sensitive"));
    l.Destroy ();

    //
    // Open the document database
    //
    const String filename = config->Find ("doc_db");
    if (initial)
        unlink (filename);

    const String index_filename = config->Find ("doc_index");
    if (initial)
        unlink (index_filename);

    const String head_filename = config->Find ("doc_excerpt");
    if (initial)
        unlink (head_filename);

    if (docs.Open (filename, index_filename, head_filename) < 0)
    {
        reportError (form ("[HTDIG] Unable to open/create document database '%s'",
                        filename.get ()));
        return(HTDIG_ERROR_OPEN_CREATE_DOCDB);
    }

    const String word_filename = config->Find ("word_db");
    if (initial)
        unlink (word_filename);

    // Initialize htword
    wc = new WordContext;
    wc->Initialize(*config);


    //a_basicdoc = new BasicDocument;
    Indexer = new TextCollector;
    
    a_basicdoc = &the_basicdoc;
    a_basicdoc->Reset();
    
    //Indexer = &the_Indexer;

    if ((a_basicdoc == NULL) || (Indexer == NULL))
        return(FALSE);


    htdig_index_open_flag = TRUE;

    return(TRUE);

}

/*******************************************************
 *
 *   LIBHTDIG API FUNCTION
 *
 *   int htdig_index_simple_doc(...)
 * 
 * 
 *   indexes a simple document supplied by parameter
 *
 *   see libhtdig_api.h headerfile for definition of 
 *       htdig_simple_doc_struct
 *  
 *   TODO Examine external function calls for error return
 *          codes
 *  
 *******************************************************/
int htdig_index_simple_doc(htdig_simple_doc_struct * a_simple_doc)
{
    int index_error = 0;
    //int ret = 0;
    
    // Reset the document to clean out any old data
    a_basicdoc->Reset();

    a_basicdoc->ModTime(a_simple_doc->doc_time);
    a_basicdoc->Location(a_simple_doc->location);
    a_basicdoc->DocumentID(a_simple_doc->documentid);
    a_basicdoc->Title(a_simple_doc->title);
    a_basicdoc->MetaContent(a_simple_doc->meta);
    a_basicdoc->Contents(a_simple_doc->contents);              //MUST ALLOCATE & FREE!!!
    a_basicdoc->ContentType(a_simple_doc->content_type);       //MIME-ISH string
    a_basicdoc->Length();


    //TODO What is this error?
    index_error = Indexer->IndexDoc(*a_basicdoc);
    
    return(TRUE);
}

/*******************************************************
 *
 *   LIBHTDIG API FUNCTION
 *
 *   int htdig_index_urls(...)
 * 
 *   Starts fetch & index of URL supplied in config file 
 *       OR supplied in htdig_index_open parameter
 * 
 *   TODO Examine external function calls for error return
 *          codes
 *   TODO  Blank/empty URL error?
 *******************************************************/
int htdig_index_urls(void)
{

    char * temp_URL_list = NULL;
    char * temp_url = NULL;

    // Create the Retriever object which we will use to parse all the
    // HTML files.
    // In case this is just an update dig, we will add all existing
    // URLs?
    //
    Retriever retriever (Retriever_logUrl);
    if (minimalFile.length () == 0)
    {
        List *list = docs.URLs ();
        retriever.Initial (*list);
        delete list;

        // Add start_url to the initial list of the retriever.
        // Don't check a URL twice!
        // Beware order is important, if this bugs you could change 
        // previous line retriever.Initial(*list, 0) to Initial(*list,1)
        retriever.Initial (config->Find ("start_url"), 1);
    }

    // Handle list of URLs given on 'command-line'
    if (myURL != NULL)
    {
        String str;
        temp_URL_list = strdup(myURL);
        temp_url = strtok(temp_URL_list, URL_SEPCHARS);
        while (temp_url != NULL)
        {
            str = temp_url;
            str.chop ("\r\n");
            if (str.length () > 0)
                retriever.Initial (str, 1);

            temp_url = strtok(NULL, URL_SEPCHARS);
        }
        free(temp_URL_list);
    }
    else if (minimalFile.length () != 0)
    {
        FILE *input = fopen (minimalFile.get (), "r");
        char buffer[1000];

        if (input)
        {
            while (fgets (buffer, sizeof (buffer), input))
            {
                String str (buffer);
                str.chop ("\r\n\t ");
                if (str.length () > 0)
                    retriever.Initial (str, 1);
            }
            fclose (input);
        }
    }

    //
    // Go do it!
    //
    retriever.Start ();

    //
    // All done with parsing.
    //

    //
    // If the user so wants, create a text version of the document database.
    //

    if (create_text_database)
    {
        const String doc_list = config->Find ("doc_list");
        if (initial)
            unlink (doc_list);
        docs.DumpDB (doc_list);
        const String word_dump = config->Find ("word_dump");
        if (initial)
            unlink (word_dump);
        HtWordList words (*config);
        if (words.Open (config->Find ("word_db"), O_RDONLY) == OK)
        {
            words.Dump (word_dump);
        }
    }

    //
    // Cleanup
    //
    if (images_seen)
        fclose (images_seen);

    //
    // If needed, report some statistics
    //
    if (report_statistics)
    {
        retriever.ReportStatistics ("htdig");
    }

     return(TRUE);
}


/*******************************************************
 *
 *   LIBHTDIG API FUNCTION
 *
 *   int htdig_index_close(...)
 *
 *   Closes the database and destroys various objects
 * 
 *   TODO Examine external function calls for error return
 *          codes
 *  
 *******************************************************/
int htdig_index_close(void)
{
    int ret = -1;

    if(htdig_index_open_flag == TRUE)
    {
        //delete a_basicdoc;
        //delete Indexer;

        Indexer->FlushWordDB();

        if (_cookie_jar)
            delete _cookie_jar;

        //if (max_hops != NULL)
        //    free(max_hops);

        if (myURL != NULL)
            free(myURL);

        //call destructors here
        docs.~DocumentDB();
        //config->~HtConfiguration();

        if (debug != 0)
        {
            ret = logClose();

            if (ret == FALSE)
            {
                reportError (form ("[HTDIG] Error closing log file . Error:[%d], %s\n",
                            errno, strerror(errno)) );
                return(HTDIG_ERROR_LOGFILE_CLOSE);
            }
        }

        /*
           if(config) {
           WordContext::Finish();
           }
         */

        if (wc)
            delete wc;
    
        if (urls_seen)
            fclose (urls_seen);

        htdig_index_open_flag = FALSE;
    }

    return(TRUE);
}

/*******************************************************
 *
 *   LIBHTDIG API FUNCTION
 *
 *   int htdig_index_reset(...)
 * 
 * 
 *   TODO Examine external function calls for error return
 *          codes
 *  
 *******************************************************/

int htdig_index_reset(void)
{
    Indexer->FlushWordDB();
    a_basicdoc->Reset();
    
    return(TRUE);
}

/*******************************************************
 *
 *   LIBHTDIG API FUNCTION
 *
 *   int htdig_get_max_head_length(...)
 * 
 * 
 *   Returns size of maximum document storage length
 *      for db.excerpts  [htdig.conf:max_head_length]
 *
 *   This represents the maximum amount of the document
 *   That will be available for excerpting.
 *  
 *
 *******************************************************/

int htdig_get_max_head_length()
{
    int ret = -1;

    if(config != NULL)    
       ret = config->Value("max_head_length");
    
    return(ret);
}

/*******************************************************
 *
 *   LIBHTDIG API FUNCTION
 *
 *   int htdig_index_test_url(...)
 * 
 *
 *   Test a URL for filter Pass/Fail
 *
 *   Pass = return(TRUE)
 *   Fail = return(XXX)  [Negative Value]
 *   
 *     
 *
 *  
 *
 *******************************************************/


//int htdig_index_test_url(htdig_parameters_struct *htdig_parms)
int htdig_index_test_url(htdig_parameters_struct *htdig_parms)
{ 
    //int ret = FALSE;
    String the_URL(htdig_parms->URL);
    HtConfiguration* config= HtConfiguration::config();
    Dictionary	invalids;
    Dictionary	valids;
    URL 	aUrl(the_URL);
    String	rewritten_url(the_URL);
    StringList	tmpList;
    HtRegex		limitTo;
    HtRegex		excludeFrom;

    //initalize outgoing-parameter rewritten_URL
    htdig_parms->rewritten_URL[0] = 0;
    
#ifdef DEBUG
    //output relevant config variables
    cout << "   bad_extensions = " << config->Find("bad_extensions")  << endl;
    cout << "   valid_extensions = " << config->Find("valid_extensions")  << endl;
    cout << "   exclude_urls = " << config->Find("exclude_urls") << endl;
    cout << "   bad_querystr = " << config->Find("bad_querystr") << endl;
    cout << "   limit_urls_to = " << config->Find("limit_urls_to") << endl;
    cout << "   limit_normalized = " << config->Find("limit_normalized") << endl;
    cout << "   restrict = " << config->Find("restrict") << endl;
    cout << "   exclude = " << config->Find("exclude") << endl;
#endif

    //------------ read the config file if it is given ---------------
    if (htdig_parms->configFile[0] != 0)
        configFile = htdig_parms->configFile;

    config = HtConfiguration::config ();

    config->Defaults (&defaults[0]);
    if (access ((char *) configFile, R_OK) < 0)
    {
        reportError (form ("[HTDIG] Unable to find configuration file '%s'",
                        configFile.get ()));
        return(HTDIG_ERROR_CONFIG_READ);
    }
    config->Read (configFile);
    
    //---------- Now override config settings -----------------
    
    //------- override database path ------------
    if(strlen(htdig_parms->DBpath) > 0)
    {
        config->Add("database_dir", htdig_parms->DBpath);
    }

    //------- custom filters from htdig_parms ----------

    if(strlen(htdig_parms->locale) > 0)
    {
        config->Add("locale", htdig_parms->locale);
    }

    if (config->Find ("locale").empty () && debug > 0)
        logEntry("Warning: unknown locale!\n");

    if (strlen(htdig_parms->max_hops) > 0)
    {
        config->Add ("max_hop_count", htdig_parms->max_hops);
    }

    if(strlen(htdig_parms->limit_urls_to) > 0)
    {
        config->Add("limit_urls_to", htdig_parms->limit_urls_to);
    }
    
    if(strlen(htdig_parms->limit_normalized) > 0)
    {
        config->Add("limit_normalized", htdig_parms->limit_normalized);
    }

    if(strlen(htdig_parms->exclude_urls) > 0)
    {
        config->Add("exclude_urls", htdig_parms->exclude_urls);
    }
    
    if(strlen(htdig_parms->url_rewrite_rules) > 0)
    {
        config->Add("url_rewrite_rules", htdig_parms->url_rewrite_rules);
    }
    
    if(strlen(htdig_parms->bad_querystr) > 0)
    {
        config->Add("bad_querystr", htdig_parms->bad_querystr);
    }

    if(strlen(htdig_parms->locale) > 0)
    {
        config->Add("locale", htdig_parms->locale);
    }

    if(strlen(htdig_parms->meta_description_factor) > 0)
    {
        config->Add("meta_description_factor", htdig_parms->meta_description_factor);
    }

    if(strlen(htdig_parms->title_factor) > 0)
    {
        config->Add("title_factor", htdig_parms->title_factor);
    }

    if(strlen(htdig_parms->text_factor) > 0)
    {
        config->Add("text_factor", htdig_parms->text_factor);
    }
    
    //-------------------------------------------------------------------
    
#ifdef DEBUG
    //output relevant config variables
    cout << "   bad_extensions = " << config->Find("bad_extensions")  << endl;
    cout << "   valid_extensions = " << config->Find("valid_extensions")  << endl;
    cout << "   exclude_urls = " << config->Find("exclude_urls") << endl;
    cout << "   bad_querystr = " << config->Find("bad_querystr") << endl;
    cout << "   limit_urls_to = " << config->Find("limit_urls_to") << endl;
    cout << "   limit_normalized = " << config->Find("limit_normalized") << endl;
    cout << "   restrict = " << config->Find("restrict") << endl;
    cout << "   exclude = " << config->Find("exclude") << endl;
#endif


    //------ bad_extensions -----------------------------------------------
    //A list of bad extensions, separated by spaces or tabs
    
    String	t = config->Find("bad_extensions");
    String lowerp;
    char	*p = strtok(t, " \t");
    while (p)
    {
        // Extensions are case insensitive
        lowerp = p;
        lowerp.lowercase();
        invalids.Add(lowerp, 0);
        p = strtok(0, " \t");
    }
    
    
    //------ valid_extensions ------------------------------------------------
    // Valid extensions are performed similarly 
    // A list of valid extensions, separated by spaces or tabs

    t = config->Find("valid_extensions");
    p = strtok(t, " \t");
    while (p)
    {
        // Extensions are case insensitive
        lowerp = p;
        lowerp.lowercase();
        valids.Add(lowerp, 0);
        p = strtok(0, " \t");
    }

    //----- rewrite the URL------------------------------------------
    aUrl.rewrite();
    rewritten_url = aUrl.get(); 

    if(rewritten_url.length() <= 0)
    {
        //Rejected: empty rewritten URL
        String temp =  config->Find("url_rewrite_rules");
        strcpy(htdig_parms->rewritten_URL, temp.get());
        system(form("echo \"%s\" > /tmp/neal", temp.get()));
        
        return(HTDIG_ERROR_TESTURL_REWRITE_EMPTY);
    }
    
    //cout << form("TestURL: org=[%s]\n", the_URL.get());
    //cout << form("    rewritten[%s]\n", rewritten_url.get());
    
    //copy the rewritten URL for outgoing parm pass
    strcpy(htdig_parms->rewritten_URL, rewritten_url.get());

    //---- exclude_urls ---------------------------------------------
    // If the URL contains any of the patterns in the exclude list,
    // mark it as invalid
    
    /*if(strlen(htdig_parms->exclude_urls) > 0)
        tmpList.Create(htdig_parms->exclude_urls," \t");
    else*/
        tmpList.Create(config->Find("exclude_urls")," \t");
    
    HtRegexList excludes;
    excludes.setEscaped(tmpList, config->Boolean("case_sensitive"));
    if (excludes.match(rewritten_url, 0, 0) != 0)
    {
        //Rejected: item in exclude list
        return(HTDIG_ERROR_TESTURL_EXCLUDE);
    }

    //---- bad_querystr -------------------------------------------
    // If the URL has a query string and it is in the bad query list
    // mark it as invalid

    tmpList.Destroy();
    
    /*if(strlen(htdig_parms->bad_querystr) > 0)
        tmpList.Create(htdig_parms->bad_querystr, " \t");
    else*/
        tmpList.Create(config->Find("bad_querystr"), " \t");
    
    HtRegexList badquerystr;
    badquerystr.setEscaped(tmpList, config->Boolean("case_sensitive"));
    char *ext = strrchr((char*)rewritten_url, '?');
    if (ext && badquerystr.match(ext, 0, 0) != 0)
    {
        //if (debug > 2)
        //    cout << endl << "   Rejected: item in bad query list ";
        return(HTDIG_ERROR_TESTURL_BADQUERY);
    }

    //------ invalid_extensions #2 ------
    // See if the file extension is in the list of invalid ones
    
    ext = strrchr((char*)rewritten_url, '.');
    String	lowerext;
    if (ext && strchr(ext,'/'))		// Ignore a dot if it's not in the
        ext = NULL;			// final component of the path.
    if(ext)
    {
        lowerext.set(ext);
        int parm = lowerext.indexOf('?');	// chop off URL parameter
        if (parm >= 0)
            lowerext.chop(lowerext.length() - parm);
        lowerext.lowercase();
        if (invalids.Exists(lowerext))
        {
            //Rejected: Extension is invalid!
            return(HTDIG_ERROR_TESTURL_EXTENSION);
        }
    }
    
    //------ valid_extensions #2 ------
    // Or NOT in the list of valid ones
    
    if (ext && valids.Count() > 0 && !valids.Exists(lowerext))
    {
        //Rejected: Extension is not valid!
        return(HTDIG_ERROR_TESTURL_EXTENSION2);
    }

    //----- limit_urls_to & limit_normalized ------------------------------
    // Set up the limits list
    
    StringList l;
    /*if(strlen(htdig_parms->limit_urls_to) > 0)
        l.Create(htdig_parms->limit_urls_to, " \t");
    else*/
        l.Create(config->Find ("limit_urls_to"), " \t");
        
    limits.setEscaped (l, config->Boolean ("case_sensitive"));

    l.Destroy ();

    /*if(strlen(htdig_parms->limit_normalized) > 0)
        l.Create (htdig_parms->limit_normalized, " \t");
    else*/
        l.Create (config->Find ("limit_normalized"), " \t");

    limitsn.setEscaped (l, config->Boolean ("case_sensitive"));
    l.Destroy ();

    // If any of the limits are met, we allow the URL
    if (limits.match(rewritten_url, 1, 0) == 0) 
    {
        //Rejected: URL not in the limits!;
        return(HTDIG_ERROR_TESTURL_LIMITS);
    }
    
    
    // or not in list of normalized urls
    // Warning! should be last in checks because of aUrl normalization
    aUrl.normalize();
    if (limitsn.match(rewritten_url.get(), 1, 0) == 0) 
    {
        //Rejected: not in "limit_normalized" list!
        return(HTDIG_ERROR_TESTURL_LIMITSNORM);
    } 

    //----- restrict & exclude ----------------------------------
    //Search-Time Filters

    String temp;
    
    /*if(strlen(htdig_parms->search_restrict) > 0)
        temp = htdig_parms->search_restrict;
    else*/
        temp = config->Find("restrict");
    
    if (temp.length())
	{
	    // Create a temporary list from either the configuration
	    // file or the input parameter
	    StringList l(temp, " \t\r\n\001|");
	    limitTo.setEscaped(l);
	}
    
    /*if(strlen(htdig_parms->search_exclude) > 0)
        temp = htdig_parms->search_exclude;
    else*/
        temp = config->Find("exclude");

    if (temp.length())
	{
	    // Create a temporary list from either the configuration
	    // file or the input parameter
	    StringList l(temp, " \t\r\n\001|");
	    excludeFrom.setEscaped(l);
	}

    //Restrict Test
    if (limitTo.match(rewritten_url, 1, 0) == 0)
    {
        //Rejected URL Not in SearchTime Restrict List
        return(HTDIG_ERROR_TESTURL_SRCH_RESTRICT);
    }
    //Exclude Test
    if (excludeFrom.match(rewritten_url, 0, 0) != 0)
    {
        //Rejected URL in SearchTime Exclude List
        return(HTDIG_ERROR_TESTURL_SRCH_EXCLUDE);
    }


    //Success!
    return TRUE;
}
