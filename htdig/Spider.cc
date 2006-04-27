//
// Spider.cc
//
// Spider - Description TBD
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: Spider.cc,v 1.1.2.2 2006/04/27 00:48:12 aarnone Exp $
//



#include "Spider.h"



//
// function prototypes
//
static void sigexit(int);
static void sigpipe(int);
static void sig_handlers(void);
static void sig_phandler(void);
static void win32_check_messages(void);
static int noSignal;

//
// program-wide debug level
//
int debug;

//
// program-wide config file location
//
String configFile = DEFAULT_CONFIG_FILE;

//
// Cookie jar object - yet another ugly global!
//
static HtCookieJar* _cookie_jar;


//*****************************************************************************
// Spider::Spider()
//
Spider::Spider(htdig_parameters_struct * params)
{
    //
    // set the debug level. should probably be done first,
    // since many other functions may use it
    //
    debug = params->debug;

    if (debug)
    {
        cout << "ht://Dig Start Time: " << StartTime.GetAscTime() << endl;
    }

    //
    // setup the configuration. do this early, since other
    // things will depend on it (or override it)
    //
    setupConfigurationFile(params);

    //
    // some of these will override config values
    //
    setupSpiderFilters(params);

    //
    // the different log files
    //
    setupLogFiles(params);

    //
    // limits are used by isValidURL
    //
    setupLimitsList();

    //
    // determine if cookies are to be used
    //
    if(params->use_cookies || !config->Boolean("disable_cookies"))
    {
        setupCookieJar();
    }

    //
    // some checks to insure everything is hunky dory 
    //
    checkDeprecatedOptions();
    checkURLErrors();

    //
    // some retriever flags that are available from either 
    // the parameters or the configuration
    //
    report_statistics   = params->report_statistics;
    max_hop_count       = config->Value("max_hop_count", 999999);
    minimumWordLength   = config->Value("minimum_word_length", 3);
    check_unique_md5    = config->Boolean("check_unique_md5", 0);
    check_unique_date   = config->Boolean("check_unique_date", 0);
    local_urls_only     = config->Boolean("local_urls_only");
    mark_dead_servers   = config->Boolean("ignore_dead_servers");
    
    //
    // set up the wordtype static object (to be removed)
    //
    WordType::Initialize(*config);

    //
    // create the document object. it HAS to be done here 
    // because it relies on some of the setup configuration
    // that has just been initialized
    //
    doc = new Document;
}



//*****************************************************************************
// Spider::~Spider()
//
Spider::~Spider()
{
    //closeDBs();

    delete doc;
    
    if (_cookie_jar)
        delete _cookie_jar;

    if (urls_seen)
        fclose(urls_seen);
    if (images_seen)
        fclose(images_seen);
    logClose();

    if (debug)
    {
        EndTime.SettoNow();
        cout << "ht://Dig End Time: " << EndTime.GetAscTime() << endl;
    }
}



void Spider::openDBs(htdig_parameters_struct * params)
{
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
    // also overrride the DB dir if requested
    //
    if(strlen(params->DBpath) > 0)
    {
        config->Add("database_dir", params->DBpath);
    }

    //
    // if an alternate work area is asked for, append .work to the
    // database filenames (CLucene's name is changed in the actuall
    // openIndex call)
    //
    if (params->alt_work_area)
    {
        String configValue = config->Find("doc_index");
        if (configValue.length() != 0)
        {
            configValue << ".work";
            config->Add("doc_index", configValue);
        }
    }

    //
    // open the indexDB
    // 
    const String index_filename = config->Find("doc_index");
    if (params->initial)
    {
        if (debug)
            cout << "Deleting old indexDB...";
        unlink(index_filename);
    }
    if (debug)
        cout << "Opening indexDB here: " << index_filename.get() << endl;
    indexDatabase.Open(index_filename);

    
    //
    // open the CLucene database (not an object because we're using the API).
    // if there's an alt work area needed, append .work to the drectory name
    // 
    const String db_dir_filename = config->Find("database_dir");
    if (!params->alt_work_area) 
    {
        if (debug)
            cout << "Opening CLucene database here: " << db_dir_filename.get() << endl;
        CLuceneOpenIndex(form("%s/CLuceneDB", (char *)db_dir_filename.get()), params->initial ? 1 : 0, &stopWords);
    }
    else
    {
        if (debug)
            cout << "Opening CLucene database here: " << db_dir_filename.get() << endl;
        CLuceneOpenIndex(form("%s/CLuceneDB.work", (char *)db_dir_filename.get()), params->initial ? 1 : 0, &stopWords);

    }
}




void Spider::closeDBs()
{
    // Close the indexDB
    indexDatabase.Close();

    // Close the CLucene index
    CLuceneCloseIndex();

    if (report_statistics)
        ReportStatistics("htdig");
}


void Spider::setupConfigurationFile(htdig_parameters_struct * params)
{
    config = HtConfiguration::config();

    //
    // load default configuration
    // 
    config->Defaults(&defaults[0]);

    //
    // override defaults from file
    // 
    if (strlen(params->configFile) > 0)
    {
        configFile = params->configFile;

        if (access((char*)configFile, R_OK) < 0)
        {
            if (debug)
            {
                cout << "Unable to find configuration file '" << configFile.get() << "'" << endl;
            }
            reportError(form("Unable to find configuration file '%s'", configFile.get()));
        }
        else
        {
            config->Read(configFile);
        }
    }
}

void Spider::setupSpiderFilters(htdig_parameters_struct * params)
{
    if(strlen(params->locale) > 0)
    {
        config->Add("locale", params->locale);
    }

    if (config->Find ("locale").empty () && debug > 0)
    {
        cout << "Warning: unknown locale!" << endl;
        //logEntry("Warning: unknown locale!\n");
    }

    if (strlen(params->max_hop_count) > 0)
    {
        config->Add ("max_hop_count", params->max_hop_count);
    }

    if (strlen(params->max_head_length) > 0)
    {
        config->Add ("max_head_length", params->max_head_length);
    }
    
    if (strlen(params->max_doc_size) > 0)
    {
        config->Add ("max_doc_size", params->max_doc_size);
    }

    if(strlen(params->limit_urls_to) > 0)
    {
        config->Add("limit_urls_to", params->limit_urls_to);
    }
    
    if(strlen(params->limit_normalized) > 0)
    {
        config->Add("limit_normalized", params->limit_normalized);
    }

    if(strlen(params->exclude_urls) > 0)
    {
        config->Add("exclude_urls", params->exclude_urls);
    }
    
    if(strlen(params->url_rewrite_rules) > 0)
    {
        config->Add("url_rewrite_rules", params->url_rewrite_rules);
    }
    
    if(strlen(params->bad_querystr) > 0)
    {
        config->Add("bad_querystr", params->bad_querystr);
    }

    if(strlen(params->credentials) > 0)
    {
        config->Add("authorization", credentials);
    }

}

