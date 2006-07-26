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
// $Id: libhtdig_htdig.cc,v 1.6.2.3 2006/07/26 23:44:28 aarnone Exp $
//
//-------------------------------------------------------------

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <iostream>
using namespace std;

extern "C" {
#include "libhtdig_api.h"
}


#include "Spider.h"

static Spider * spider = NULL;
static int openCount = 0;



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
 
DLLEXPORT int htdig_index_open(htdig_parameters_struct * htdig_params)
{
    openCount++;

    if (openCount > 1)
    {
        cout << "HtDig: attempting to open index when already open" << endl;
        return(FALSE);
    }

    spider = new Spider(htdig_params);

    spider->openDBs(htdig_params);

    return (TRUE);
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
DLLEXPORT int htdig_index_simple_doc(htdig_simple_doc_struct * input)
{
    if (spider == NULL)
    {
        return (FALSE);
    }

    singleDoc newDoc;

    newDoc["url"] = input->location;
    newDoc["id"] = input->documentid;
    newDoc["title"] = input->title;
    newDoc["meta-desc"] = input->meta;
    newDoc["contents"] = input->contents;
    newDoc["content-type"] = input->content_type;

    //
    // always send false for addToSpiderQueue, but can probably be a changed in the API
    //
    return spider->addSingleDoc(&newDoc, input->doc_time, input->spiderable,  false);
}


//
// NOTE: the caller must free the contents field!!!
//
DLLEXPORT htdig_simple_doc_struct * htdig_fetch_simple_doc(char * input)
{
    if (spider == NULL)
    {
        return (FALSE);
    }

    string url = input;

    singleDoc * doc = spider->fetchSingleDoc(&url);

    if (doc)
    {
        //cout << "in htdig api before fetch" << endl;
        htdig_simple_doc_struct * output = (htdig_simple_doc_struct*) malloc(sizeof(htdig_simple_doc_struct));
        //cout << "in htdig api after fetch" << endl;

        //
        // set all the "strings" to empty
        //
        output->title[0] = '\0';
        output->meta[0] = '\0';
        output->content_type[0] = '\0';
        output->location[0] = '\0';


        //
        // last modified time (can be overriden by meta tags in the document)
        //
        output->doc_time = atoi((*doc)["doc-time"].c_str());

        //
        // length of the contents
        //
        output->content_length = (*doc)["contents"].length();

        //
        // title string
        //
        strncpy(output->title, (*doc)["title"].c_str(), HTDIG_DOCUMENT_TITLE_L);
        output->title[HTDIG_DOCUMENT_TITLE_L - 1] = '\0';

        //
        // meta description
        //
        strncpy(output->meta, (*doc)["meta-desc"].c_str(), HTDIG_DOCUMENT_META_L);
        output->meta[HTDIG_DOCUMENT_META_L - 1] = '\0';

        //
        // the content type (encoding and mime)
        //
        strncpy(output->content_type, (*doc)["content-type"].c_str(), HTDIG_DOCUMENT_CONTENT_TYPE_L);
        output->content_type[HTDIG_DOCUMENT_CONTENT_TYPE_L - 1] = '\0';

        //
        // the contents field. this is just a char *, so it needs to be malloc'ed
        //
        output->contents = (char*)malloc((output->content_length * sizeof(char)) + 1);
        strncpy(output->contents, (*doc)["contents"].c_str(), output->content_length);
        output->contents[output->content_length - 1] = '\0';

        //
        // this should just contain the URL, but it can be adjusted based on URL redirects
        //
        strncpy(output->location, (*doc)["url"].c_str(), HTDIG_MAX_FILENAME_PATH_L);
        output->location[HTDIG_MAX_FILENAME_PATH_L - 1] = '\0';

        //
        // documentid and spiderable aren't really useful, but what the heck
        //
        output->spiderable = 1;
        output->documentid[0] = '\0';

        return(output);
    }
    else
    {
        return NULL;
    }
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
DLLEXPORT int htdig_index_urls(htdig_parameters_struct * htdig_params)
{
    if (spider == NULL)
    {
        return (FALSE);
    }

    spider->Start(htdig_params);

    return (TRUE);
}


DLLEXPORT int htdig_remove_doc_by_url(char * input)
{
    if (spider == NULL)
    {
        return (FALSE);
    }

    string url = input;
    return spider->DeleteDoc(&url);
}

DLLEXPORT int htdig_remove_doc_by_id(int input)
{
    if (spider == NULL)
    {
        return (FALSE);
    }

    return spider->DeleteDoc(input);
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
DLLEXPORT int htdig_index_close(void)
{
    openCount--;
    if (openCount > 0)
    {
        cout << "Index not really closed" << endl;
        return (TRUE);
    }
    else if (openCount < 0)
    {
        cout << "Index already closed" << endl;
        openCount = 0;
        return (TRUE);
    }

    if(spider != NULL)
    {
        delete spider;
        spider = NULL;
    }

    return (TRUE);
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

DLLEXPORT int htdig_get_max_head_length()
{
    HtConfiguration* config= HtConfiguration::config();

    if(config != NULL)
    {
       return config->Value("max_head_length");
    }
    else
    {
        return -1;
    }
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

DLLEXPORT int htdig_index_test_url(htdig_parameters_struct *htdig_parms)
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

    HtRegexList limits;
    HtRegexList limitsn;

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
        //reportError (form ("[HTDIG] Unable to find configuration file '%s'", configFile.get ()));
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
        cout << "Warning: unknown locale!" << endl;

    if (strlen(htdig_parms->max_hop_count) > 0)
    {
        config->Add ("max_hop_count", htdig_parms->max_hop_count);
    }
    
    if (strlen(htdig_parms->max_head_length) > 0)
    {
        config->Add ("max_head_length", htdig_parms->max_head_length);
    }
    
    if (strlen(htdig_parms->max_doc_size) > 0)
    {
        config->Add ("max_doc_size", htdig_parms->max_doc_size);
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
    
    //if(strlen(htdig_parms->exclude_urls) > 0)
    //    tmpList.Create(htdig_parms->exclude_urls," \t");
    //else
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
    
    //if(strlen(htdig_parms->bad_querystr) > 0)
    //    tmpList.Create(htdig_parms->bad_querystr, " \t");
    //else
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
    //if(strlen(htdig_parms->limit_urls_to) > 0)
    //    l.Create(htdig_parms->limit_urls_to, " \t");
    //else
    //    l.Create(config->Find ("limit_urls_to"), " \t");
        
    limits.setEscaped (l, config->Boolean ("case_sensitive"));

    l.Destroy ();

    //if(strlen(htdig_parms->limit_normalized) > 0)
    //    l.Create (htdig_parms->limit_normalized, " \t");
    //else
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
    
    //if(strlen(htdig_parms->search_restrict) > 0)
    //    temp = htdig_parms->search_restrict;
    //else
        temp = config->Find("restrict");
    
    if (temp.length())
	{
	    // Create a temporary list from either the configuration
	    // file or the input parameter
	    StringList l(temp, " \t\r\n\001|");
	    limitTo.setEscaped(l);
	}
    
    //if(strlen(htdig_parms->search_exclude) > 0)
    //    temp = htdig_parms->search_exclude;
    //else
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


