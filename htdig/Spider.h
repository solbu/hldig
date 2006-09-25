//
// Spider.h
//
// Spider: Description TBD
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: Spider.h,v 1.1.2.2 2006/09/25 23:05:41 aarnone Exp $
//

#ifndef _Spider_h_
#define _Spider_h_

#ifdef HAVE_CONFIG_H
  #include "htconfig.h"
#endif //HAVE_CONFIG_H

#include "HtConfiguration.h"
#include "HtDebug.h"
#include "defaults.h"

#include "HtStdHeader.h"
#include "libhtdig_api.h"
#include "List.h"
#include "TidyParser.h"
#include "Dictionary.h"
#include "HtDateTime.h"
#include "WordType.h"
#include "HtRegexList.h"
#include "IndexDBRef.h"
#include "IndexDB.h"
#include "Document.h"
#include "DocumentRef.h"
#include "CLuceneAPI.h"

#include "HtDateTime.h"
#include "HtURLRewriter.h"
#include "HtURLCodec.h"

////////////////////////////
// For cookie jar
////////////////////////////
#include "HtCookieJar.h"
#include "HtCookieMemJar.h"
#include "HtCookieInFileJar.h"
#include "HtHTTP.h"
////////////////////////////

extern String configFile;

class Spider
{
    public:
    //
    // Construction/Destruction
    //
    Spider(htdig_parameters_struct * params);
    ~Spider();

    //
    // index open/close operations
    //
    void        openDBs(htdig_parameters_struct * params);
    void        closeDBs();

    //
    // used by the singnal handling function to kill the spider
    // gracefully. will fill url_log with unvisited URLs
    //
    void        interruptClose(int);

    //
    // Getting it all started
    //
    void        Start(htdig_parameters_struct * params);

    //
    // Index a single document
    //
    int        addSingleDoc(singleDoc * newDoc, time_t altTime, int spiderable, bool addToSpiderQueue);

    //
    // Fetch a single document from its URL. return a single doc
    //
    singleDoc*  fetchSingleDoc(string * url);

    //
    // delete a document based on its URL
    //
    int         DeleteDoc(string * input);

    //
    // delete a document based on its doc-id
    //
    int         DeleteDoc(int input);

    private:

    //
    // constructor helper functions
    //
    void        setupConfigurationFile(htdig_parameters_struct * params);
    void        setupSpiderFilters(htdig_parameters_struct * params);
    void        initializeQueue(htdig_parameters_struct * params);
    void        setupLogFiles(htdig_parameters_struct * params);
    void        setupLimitsList();
    void        setupCookieJar();
    void        checkDeprecatedOptions();
    void        checkURLErrors();
    void        Initial(const String& url, int from = 0);
    void        Initial(List &list , int from = 0);



    //
    // retrieve the document, based on time. return retrieval status
    //
    Transport::DocStatus retrieveDoc(URLRef & urlRef, time_t date);

    //
    // parse the CLucene document, return whether noIndex was found
    //
    bool        parseDoc(char * contents, bool follow = true);

    //
    // commit both documents
    //
    void        commitDocs();

    //
    // Report statistics about the parser
    //
    void        ReportStatistics(const String& name);
	
    //
    // Allow for the indexing of protected sites by using a
    // username/password
    //
    void        setUsernamePassword(const char *credentials);

    //
    // add a URL to the queue
    //
    void            addURL(URL * url);
    
    //
    // More helper routines
    //
    int             Need2Get (const String &url);
    int             IsValidURL (const String &url);
    void            parse_url (URLRef &urlRef);
    void            got_redirect (const char *, const char * = 0);
    void            recordNotFound (const String &url, const String &referer, int reason);

    //
    // Routines for dealing with local filesystem access
    //
    StringList *    GetLocal(const String &strurl);
    StringList *    GetLocalUser(const String &url, StringList *defaultdocs);
    int             IsLocalURL(const String &url);



    TidyParser      tparser;        // TidyParser object

    IndexDBRef      * indexDoc;     // reference object to current index database entry

    DocumentRef     * CLuceneDoc;   // reference to the current CLucene doc

    IndexDB         indexDatabase;  // the index database

    Document		* doc;          // the raw document, which will retrieve itself



    //
    // Some more variables
    //
    int             max_hop_count;
    int             check_unique_md5;
    int             check_unique_date;
    int             minimumWordLength;
    int             local_urls_only;
    int             mark_dead_servers;
    int             die_on_timer;
    bool            DBsOpen;
 
    HtConfiguration *config;
    Server          *currentServer;
    URL             *base;

    int             currenthopcount;
    String          credentials;
    int             indexCount; // current count of documents seen


    //
    // A hash to keep track of what we've seen
    //
    Dictionary      visited;
   
    //
    // logging variables/objects
    //  
    HtDateTime      StartTime;
    HtDateTime      EndTime;
    FILE            *urls_seen;
    FILE            *images_seen;
    int             report_statistics;

    //
    // URL limits
    //
    HtRegexList     limits;
    HtRegexList     limitsn;

    //
    // The list of server-specific information objects
    // is indexed by ip address and port number.
    // The list contains Server objects.
    //
    Dictionary      servers;

    //
    // used in the statistics report
    //
    String          notFound;

    void sig_handlers(int, int);
    void sig_phandler(void);
    void win32_check_messages(void);


};

#endif // _Spider_h_