void Spider::initializeQueue(htdig_parameters_struct * params)
{
    //
    // figure out which URLs are going to be put
    // in the queue and from where.
    //
    // the logic of the initialization:
    //
    // params->URL != NULL
    //   yes:
    //     read URLs from params->URL
    //   no:
    //     read URLs from indexDB
    //     read config("url_log") URLs
    //     add config("start_url")
    //

    if(strlen(params->URL) > 0)
    {
        //
        // there are URLs specified, index these 
        // instead of doing a usual dig
        //
        String str;
        char * temp_URL_list = strdup(params->URL);
        char * temp_url = strtok(temp_URL_list, URL_SEPCHARS);
        while (temp_url != NULL)
        {
            str = temp_url;
            str.chop ("\r\n");
            if (str.length () > 0)
                Initial (str, 1);

            temp_url = strtok(NULL, URL_SEPCHARS);
        }
        free(temp_URL_list);
    }
    else
    {
        //
        // no URLs are specified. dig as normal. the IndexDB
        // and url_log might have been wiped already, so it
        // might just be the start_url. the order is important,
        // since the Initial() functions depend on it.
        //

        //
        // add the URLs from the url_log
        //
        FILE *urls_parsed;
        String filelog = config->Find("url_log");
        char buffer[1024];

        urls_parsed = fopen((char *) filelog, "r");
        if (urls_parsed != 0)
        {
            // read all url discovered but not fetched before 
            while (fgets(buffer, sizeof(buffer), urls_parsed))
            {
                buffer[strlen(buffer) - 1] = 0;
                Initial(buffer, 2);
            }
            fclose(urls_parsed);
        }
        unlink((char *) filelog);

        //
        // insert all URLs from the indexDB
        //
        List *list = indexDatabase.URLs();
        Initial(*list);
        delete list;

        // 
        // Add start_url to the initial list of the retriever.
        // Don't check a URL twice!
        // 
        Initial(config->Find("start_url"), 1);
    }
}


void Spider::setupLogFiles(htdig_parameters_struct * params)
{
    urls_seen = NULL;
    images_seen = NULL;

    //
    // clear out the url_log, so it can be filled from this dig
    //
    if (params->initial)
    {
       unlink(config->Find("url_log"));
    }

    //
    // create the error/dig log.
    //
    if(strlen(params->logFile) > 0)
    {
        logOpen(params->logFile);
    }

    //
    // If needed, we will create a list of every URL we come across.
    //
    if (config->Boolean("create_url_list"))
    {
        const String filename = config->Find("url_list");
        urls_seen = fopen(filename, params->initial ? "w" : "a");
        if (urls_seen == 0)
        {
            if (debug)
            {
                cout << "Unable to create URL file " << filename.get() << endl;
            }
            reportError(form("Unable to create URL file '%s'", filename.get()));
        }
    }

    //
    // If needed, we will create a list of every image we come across.
    //
    if (config->Boolean("create_image_list"))
    {
        const String filename = config->Find("image_list");
        images_seen = fopen(filename, params->initial ? "w" : "a");
        if (images_seen == 0)
        {
            if (debug)
            {
                cout << "Unable to create images file " << filename.get() << endl;
            }
            reportError(form("Unable to create images file '%s'", filename.get()));
        }
    }
}



void Spider::setupLimitsList()
{
    StringList l(config->Find("limit_urls_to"), " \t");
    limits.setEscaped(l, config->Boolean("case_sensitive"));
    l.Destroy();

    l.Create(config->Find("limit_normalized"), " \t");
    limitsn.setEscaped(l, config->Boolean("case_sensitive"));
    l.Destroy();
}



void Spider::setupCookieJar()
{
    _cookie_jar = new HtCookieMemJar();
    if (_cookie_jar)
    {
       HtHTTP::SetCookieJar(_cookie_jar);
    }

    // Imports the cookies file
    const String CookiesInputFile = config->Find("cookies_input_file");
    if (CookiesInputFile.length())
    {
        if (debug)
            cout << "Importing Cookies input file " << CookiesInputFile << endl;

        int result;
        HtCookieJar::SetDebugLevel(debug); // Set the debug level
        HtCookieInFileJar* cookie_file = new HtCookieInFileJar(CookiesInputFile, result);
        if (cookie_file)
        {
            if (!result)
            {
                if (debug>0)
                    cookie_file->ShowSummary();
                delete _cookie_jar;                         // Deletes previous cookie jar
                _cookie_jar = (HtCookieJar*) cookie_file;   // set the imported one
                HtHTTP::SetCookieJar(_cookie_jar);          // and set the new HTTP jar
            }
            else if (debug > 0)
                cout << "Warning: Import failed! (" << CookiesInputFile << ")" << endl;
        }
        else
        {
            if (debug)
            {
                cout << "Unable to load cookies file " << CookiesInputFile.get() << endl;
            }
            reportError(form("Unable to load cookies file '%s' in memory", CookiesInputFile.get()));
        }
    }
}


void Spider::checkDeprecatedOptions()
{
    // Warn user if any obsolete options are found in config file
    // For efficiency, check all fields here.  If different config
    // files are used for searching, obsolete options may remain
    char *deprecatedOptions [] = {
        "heading_factor_1", "heading_factor_2", "heading_factor_3",
        "heading_factor_4", "heading_factor_5", "heading_factor_6",
        "modification_time_is_now", "pdf_parser", "translate_amp",
        "translate_lt_gt", "translate_quot", "uncoded_db_compatible",
        ""	// empty terminator
    };
    char **option;
    for (option = deprecatedOptions; **option; option++)
    {
        if (!config->Find(*option).empty())
        {
            cout << "Warning: Configuration option " << *option << " is no longer supported\n";
        }
    }
}


void Spider::checkURLErrors()
{
    String url_part_errors = HtURLCodec::instance()->ErrMsg();
    if (url_part_errors.length() != 0)
    {
        if (debug)
        {
            cout << "Invalid url_part_aliases or common_url_parts: " << url_part_errors.get() << endl;
        }
        reportError(form("Invalid url_part_aliases or common_url_parts: %s", url_part_errors.get()));
    }

    String url_rewrite_rules = HtURLRewriter::instance()->ErrMsg();
    if (url_rewrite_rules.length() != 0)
    {
        if (debug)
        {
            cout << "Invalid url_rewrite_rules: " << url_rewrite_rules.get() << endl;
        }
        reportError(form("Invalid url_rewrite_rules: %s", url_rewrite_rules.get()));
    }
}




//*****************************************************************************
// void Spider::setUsernamePassword(char *credentials)
//
void Spider::setUsernamePassword(const char *credentials)
{
    doc->setUsernamePassword(credentials);
}


//*****************************************************************************
// void Spider::Initial(char *list, int from)
//   Add a single URL to the list of URLs to visit.
//   Since URLs are stored on a per server basis, we first need to find the
//   the correct server to add the URL's path to.
//
//   from == 0 urls in db.docs and no db.log
//   from == 1 urls in start_url add url only if not already in the list 
//   from == 2 add url from db.log 
//   from == 3 urls in db.docs and there was a db.log 
//
void Spider::Initial(const String & list, int from)
{
    //
    // Split the list of urls up into individual urls.
    //
    StringList tokens(list, " \t");
    String sig;
    String url;
    Server *server;

    for (int i = 0; i < tokens.Count(); i++)
    {
        URL u(tokens[i]);
        url = u.get();	// get before u.signature() resolves aliases
        server = (Server *) servers[u.signature()];
        if (debug > 2)
            cout << "\t" << from << ":" << url;
        if (!server)
        {
            String robotsURL = u.signature();
            robotsURL << "robots.txt";
            StringList *localRobotsFile = GetLocal(robotsURL);

            server = new Server(u, localRobotsFile);
            servers.Add(u.signature(), server);
            delete localRobotsFile;
        }

        if (from && visited.Exists(url))
        {
            if (debug > 2)
                cout << " skipped" << endl;
            continue;
        }
        else if (IsValidURL(url) != 1)
        {
            if (debug > 2)
                cout << endl;
            continue;
        }

        if (from != 3)
        {
            if (debug > 2)
                cout << " pushed";
            server->push(u.get(), 0, 0, IsLocalURL(url.get()));
		}
        if (debug > 2)
            cout << endl;
        visited.Add(url, 0);
    }
}


//*****************************************************************************
// void Spider::Initial(List &list, int from)
//
void Spider::Initial(List & list, int from)
{
    list.Start_Get();
    String *str;

    //
    // from == 0 is an optimisation for pushing url in update mode
    //  assuming that 
    // 1) there's many more urls in docdb 
    // 2) they're pushed first
    // 3) there's no duplicate url in docdb
    // then they don't need to be check against already pushed urls
    // But 2) can be false with -l option
    //
    // FIXME it's nasty, what have to be test is :
    // we have urls to push from db.docs but do we already have them in
    // db.log? For this it's using a side effect with 'visited' and that
    // urls in db.docs are only pushed via this method, and that db.log are pushed
    // first, db.docs second, start_urls third!
    //  
    if (!from && visited.Count())
    {
        from = 3;
    }
    while ((str = (String *) list.Get_Next()))
    {
        Initial(str->get(), from);
    }
}



//*****************************************************************************
// void Spider::Start()
//   This is the main loop of the retriever.  We will go through the
//   list of paths stored for each server.  While parsing the
//   retrieved documents, new paths will be added to the servers.  We
//   return if no more paths need to be retrieved.
//
void Spider::Start(htdig_parameters_struct * params)
{
    //
    // Main digger loop.  The todo list should initialy have the start
    // URL and all the URLs which were seen in a previous dig.  The
    // loop will continue as long as there are more URLs to visit.
    //
    int more = 1;
    Server *server;
    URLRef *ref;
    indexCount = 0;
    currenthopcount = 0;

    //
    // initialize the spider queue.
    //
    initializeQueue(params);

    HtConfiguration *config = HtConfiguration::config();

    //  
    // Always sig. The delay bothers me but a bad db is worse
    // 
    sig_handlers();
    sig_phandler();
    noSignal = 1;


    //
    // Main loop. We keep on retrieving until a signal is received
    // or all the servers' queues are empty.
    //    

    win32_check_messages();

    while (more && noSignal)
    {
        more = 0;

        //
        // Go through all the current servers in sequence.
        // If they support persistent connections, we keep on popping
        // from the same server queue until it's empty or we reach a maximum
        // number of consecutive requests ("max_connection_requests").
        // Or the loop may also continue for the infinite,
        // if we set the "max_connection_requests" to -1.
        // If the server doesn't support persistent connection, we take
        // only an URL from it, then we skip to the next server.
        //
        // Since 15.05.02: even when persistent connections are activated
        // we should wait for a 'server_wait_time' number of seconds
        // after the 'max_connection_requests' value has been reached.
        //

        // Let's position at the beginning
        servers.Start_Get();

        int count;

        // Maximum number of repeated requests with the same
        // TCP connection (so on the same Server:Port).

        int max_connection_requests;

        win32_check_messages();

        while ((server = (Server *) servers.Get_NextElement()) && noSignal)
        {
            if (debug > 1)
                cout << "pick: " << server->host() << ", # servers = " << servers.Count() << endl;

            // We already know if a server supports HTTP pers. connections,
            // because we asked it for the robots.txt file (constructor of
            // the class).

            // If the Server doesn't support persistent connections
            // we turn it down to 1.

            if (server->IsPersistentConnectionAllowed())
            {

                // Let's check for a '0' value (out of range)
                // If set, we change it to 1.

                if (config->Value("server", server->host(), "max_connection_requests") == 0)
                    max_connection_requests = 1;
                else
                    max_connection_requests =
                        config->Value("server", server->host(), "max_connection_requests");

                if (debug > 2)
                {

                    cout << "> " << server->host() << " supports HTTP persistent connections";

                    if (max_connection_requests == -1)
                        cout << " (" << "infinite" << ")" << endl;
                    else
                        cout << " (" << max_connection_requests << ")" << endl;

                }

            }
            else
            {

                // No HTTP persistent connections. So we request only 1 document.

                max_connection_requests = 1;

                if (debug > 2)
                    cout << "> " << server->host() << " with a traditional HTTP connection" << endl;

			}


            count = 0;

            win32_check_messages();

            // Loop until interrupted, too many requests, or done.
            // ('noSignal' must be before 'pop', or the popped
            // URL will not be included in  url_log  file, below.)
            while (noSignal && ((max_connection_requests == -1) ||
              (count < max_connection_requests)) && (ref = server->pop()))
            {
                count++;

                //
                // We have a URL to index, now.  We need to register the
                // fact that we are not done yet by setting the 'more'
                // variable. So, we have to restart scanning the queue.
                //

                more = 1;

                //
                // Deal with the actual URL.
                // We'll check with the server to see if we need to sleep()
                // before parsing it.
                //

                parse_url(*ref);
                delete ref;

                // We reached the maximum number of connections (either with
                // or without persistent connections) and we must pause and
                // respect the 'net ethic'.
                if ((max_connection_requests - count) == 0)
                    server->delay();    // This will pause if needed
                // and reset the time

                win32_check_messages();
            }
            win32_check_messages();
        }
    }
    win32_check_messages();


    //
    // if we exited on signal, try to clean up
    //   close the DBs
    //   log the URLs not yet parsed
    //
    if (!noSignal)
    {
        closeDBs();

        String filelog = config->Find("url_log");
        FILE *urls_parsed = fopen((char *) filelog, "w");

        if (urls_parsed == 0)
        {
            cout << "Unable to create URL log file '" << filelog.get() << "'" << endl;
            reportError(form("Unable to create URL log file '%s'", filelog.get()));
        }
        else
        {
            servers.Start_Get();
            while ((server = (Server *) servers.Get_NextElement()))
            {
                while (NULL != (ref = server->pop()))
                {
                    fprintf(urls_parsed, "%s\n", (const char *) ref->GetURL().get());
                    delete ref;
                }
            }
            fclose(urls_parsed);
        }
    }
}


// -------------------------------------------------------
//
// add a simple document - no HTML parsing is required
//
void Spider::addSingleDoc(singleDoc * newDoc, time_t docTime, int spiderable)
{
    //
    // get teh old odcument information out of the indexDB 
    // 
    indexDoc = indexDatabase.Exists((*newDoc)["url"].c_str());

    if (indexDoc)
    {
        if (docTime < indexDoc->DocTime())
        {
            //
            // the document hasn't changed, so just return
            //
            delete indexDoc;
            return;
        }

        //
        // erase old document from CLucene (by URL)
        // 
        CLuceneDeleteURLFromIndex(&(*newDoc)["url"]);

        //
        // erase old document from indexDB (by URL)
        //
        indexDatabase.Delete(indexDoc->DocURL());
    }
    delete indexDoc;

    if (spiderable)
    {
        //
        // set up the urlRef
        //
        URLRef * urlRef = new URLRef;
        String tempURLstring = (*newDoc)["url"].c_str(); 
        urlRef->SetURL(tempURLstring);

        if (retrieveDoc(*urlRef, docTime) != Transport::Document_ok)
        {
            //
            // awww... the documents was marked spiderable, but wasn't retrievable
            //
            return;
        }
    }

    //
    // set up the indexDoc
    //
    indexDoc = new IndexDBRef;

    indexDoc->DocURL((*newDoc)["url"].c_str());
    if(spiderable)
    {
        indexDoc->DocSize(doc->Length());
     
        //
        // external parsing ???????
        //
    }
    else
    {
        indexDoc->DocSize((*newDoc)["contents"].length());
    }
    indexDoc->DocTime(docTime);
    indexDoc->DocSpiderable(spiderable);
    // hopcount is irrelevant
    // backlink count is irrelevant




    //
    // a new pointer to a Clucene document
    //
    CLuceneDoc = new DocumentRef;

    //
    // the title and meta description need to be added to the CLucene
    // document. however, these might also be discovered during parsing.
    // therefore, wait until after parsing and then replace these
    // parsed-out fields. stemmed and synonm fields are treated as normal.
    //
    CLuceneDoc->appendField("stemmed", (*newDoc)["title"].c_str());
    CLuceneDoc->appendField("synonym", (*newDoc)["title"].c_str());

    CLuceneDoc->appendField("stemmed", (*newDoc)["meta-desc"].c_str());
    CLuceneDoc->appendField("synonym", (*newDoc)["meta-desc"].c_str());

    //
    // initialize the parser. the encoding is unknown (or at least not
    // specified), so just use the default: utf8
    //
    tparser.initialize(CLuceneDoc, NULL);

    //
    // parse the actual document. the URLs seen can be ignored, as
    // can the noIndex flag (obviously someone wants it in the index)
    //
    if (spiderable)
    {
        parseDoc(doc->Contents(), false);
    }
    else
    {
        parseDoc((char*)(*newDoc)["contents"].c_str(), false);
    }

    // 
    // move the parsed-out title and meta description to the keywords field,
    // and replace with the simpleDoc fields (but only if the fields exist)
    //
    if ((*newDoc)["title"].length())
    {
        CLuceneDoc->appendField("keywords", CLuceneDoc->getField("title"));

        CLuceneDoc->insertField("title", (*newDoc)["title"].c_str());
        CLuceneDoc->insertField("doc-title", (*newDoc)["title"].c_str());
    }

    //
    // if an alternate meta description was specified, append it to the field
    //
    if ((*newDoc)["meta-desc"].length())
    {
        CLuceneDoc->appendField("keywords", CLuceneDoc->getField("meta-desc"));

        CLuceneDoc->appendField("meta-desc", (*newDoc)["meta-desc"].c_str());
        CLuceneDoc->appendField("doc-meta-desc", (*newDoc)["meta-desc"].c_str());
    }

    commitDoc();

    delete CLuceneDoc;
    delete indexDoc;
}




//*****************************************************************************
// void Spider::parse_url(URLRef &urlRef)
//
void Spider::parse_url(URLRef & urlRef)
{
    URL url;
    bool old_document;

    url.parse(urlRef.GetURL().get());

    currenthopcount = urlRef.GetHopCount();
    currentServer = NULL;

    if (debug > 0)
    {
        // Display progress
        cout << indexCount++ << currenthopcount << ':' << url.get() << ": ";
        cout.flush();
    }

    // 
    // search for the URL in the indexDB
    // 
    indexDoc = indexDatabase.Exists(url.get());
    
    if (indexDoc)
    {
        //
        // if the document is marked as not spiderable, don't even try
        //
        if (indexDoc->DocSpiderable() == 0)
        {
            if (debug)
            {
                cout << " marked not spiderable" << endl;
            }
            delete indexDoc;
            return;
        }

        //
        // We already have an entry for this document in our database.
        //
        old_document = true;

        //
        // this can VASTLY overinflate the backcount number... this
        // should be tied to the actual backlink URLs
        //
        indexDoc->DocBacklinks(indexDoc->DocBacklinks() + 1);

        //
        // adjust the hopcount
        //
        if (currenthopcount < indexDoc->DocHopCount())
        {
            indexDoc->DocHopCount(currenthopcount);
        }
        else
        {
            currenthopcount = indexDoc->DocHopCount();
        }
    }
    else
    {
        //
        // Never seen this document before.  We need to create a
        // new IndexDBRef for it
        //
        old_document = false;

        indexDoc = new IndexDBRef;
        indexDoc->DocURL(url.get());
        indexDoc->DocTime(0);
        indexDoc->DocHopCount(currenthopcount);
        indexDoc->DocBacklinks(1);
        indexDoc->DocSpiderable(1);
    }

    Transport::DocStatus status = retrieveDoc(urlRef, indexDoc->DocTime());

    //
    // Determine what to do by looking at the status code returned by
    // the Document retrieval process.
    //
    switch (status)
    {

    case Transport::Document_ok:

        if (old_document)
        {
            //
            // Since we already had a record of this document and
            // we were able to retrieve it, it must have changed
            // since the last time we scanned it. So...
            //

            //
            // erase old document from CLucene (by URL)
            // 
            CLuceneDeleteURLFromIndex( new string(indexDoc->DocURL().get()) );

            //
            // erase old document from indexDB (by URL)
            //
            indexDatabase.Delete(indexDoc->DocURL());

            if (debug)
            {
                cout << " (changed) ";
            }
        }
        indexDoc->DocSize(doc->Length());
        indexDoc->DocTime(doc->ModTime());

        //
        // external parsing using doc->ContentType() or doc->getParsable()
        //

        //
        // create the CLuceneDoc. since this is a standard
        // HTML document, there's no need for special field
        // inserts or anything.
        //
        CLuceneDoc = new DocumentRef;

        //
        // parse the contents of the retrieved doc. by now, they should be
        // converted by external parsers, too
        //
        if (parseDoc(doc->Contents()))
        {
            //
            // Sucess! insert the documents
            //
            commitDoc(); 

            if (debug)
            {
                cout << " size = " << indexDoc->DocSize() << endl;
            }

            if (urls_seen)
            {
                fprintf(urls_seen, "%s|%d|%s|%d|%d|1\n",
                        (indexDoc->DocURL()).get(), indexDoc->DocSize(),
                        doc->ContentType(), (int)indexDoc->DocTime(), currenthopcount);
            }
        }
        else if (debug)
        {
            //
            // if the document was marked no index, then don't insert into
            // either database to keep them in sync
            //
            cout << "marked 'noindex'" << endl;
        }
        delete CLuceneDoc;

        break;

    case Transport::Document_not_changed:
        if (debug)
            cout << " not changed" << endl;
        break;

    case Transport::Document_not_found:
        if (debug)
            cout << " not found" << endl;
        recordNotFound(url.get(), urlRef.GetReferer().get(), Transport::Document_not_found);
        break;

    case Transport::Document_no_host:
        if (debug)
            cout << " host not found" << endl;
        recordNotFound(url.get(), urlRef.GetReferer().get(), Transport::Document_no_host);

        // Mark the current server as being down
        if (currentServer && mark_dead_servers)
            currentServer->IsDead(1);
        break;

    case Transport::Document_no_port:
        if (debug)
            cout << " host not found (port)" << endl;
        recordNotFound(url.get(), urlRef.GetReferer().get(), Transport::Document_no_port);

        // Mark the current server as being down
        if (currentServer && mark_dead_servers)
            currentServer->IsDead(1);
        break;

    case Transport::Document_not_parsable:
        if (debug)
            cout << " not Parsable" << endl;
        break;

    case Transport::Document_redirect:
        if (debug)
            cout << " redirect" << endl;
        got_redirect(doc->Redirected(), (urlRef.GetReferer()).get());
        break;

    case Transport::Document_not_authorized:
        if (debug)
            cout << " not authorized" << endl;
        break;

    case Transport::Document_not_local:
        if (debug)
            cout << " not local" << endl;
        break;

    case Transport::Document_no_header:
        if (debug)
            cout << " no header" << endl;
        break;

    case Transport::Document_connection_down:
        if (debug)
            cout << " connection down" << endl;
        break;

    case Transport::Document_no_connection:
        if (debug)
            cout << " no connection" << endl;
        break;

    case Transport::Document_not_recognized_service:
        if (debug)
            cout << " service not recognized" << endl;

        // Mark the current server as being down
        if (currentServer && mark_dead_servers)
            currentServer->IsDead(1);
        break;

    case Transport::Document_other_error:
        if (debug)
            cout << " other error" << endl;
        break;
	}
    delete indexDoc;
}

// -----------------------------------------------------------
//
// Pre:  - the urlRef is filled correctly
//       - the date is what should be used for the "get if newer"
//
// Post: - the doc contents may have been filled, and the return
//           status is appropriate
//       - the base URL has been set
//
Transport::DocStatus Spider::retrieveDoc(URLRef & urlRef, time_t date)
{
    URL url;
    url.parse(urlRef.GetURL().get());

    // 
    // Reset the document to clean out any old data
    // 
    doc->Reset();
    doc->Url(url.get());
    doc->Referer(urlRef.GetReferer().get());

    //
    // Set the base URL
    // 
    base = doc->Url();

    // 
    // Retrieve document, first trying local file access if possible.
    //
    Transport::DocStatus status;
    currentServer = (Server *)servers[url.signature()];
    StringList *local_filenames = GetLocal(url.get());
    if (local_filenames)
    {
        if (debug > 1)
            cout << "Trying local files" << endl;
        status = doc->RetrieveLocal(date, local_filenames);
        if (status == Transport::Document_not_local)
        {
            if (currentServer && !currentServer->IsDead() && !local_urls_only)
                status = doc->Retrieve(currentServer, date);
            else
                status = Transport::Document_no_host;
            if (debug > 1)
                cout << "Local retrieval failed, trying HTTP" << endl;
        }
        delete local_filenames;
    }
    else if (currentServer && !currentServer->IsDead() && !local_urls_only)
        status = doc->Retrieve(currentServer, date);
    else
        status = Transport::Document_no_host;

    return(status);
}

// ------------------------------------------------------------
//
// Pre:  - indexDoc has ALL of its fields filled
//       - CLucene document has been initialized, and any important text 
//           has been inserted into the appropriate fields - some exceptions: 
//           size, time and URL will be added here
//       - contents contains the document in parsable form (external parsing is done)
//       - follow set to whether the URLs found in parsing are to be followed or not
//
// Post: - doc contents have been translated to html with external parsers
//       - doc contents have been parsed by HTMLTidy into the CLucene document
//       - the indexDoc time might have been updated after parsing
//       - All URLs from parsing have been added to the queue
//       - indexDoc and CLuceneDoc are ready to be inserted into the DBs (though
//           further modification is possible)
//
// Return: TRUE if documents should be indexed, FALSE if not
//
bool Spider::parseDoc(char * contents, bool follow)
{
    //
    // initialize the parser with the CLucene doc and
    // with the encoding type 
    //
    tparser.initialize(CLuceneDoc, NULL);
    //tparser.initialize(CLuceneDoc, doc->ContentType());

    //
    // add the time to the CLucene doc
    //
    char tempTime[32];
    sprintf(tempTime, "%d", (int)indexDoc->DocTime());
    CLuceneDoc->insertField("doc-time", tempTime);

    //
    // add the document size to the CLucene doc
    //
    char tempSize[32];
    sprintf(tempSize, "%d", indexDoc->DocSize());
    CLuceneDoc->insertField("doc-size", tempSize);

    //
    // Add the URL to the CLucene doc
    //
    CLuceneDoc->insertField("url", (indexDoc->DocURL()).get());

    //
    // parse the actual document, recieving the parsed
    // urls in return
    //
    set<string> URLlist = tparser.parseDoc(contents);

    //tparser.commitDoc();
    
    //
    // now that parsing is finished, the 'true' time is
    // known (the time might have been specified in a meta tag)
    // 
    char* tempTime2 = CLuceneDoc->getField("doc-time");
    time_t t = atoi(tempTime2);
    indexDoc->DocTime(mktime(gmtime(&t)));
    free(tempTime2);

    //
    // add all the URLs seen to the queue (if there are any), 
    // but only if follow says to
    //
    if (follow)
    {
        set<string>::iterator i;
        for (i = URLlist.begin(); i != URLlist.end(); i++)
        {
            // 
            // create a new URL object with the current URL as its parent
            // 
            URL * tempURL = new URL((*i).c_str(), *base);
            addURL(tempURL);
            delete tempURL;
        }
    }

    return !tparser.NoIndex();
}



// ----------------------------------------------------------
//
// Pre:  - both documents are ready to insert
//
// Post: - both documents have been inserted
//
void Spider::commitDoc()
{
    //
    // Insert the index document into indexDB
    //
    indexDatabase.Add(*indexDoc);

    //
    // insert the document into CLucene 
    //
    CLuceneAddDocToIndex(CLuceneDoc->contents());
}



//*****************************************************************************
// int Spider::Need2Get(const String &u)
//   Return TRUE if we need to retrieve the given url.  This will
//   check the list of urls we have already visited.
//
int Spider::Need2Get(const String & u)
{
    static String url;
    url = u;

    return !visited.Exists(url);
}



//*****************************************************************************
// int Spider::IsValidURL(const String &u)
//   Return TRUE if we need to retrieve the given url.  We will check
//   for limits here.
//
int Spider::IsValidURL(const String & u)
{
	HtConfiguration *config = HtConfiguration::config();
	Dictionary invalids;
	Dictionary valids;
	URL aUrl(u);
	StringList tmpList;

	// A list of bad extensions, separated by spaces or tabs
	String t = config->Find(&aUrl, "bad_extensions");
	String lowerp;
	char *p = strtok(t, " \t");
	while (p)
	{
		// Extensions are case insensitive
		lowerp = p;
		lowerp.lowercase();
		invalids.Add(lowerp, 0);
		p = strtok(0, " \t");
	}

	//
	// Valid extensions are performed similarly 
	//
	// A list of valid extensions, separated by spaces or tabs

	t = config->Find(&aUrl, "valid_extensions");
	p = strtok(t, " \t");
	while (p)
	{
		// Extensions are case insensitive
		lowerp = p;
		lowerp.lowercase();
		valids.Add(lowerp, 0);
		p = strtok(0, " \t");
	}

	static String url;
	url = u;

	//
	// If the URL contains any of the patterns in the exclude list,
	// mark it as invalid
	//
	String exclude_urls = config->Find(&aUrl, "exclude_urls");
	static String *prevexcludes = 0;
	static HtRegexList *excludes = 0;
	if (!excludes || !prevexcludes || prevexcludes->compare(exclude_urls) != 0)
	{
		if (!excludes)
			excludes = new HtRegexList;
		if (prevexcludes)
			delete prevexcludes;
		prevexcludes = new String(exclude_urls);
		tmpList.Create(exclude_urls, " \t");
		excludes->setEscaped(tmpList, config->Boolean("case_sensitive"));
		tmpList.Destroy();
	}
	if (excludes->match(url, 0, 0) != 0)
	{
		if (debug > 2)
			cout << endl << "   Rejected: item in exclude list ";
		return (HTDIG_ERROR_TESTURL_EXCLUDE);
	}

	//
	// If the URL has a query string and it is in the bad query list
	// mark it as invalid
	//
	String bad_querystr = config->Find(&aUrl, "bad_querystr");
	static String *prevbadquerystr = 0;
	static HtRegexList *badquerystr = 0;
	if (!badquerystr || !prevbadquerystr || prevbadquerystr->compare(bad_querystr) != 0)
	{
		if (!badquerystr)
			badquerystr = new HtRegexList;
		if (prevbadquerystr)
			delete prevbadquerystr;
		prevbadquerystr = new String(bad_querystr);
		tmpList.Create(bad_querystr, " \t");
		badquerystr->setEscaped(tmpList, config->Boolean("case_sensitive"));
		tmpList.Destroy();
	}
	char *ext = strrchr((char *) url, '?');
	if (ext && badquerystr->match(ext, 0, 0) != 0)
	{
		if (debug > 2)
			cout << endl << "   Rejected: item in bad query list ";
		return (HTDIG_ERROR_TESTURL_BADQUERY);
	}

	//
	// See if the file extension is in the list of invalid ones
	//
	String urlpath = url.get();
	int parm = urlpath.indexOf('?');	// chop off URL parameter
	if (parm >= 0)
		urlpath.chop(urlpath.length() - parm);
	ext = strrchr((char *) urlpath.get(), '.');
	String lowerext;
	if (ext && strchr(ext, '/'))	// Ignore a dot if it's not in the
		ext = NULL;		  // final component of the path.
	if (ext)
	{
		lowerext.set(ext);
		lowerext.lowercase();
		if (invalids.Exists(lowerext))
		{
			if (debug > 2)
				cout << endl << "   Rejected: Extension is invalid!";
			return (HTDIG_ERROR_TESTURL_EXTENSION);
		}
	}
	//
	// Or NOT in the list of valid ones
	//
	if (ext && valids.Count() > 0 && !valids.Exists(lowerext))
	{
		if (debug > 2)
			cout << endl << "   Rejected: Extension is not valid!";
		return (HTDIG_ERROR_TESTURL_EXTENSION2);
	}

	//
	// If none of the limits is met, we disallow the URL
	//
	if (limits.match(url, 1, 0) == 0)
	{
		if (debug > 1)
			cout << endl << "   Rejected: URL not in the limits! ";
		return (HTDIG_ERROR_TESTURL_LIMITS);
	}
	//
	// Likewise if not in list of normalized urls
	//
	// Warning!
	// should be last in checks because of aUrl normalization
	//
		// signature()  implicitly normalizes the URL.  Be efficient...
	Server *server = (Server *) servers[aUrl.signature()];
//	aUrl.normalize();
	if (limitsn.match(aUrl.get(), 1, 0) == 0)
	{
		if (debug > 2)
			cout << endl << "   Rejected: not in \"limit_normalized\" list!";
		return (HTDIG_ERROR_TESTURL_LIMITSNORM);
	}

	//
	// After that gauntlet, check to see if the server allows it
	// (robots.txt)
	//
	if (server && server->IsDisallowed(url) != 0)
	{
		if (debug > 2)
			cout << endl << "   Rejected: forbidden by server robots.txt!";
		return (HTDIG_ERROR_TESTURL_ROBOT_FORBID);
	}

	return (1);
}


//*****************************************************************************
// StringList* Spider::GetLocal(const String &url)
//   Returns a list of strings containing the (possible) local filenames
//   of the given url, or 0 if it's definitely not local.
//   THE CALLER MUST FREE THE STRINGLIST AFTER USE!
//   Returned strings are not hex encoded.
//
StringList *Spider::GetLocal(const String & strurl)
{
	HtConfiguration *config = HtConfiguration::config();
	static StringList *prefixes = 0;
	String url = strurl;

	static StringList *paths = 0;
	StringList *defaultdocs = 0;
	URL aUrl(url);
	url = aUrl.get();		  // make sure we look at a parsed URL

	//
	// Initialize prefix/path list if this is the first time.
	// The list is given in format "prefix1=path1 prefix2=path2 ..."
	//
	if (!prefixes)
	{
		prefixes = new StringList();
		paths = new StringList();

		String t = config->Find("local_urls");
		char *p = strtok(t, " \t");
		while (p)
		{
			char *path = strchr(p, '=');
			if (!path)
			{
				p = strtok(0, " \t");
				continue;
			}
			*path++ = '\0';
			String *pre = new String(p);
			decodeURL(*pre);
			prefixes->Add(pre);
			String *pat = new String(path);
			decodeURL(*pat);
			paths->Add(pat);
			p = strtok(0, " \t");
		}
	}
	if (!config->Find(&aUrl, "local_default_doc").empty())
	{
		defaultdocs = new StringList();
		String t = config->Find(&aUrl, "local_default_doc");
		char *p = strtok(t, " \t");
		while (p)
		{
			String *def = new String(p);
			decodeURL(*def);
			defaultdocs->Add(def);
			p = strtok(0, " \t");
		}
		if (defaultdocs->Count() == 0)
		{
			delete defaultdocs;
			defaultdocs = 0;
		}
	}

	// Begin by hex-decoding URL...
	String hexurl = url;
	decodeURL(hexurl);
	url = hexurl.get();

	// Check first for local user...
	if (strchr(url.get(), '~'))
	{
		StringList *local = GetLocalUser(url, defaultdocs);
		if (local)
		{
			if (defaultdocs)
				delete defaultdocs;
			return local;
		}
	}

	// This shouldn't happen, but check anyway...
	if (strstr(url.get(), ".."))
		return 0;

	String *prefix, *path;
	String *defaultdoc;
	StringList *local_names = new StringList();
	prefixes->Start_Get();
	paths->Start_Get();
	while ((prefix = (String *) prefixes->Get_Next()))
	{
		path = (String *) paths->Get_Next();
		if (mystrncasecmp((char *) *prefix, (char *) url, prefix->length()) == 0)
		{
			int l = strlen(url.get()) - prefix->length() + path->length() + 4;
			String *local = new String(*path, l);
			*local += &url[prefix->length()];
			if (local->last() == '/' && defaultdocs)
			{
				defaultdocs->Start_Get();
				while ((defaultdoc = (String *) defaultdocs->Get_Next()))
				{
					String *localdefault =
						new String(*local, local->length() + defaultdoc->length() + 1);
					localdefault->append(*defaultdoc);
					local_names->Add(localdefault);
				}
				delete local;
			}
			else
				local_names->Add(local);
		}
	}
	if (local_names->Count() > 0)
	{
		if (defaultdocs)
			delete defaultdocs;
		return local_names;
	}

	if (defaultdocs)
		delete defaultdocs;
	delete local_names;
	return 0;
}


//*****************************************************************************
// StringList* Spider::GetLocalUser(const String &url, StringList *defaultdocs)
//   If the URL has ~user part, return a list of strings containing the
//   (possible) local filenames of the given url, or 0 if it's
//   definitely not local.
//   THE CALLER MUST FREE THE STRINGLIST AFTER USE!
//
StringList *Spider::GetLocalUser(const String & url, StringList * defaultdocs)
{
//  NOTE:  Native Windows does not have this contruct for the user Web files
#ifndef _MSC_VER /* _WIN32 */
	HtConfiguration *config = HtConfiguration::config();
	static StringList *prefixes = 0, *paths = 0, *dirs = 0;
	static Dictionary home_cache;
	URL aUrl(url);

	//
	// Initialize prefix/path list if this is the first time.
	// The list is given in format "prefix1=path1,dir1 ..."
	// If path is zero-length, user's home directory is looked up. 
	//
	if (!prefixes)
	{
		prefixes = new StringList();
		paths = new StringList();
		dirs = new StringList();
		String t = config->Find("local_user_urls");
		char *p = strtok(t, " \t");
		while (p)
		{
			char *path = strchr(p, '=');
			if (!path)
			{
				p = strtok(0, " \t");
				continue;
			}
			*path++ = '\0';
			char *dir = strchr(path, ',');
			if (!dir)
			{
				p = strtok(0, " \t");
				continue;
			}
			*dir++ = '\0';
			String *pre = new String(p);
			decodeURL(*pre);
			prefixes->Add(pre);
			String *pat = new String(path);
			decodeURL(*pat);
			paths->Add(pat);
			String *ptd = new String(dir);
			decodeURL(*ptd);
			dirs->Add(ptd);
			p = strtok(0, " \t");
		}
	}

	// Can we do anything about this?
	if (!strchr(url, '~') || !prefixes->Count() || strstr(url, ".."))
		return 0;

	// Split the URL to components
	String tmp = url;
	char *name = strchr((char *) tmp, '~');
	*name++ = '\0';
	char *rest = strchr(name, '/');
	if (!rest || (rest - name <= 1) || (rest - name > 32))
		return 0;
	*rest++ = '\0';

	// Look it up in the prefix/path/dir table
	prefixes->Start_Get();
	paths->Start_Get();
	dirs->Start_Get();
	String *prefix, *path, *dir;
	String *defaultdoc;
	StringList *local_names = new StringList();
	while ((prefix = (String *) prefixes->Get_Next()))
	{
		path = (String *) paths->Get_Next();
		dir = (String *) dirs->Get_Next();
		if (mystrcasecmp((char *) *prefix, (char *) tmp) != 0)
			continue;

		String *local = new String;
		// No path, look up home directory
		if (path->length() == 0)
		{
			String *home = (String *) home_cache[name];
			if (!home)
			{
				struct passwd *passwd = getpwnam(name);
				if (passwd)
				{
					home = new String(passwd->pw_dir);
					home_cache.Add(name, home);
				}
			}
			if (home)
				*local += *home;
			else
				continue;
		}
		else
		{
			*local += *path;
			*local += name;
		}
		*local += *dir;
		*local += rest;
		if (local->last() == '/' && defaultdocs)
		{
			defaultdocs->Start_Get();
			while ((defaultdoc = (String *) defaultdocs->Get_Next()))
			{
				String *localdefault = new String(*local, local->length() + defaultdoc->length() + 1);
				localdefault->append(*defaultdoc);
				local_names->Add(localdefault);
			}
			delete local;
		}
		else
			local_names->Add(local);
	}

	if (local_names->Count() > 0)
		return local_names;

	delete local_names;
#endif //_MSC_VER /* _WIN32 */

    return 0;
}


//*****************************************************************************
// int Spider::IsLocalURL(const String &url)
//   Returns 1 if the given url has a (possible) local filename
//   or 0 if it's definitely not local.
//
int Spider::IsLocalURL(const String & url)
{
	int ret;

	StringList *local_filename = GetLocal(url);
	ret = (local_filename != 0);
	if (local_filename)
		delete local_filename;

	return ret;
}

//******************************************************************************
// void Spider::addURL(URL * url)
//   add a URL to the queue
//
void Spider::addURL(URL * url)
{
    Server *server = 0;
    int valid_url_code = 0;

    // Rewrite the URL (if need be) before we do anything to it.
    url->rewrite();

    if (debug > 2)
        cout << "new href: " << url->get() << " (" << "no description" << ')' << endl;

#ifndef LIBHTDIG
    if (urls_seen)
        fprintf(urls_seen, "%s\n", (const char *) url->get());
#endif

    //
    // Check if this URL falls within the valid range of URLs.
    //
    valid_url_code = IsValidURL(url->get());
    if (valid_url_code > 0)
    {
        //
        // It is valid.  Normalize it (resolve cnames for
        // the server) and check again...
        //
        if (debug > 2)
        {
            cout << "resolving '" << url->get() << "'" << endl;
        }

        url->normalize();

        // 
        // If it is a backlink from the current document,
        // just update that field.  Writing to the database
        // is meaningless, as it will be overwritten.
        // 
        // Adding it as a new document may even be harmful, as
        // that will be a duplicate.  This can happen if the
        // current document is never referenced before, as in a
        // start_url.
        // 


        if (strcmp(url->get(), (indexDoc->DocURL()).get()) == 0)
        {
            //
            // we can still update the backlink count and descriptions
            // 

// Anthony - AddDescription is gone from DocumentRef for right now
//            indexDoc->DocBacklinks(indexDoc->DocBacklinks() + 1);
//            indexDoc->AddDescription(description, words);
        }
        else
        {
            //
            // put it in the list of URLs to still visit.
            //
            if (Need2Get(url->get())) {
                if (debug > 1)
                {
                    cout << "\n   pushing " << url->get() << endl;
                }
                
                server = (Server *) servers[url->signature()];

                if (!server)
                {
                    //
                    // Hadn't seen this server, yet.  Register it
                    //
                    String robotsURL = url->signature();
                    robotsURL << "robots.txt";
                    StringList *localRobotsFile = GetLocal(robotsURL.get());

                    server = new Server(*url, localRobotsFile);
                    servers.Add(url->signature(), server);
                    delete localRobotsFile;
                }

                //
                // Let's just be sure we're not pushing an empty URL
                //
				if (strlen(url->get()))
                {
                    server->push(url->get(), currenthopcount+1, base->get(), IsLocalURL(url->get()));
                }

                String temp = url->get();
                visited.Add(temp, 0);

                if (debug)
                {
                    cout << '+';
                }
            }
			else if (debug)
            {
                cout << '*';
            }
        }
    }
    else
    {
        //
        // Not a valid URL
        //
        if (debug > 1)
            cout << "\nurl rejected: (level 1)" << url->get() << endl;
        if (debug == 1)
            cout << '-';

        if (urls_seen)
        {
            fprintf(urls_seen, "%s|||||%d\n", (const char *) url->get(), valid_url_code);
        }
    }
    if (debug)
        cout.flush();
}


//*****************************************************************************
// void Spider::got_redirect(const char *new_url, DocumentRef *old_ref)
//
void Spider::got_redirect(const char *new_url, const char *referer)
{
    //
    // First we must piece together the new URL, which may be relative
    // 
    URL parent(indexDoc->DocURL());
    URL url(new_url, parent);

    // Rewrite the URL (if need be) before we do anything to it.
    url.rewrite();

    if (debug > 2)
    {
        cout << "redirect: " << url.get() << endl;
    }

    if (urls_seen)
    {
        fprintf(urls_seen, "%s\n", (const char *) url.get());
    }

    //
    // Check if this URL falls within the valid range of URLs.
    //
    if (IsValidURL(url.get()) > 0)
    {
        //
        // It is valid.  Normalize it (resolve cnames for the server)
        // and check again...
        //
        if (debug > 2)
        {
            cout << "resolving '" << url.get() << "'" << endl;
        }

        url.normalize();
        //
        // Now put it in the list of URLs to still visit.
        //
        if (Need2Get(url.get()))
        {
            if (debug > 1)
                cout << "   pushing " << url.get() << endl;
            Server *server = (Server *) servers[url.signature()];
            if (!server)
            {
                //
                // Hadn't seen this server, yet.  Register it
                //
                String robotsURL = url.signature();
                robotsURL << "robots.txt";
                StringList *localRobotsFile = GetLocal(robotsURL.get());

                server = new Server(url, localRobotsFile);
                servers.Add(url.signature(), server);
                delete localRobotsFile;
            }

            if (!referer || strlen(referer) == 0)
            {
                server->push(url.get(), currenthopcount, base->get(), IsLocalURL(url.get()), 0);
            }
            else
            {
                server->push(url.get(), currenthopcount, referer, IsLocalURL(url.get()), 0);
            }

            String temp = url.get();
            visited.Add(temp, 0);
        }
    }
}



//*****************************************************************************
//
void Spider::recordNotFound(const String & url, const String & referer, int reason)
{
	char *message = "";

	switch (reason)
	{
	case Transport::Document_not_found:
		message = "Not found";
		break;

	case Transport::Document_no_host:
		message = "Unknown host or unable to contact server";
		break;

	case Transport::Document_no_port:
		message = "Unknown host or unable to contact server (port)";
		break;

	default:
		break;

	}

	notFound << message << ": " << url << " Ref: " << referer << '\n';
}



//*****************************************************************************
// void Spider::ReportStatistics(char *name)
//
void Spider::ReportStatistics(const String & name)
{
	HtConfiguration *config = HtConfiguration::config();
	cout << name << ": Run complete\n";
	cout << name << ": " << servers.Count() << " server";
	if (servers.Count() > 1)
		cout << "s";
	cout << " seen:\n";

	Server *server;
	String buffer;
	StringList results;
	String newname = name;

	newname << ":    ";

	servers.Start_Get();
	while ((server = (Server *) servers.Get_NextElement()))
	{
		buffer = 0;
		server->reportStatistics(buffer, newname);
		results.Add(buffer);
	}
	results.Sort();

	for (int i = 0; i < results.Count(); i++)
	{
		cout << results[i] << "\n";
	}

	if (notFound.length() > 0)
	{
		cout << "\n" << name << ": Errors to take note of:\n";
		cout << notFound;
	}

	cout << endl;

	// Report HTTP connections stats
	cout << "HTTP statistics" << endl;
	cout << "===============" << endl;

	if (config->Boolean("persistent_connections"))
	{
		cout << " Persistent connections    : Yes" << endl;

		if (config->Boolean("head_before_get"))
			cout << " HEAD call before GET      : Yes" << endl;
		else
			cout << " HEAD call before GET      : No" << endl;
	}
	else
	{
		cout << " Persistent connections    : No" << endl;
	}

	HtHTTP::ShowStatistics(cout);

    cout << "===============" << endl;
}

/*
// *****************************************************************************
// Place a log entry into the error log
//
void logEntry (char *msg)
{
    if(error_log != NULL)
    {
        time_t now = time(NULL);
    	fprintf(error_log, "[%s] %s\n", ctime(&now), msg);
    }

}


// *****************************************************************************
// Report an error to the error log
//
void reportError (char *msg)
{
    if(error_log != NULL)
    {
        time_t now = time(NULL);
    	fprintf(error_log, "%s  [ERROR] %s\n", ctime(&now), msg);
    }
}

*/

static void sigexit(int)
{
    noSignal = 0;   //don't exit here.. just set the flag.
}

static void sigpipe(int)
{
}

//*****************************************************************************
// static void sig_handlers
//   initialise signal handlers
//
static void sig_handlers(void)
{
#ifndef _MSC_VER /* _WIN32 */
    //POSIX SIGNALS
    struct sigaction action;

	/* SIGINT, SIGQUIT, SIGTERM */
    action.sa_handler = sigexit;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGINT, &action, NULL) != 0)
        reportError("Cannot install SIGINT handler\n");
    if (sigaction(SIGABRT, &action, NULL) != 0)
        reportError("Cannot install SIGINT handler\n");
    if (sigaction(SIGQUIT, &action, NULL) != 0)
        reportError("Cannot install SIGQUIT handler\n");
    if (sigaction(SIGTERM, &action, NULL) != 0)
        reportError("Cannot install SIGTERM handler\n");
    if (sigaction(SIGHUP, &action, NULL) != 0)
        reportError("Cannot install SIGHUP handler\n");
#else
    //ANSI C signal handling - Limited to supported Windows signals.
    signal(SIGINT, sigexit); 
    signal(SIGTERM, sigexit); 
#endif //_MSC_VER /* _WIN32 */
}



static void sig_phandler(void)
{
#ifndef _MSC_VER /* _WIN32 */
    struct sigaction action;

    sigemptyset(&action.sa_mask);
    action.sa_handler = sigpipe;
    action.sa_flags = SA_RESTART;
    if (sigaction(SIGPIPE, &action, NULL) != 0)
    {
        cout << "Cannot install SIGPIPE handler" << endl;
        reportError("Cannot install SIGPIPE handler\n");
    }
#endif //_MSC_VER /* _WIN32 */
}


//*****************************************************************************
// static void win32_check_messages
//   Check WIN32 messages!
//
static void win32_check_messages(void)
{
#ifdef _MSC_VER /* _WIN32 */
// NEAL - NEEDS FINISHING/TESTING
#if 0
    MSG msg = {0, 0, 0, 0};
    int cDown = 0;
    int controlDown = 0;

    if( GetMessage(&msg, 0, 0, 0) )
    {

        switch(msg.message)
        {
            case WM_KEYDOWN:
                {
                    if(LOWORD(msg.message)== 17)
                        controlDown = 1;
                    else if(LOWORD(msg.message) == 67)
                    {
                        cDown = 1;
                    }
                }
                break;
            case WM_KEYUP:
                {
                    if(LOWORD(msg.message) == 17)
                        controlDown = 0;
                    else if(LOWORD(msg.message) == 67)
                        cDown = 0;
                }
                break;
        }
    }

    DispatchMessage(&msg);
#endif
#endif //_MSC_VER /* _WIN32 */
}




