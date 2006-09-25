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
// $Id: Spider.cc,v 1.1.2.3 2006/09/25 23:05:41 aarnone Exp $
//



#include "Spider.h"

HtDebug * debug;

//
// Global signal handling functions and Spider pointer.
// The function(s) will use the Spider pointer to call
// member functions of a specific Spider instance. the
// Spider object pointer should not be used elsewhere 
// (to maintain program stability).
//
void sigpipe(int);
void sigexit(int);
static Spider * interruptHandlerSpiderPointer = NULL;

//
// program-wide config file location
//
String configFile = DEFAULT_CONFIG_FILE;

//
// Cookie jar object - yet another ugly global!
//
static HtCookieJar* _cookie_jar = NULL;


//*****************************************************************************
// Spider::Spider()
//

Spider::Spider(htdig_parameters_struct * params)
{
    //
    // set up the debug output. should probably be done first,
    // since many other functions may use it
    //
    debug = HtDebug::Instance();
    if (strlen(params->debugFile) > 0)
    {
        //
        // if the debug output file was specified, then set it up. also, kill stdout output.
        //
        if (debug->setLogfile(params->debugFile) == false)
        {
            cout << "HtDig: Error opening debug output file ["<< params->debugFile << "]" << endl;
            cout << "Turning on stdout debugging" << endl;
            debug->setStdoutLevel(params->debug);
        }
        else
        {
            debug->setFileLevel(params->debug);
        }
    }
    else
    {
        debug->setStdoutLevel(params->debug);
    }

    DBsOpen = false;

    doc = NULL;
    indexDoc = NULL;
    CLuceneDoc = NULL;
    _cookie_jar = NULL;
    config = NULL;

    debug->outlog(0, "ht://Dig Start Time: %s\n", StartTime.GetAscTime());

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
    if (doc != NULL)
    {
        delete doc;
        doc = NULL;
    }
    doc = new Document;

    //  
    // Always sig. The delay bothers me but a bad db is worse
    // 
    //sig_handlers(config->Value("index_timeout"), config->Value("index_timeout_precise"));
    //sig_phandler();
}



//*****************************************************************************
// Spider::~Spider()
//
Spider::~Spider()
{
    if (doc != NULL)
    {
        delete doc;
        doc = NULL;
    }
    
    if (DBsOpen)
    {
        closeDBs();
    }

    config = NULL;

    //
    // the spider is about to die, so get rid of the the this pointer
    //
    interruptHandlerSpiderPointer = NULL;

    if (_cookie_jar != NULL)
    {
        delete _cookie_jar;
        _cookie_jar = NULL;
    }

    if (urls_seen)
        fclose(urls_seen);
    if (images_seen)
        fclose(images_seen);

    if (report_statistics)
        ReportStatistics("htdig");

    EndTime.SettoNow();
    debug->outlog(0, "ht://Dig End Time: %s\n", EndTime.GetAscTime());
    debug->close();
}



void Spider::openDBs(htdig_parameters_struct * params)
{
    if(DBsOpen)
    {
        return;
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
            debug->outlog(4, "Stopwords from [%s]:\n", stopWordsFilename.c_str());

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
            debug->outlog(0, "Unable to open stop word file\n");
        }
    }
    else
    {
        debug->outlog(1, "Stop word file not specified, using default CLucene stop words\n");
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
        debug->outlog(0, "Deleting old indexDB\n");
        unlink(index_filename);
    }
    debug->outlog(0, "Opening indexDB here: %s\n", index_filename.get());
    indexDatabase.Open(index_filename);

    
    //
    // open the CLucene database (not an object because we're using the API).
    // if there's an alt work area needed, append .work to the drectory name
    // 
    const String db_dir_filename = config->Find("database_dir");
    if (!params->alt_work_area) 
    {
        debug->outlog(0, "Opening CLucene database here: %s\n", db_dir_filename.get());
        CLuceneOpenIndex(form("%s/CLuceneDB", (char *)db_dir_filename.get()), params->initial ? 1 : 0, &stopWords);
    }
    else
    {
        debug->outlog(0, "Opening CLucene database (working copy) here: %s\n", db_dir_filename.get());
        CLuceneOpenIndex(form("%s/CLuceneDB.work", (char *)db_dir_filename.get()), params->initial ? 1 : 0, &stopWords);

    }

    DBsOpen = true;
}




void Spider::closeDBs()
{
    if (DBsOpen)
    {
        // Close the indexDB
        indexDatabase.Close();

        // Close the CLucene index
        CLuceneCloseIndex();

        DBsOpen = false;
    }
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
            debug->outlog(0, "Unable to find configuration file [%s]\n", configFile.get());
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

    if (config->Find ("locale").empty ())
    {
        debug->outlog(0, "Warning: unknown locale!\n");
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
        config->Add("url_list", params->logFile);
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
            debug->outlog(0, "Unable to create URL file [%s]\n", filename.get());
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
            debug->outlog(0, "Unable to create images file [%s]\n", filename.get());
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
    if (_cookie_jar != NULL)
    {
        delete _cookie_jar;
        _cookie_jar = NULL;
    }
    _cookie_jar = new HtCookieMemJar();
    if (_cookie_jar)
    {
       HtHTTP::SetCookieJar(_cookie_jar);
    }

    // Imports the cookies file
    const String CookiesInputFile = config->Find("cookies_input_file");
    if (CookiesInputFile.length())
    {
        debug->outlog(0, "Importing Cookies input file [%s]", CookiesInputFile.get());

        int result;
        //HtCookieJar::SetDebugLevel(debug->getLevel()); // Set the debug level
        HtCookieInFileJar* cookie_file = new HtCookieInFileJar(CookiesInputFile, result);
        if (cookie_file)
        {
            if (!result)
            {
                if (debug->getLevel() > 0)
                    cookie_file->ShowSummary();
                delete _cookie_jar;                         // Deletes previous cookie jar
                _cookie_jar = (HtCookieJar*) cookie_file;   // set the imported one
                HtHTTP::SetCookieJar(_cookie_jar);          // and set the new HTTP jar
            }
            else
            {
                debug->outlog(0, "Warning: Import failed from [%s]\n", CookiesInputFile.get());
            }
        }
        else
        {
            debug->outlog(0, "Unable to load cookies file [%s]\n", CookiesInputFile.get());
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
        debug->outlog(0, "Invalid url_part_aliases or common_url_parts: %s\n", url_part_errors.get());
    }

    String url_rewrite_rules = HtURLRewriter::instance()->ErrMsg();
    if (url_rewrite_rules.length() != 0)
    {
        debug->outlog(0, "Invalid url_rewrite_rules: %s\n", url_rewrite_rules.get());
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
    Server * server = NULL;

    for (int i = 0; i < tokens.Count(); i++)
    {
        URL u(tokens[i]);
        url = u.get();	// get before u.signature() resolves aliases
        server = (Server *) servers[u.signature()];
        debug->outlog(2, "\t%d:%s", from, url.get());

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
            debug->outlog(2, " skipped\n");
            continue;
        }
        else if (IsValidURL(url) != 1)
        {
            debug->outlog(2, "\n");
            continue;
        }

        if (from != 3)
        {
            debug->outlog(2, " pushed");
            server->push(u.get(), 0, 0, IsLocalURL(url.get()));
		}
        debug->outlog(2, "\n");
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
    // Main loop. We keep on retrieving until a signal is received
    // or all the servers' queues are empty.
    //    

    win32_check_messages();

    while (more)
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

        while ((server = (Server *) servers.Get_NextElement()))
        {
            debug->outlog(1, "pick: %s, # servers = %d\n", server->host().get(), servers.Count());

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

                if (debug->getLevel() > 2)
                {

                    debug->outlog(2, "> %s supports HTTP persistent connections", server->host().get());

                    if (max_connection_requests == -1)
                        debug->outlog(2, " (infinite)\n");
                    else
                        debug->outlog(2," (%d)\n", max_connection_requests);

                }

            }
            else
            {

                // No HTTP persistent connections. So we request only 1 document.

                max_connection_requests = 1;

                debug->outlog(2, "> %s with a traditional HTTP connection\n", server->host().get());

			}


            count = 0;

            win32_check_messages();

            // Loop until interrupted, too many requests, or done.
            // ('noSignal' must be before 'pop', or the popped
            // URL will not be included in  url_log  file, below.)
            while (
                    //noSignal
                    //&&
                    ((max_connection_requests == -1) || (count < max_connection_requests))
                    &&
                    (ref = server->pop())
                  )
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

                //
                // We reached the maximum number of connections (either with
                // or without persistent connections) and we must pause and
                // respect the 'net ethic'.
                //
                if ((max_connection_requests - count) == 0)
                    server->delay();    // This will pause if needed
                // and reset the time

                win32_check_messages();
            }
            win32_check_messages();
        }
    }
    win32_check_messages();
}



singleDoc * Spider::fetchSingleDoc(string * url)
{
    Transport::DocStatus status = Transport::Document_ok;
    String tempURLString = url->c_str();

    debug->outlog(2, "Attempting to fetch ");

    do {
        Server * server = NULL;
        URL * tempURL = new URL(tempURLString);
        URLRef * tempURLRef = new URLRef;
        tempURLRef->SetURL(*tempURL);

        debug->outlog(2, "%s\n", tempURLRef->GetURL().get().get());

        server = (Server *) servers[tempURL->signature()];
        if (!server)
        {
            String robotsURL = tempURL->signature();
            robotsURL << "robots.txt";
            StringList *localRobotsFile = GetLocal(robotsURL);

            //
            // create a new server. the ownership of these objects pass to the
            // 'servers' Dictionary object, so no deleting!
            //
            server = new Server(*tempURL, localRobotsFile);
            servers.Add(tempURL->signature(), server);

            delete localRobotsFile;
        }

        //
        // now that everything is set up, try to retrieve the actual
        // document. since we definitely want this, send a time of zero
        // to the retriever, so it gets fetched every time.
        //
        status = retrieveDoc(*tempURLRef, 0);

        if (status == Transport::Document_redirect)
        {
            tempURLString = doc->Redirected();
            debug->outlog(2, " (redirect) \n");
        }

        delete tempURL;
        delete tempURLRef;

    } while (status == Transport::Document_redirect);

    //
    // redirects are all done, and the retrieval was either a success or not
    //
    if (status != Transport::Document_ok)
    {
        //
        // awww... the documents was marked spiderable, but wasn't retrievable
        //
        debug->outlog(2, " FAIL.\n");
        return 0;
    }
    debug->outlog(2, " success.\n");

    //
    // a new pointer to a Clucene document
    //
    if (CLuceneDoc != NULL)
    {
        delete CLuceneDoc;
        CLuceneDoc = NULL;
    }
    CLuceneDoc = new DocumentRef;

    //
    // this will be neccessary so parseDoc doesn't explode. however,
    // the whoe object can be thrown away after parsing
    //
    if (indexDoc != NULL)
    {
        delete indexDoc;
        indexDoc = NULL;
    }
    indexDoc = new IndexDBRef;

    //
    // parse the actual document. the URLs seen can be ignored, as
    // can the noIndex flag (the noIndex flag is the return value 
    // of parseDoc)
    //
    config->Add("use_stemming", "false");
    config->Add("use_synonyms", "false");
    
    debug->outlog(2, "Parse initializing\n");
    parseDoc(doc->Contents(), false);
    debug->outlog(2, "Parse complete\n");

    singleDoc * newDoc = new singleDoc;

    //
    // get the URL fromt he tempURLString, since it will
    // be the final URL after redirects happen
    //
    (*newDoc)["url"] = tempURLString.get();

    char * temp;

    temp = CLuceneDoc->getField("title");
    (*newDoc)["title"] = temp;
    free(temp);

    temp = CLuceneDoc->getField("meta-desc");
    (*newDoc)["meta-desc"] = temp;
    free(temp);

    temp = CLuceneDoc->getField("keywords");
    (*newDoc)["meta-desc"] += temp;
    free(temp);

    temp = CLuceneDoc->getField("contents");
    (*newDoc)["contents"] = temp;
    free(temp);

    char tempTime[32];
    sprintf(tempTime, "%d", (int)doc->ModTime());
    (*newDoc)["doc-time"] = tempTime;

    (*newDoc)["content-type"] = doc->ContentType();

    if (CLuceneDoc != NULL)
    {
        delete CLuceneDoc;
        CLuceneDoc = NULL;
    }
    if (indexDoc != NULL)
    {
        delete indexDoc;
        indexDoc = NULL;
    }

    return newDoc;
}



// -------------------------------------------------------
//
// add a simple document - if the document is marked as spiderable, try
// to retrieve and parse it like a regular document. this requires 
// creating a server object (or more if redirects happen) and also
// a robots.txt retrieve, so this can be a bit slow. if the addToSpiderQueue flag
// is set to false, the spiderable flag will be set to false before inserting into
// the indexDB, so that this document will not be revisited on future spidering runs.
//
int Spider::addSingleDoc(singleDoc * newDoc, time_t altTime, int spiderable, bool addToSpiderQueue)
{
    bool needToDelete = false;

    debug->outlog(0, "Index Single: %s", (*newDoc)["url"].c_str());

    //
    // get the old odcument information out of the indexDB 
    // 
    if (indexDoc != NULL)
    {
        delete indexDoc;
        indexDoc = NULL;
    }
    indexDoc = indexDatabase.Exists((*newDoc)["url"].c_str());

    if (indexDoc)
    {
        //
        // if the alternate time given is greater (aka newer) than the
        // time stored in the indexDB, or if the alternate time stored in
        // the indexDB is zero, this document needs to be replaced 
        //
        if ( (altTime > indexDoc->DocAltTime()) || (indexDoc->DocAltTime() == 0) )
        {
            //
            // don't actualy delete until after retrieval and parsing are
            // done, in case something goes wrong
            //
            debug->outlog(0, " (changed)");
            needToDelete = true;
        }
        else
        {
            //
            // the document hasn't changed, so just return
            //
            debug->outlog(0, " (not changed)\n");
            delete indexDoc;
            indexDoc = NULL;
            return 1;
        }
    }
    delete indexDoc;
    indexDoc = NULL;


    if (spiderable)
    {
        //
        // try to spider the URL, while handling redirects
        //

        Transport::DocStatus status = Transport::Document_ok;
        String tempURLString = (*newDoc)["url"].c_str();

        do {
            Server * server;
            URL * tempURL = new URL(tempURLString);
            URLRef * tempURLRef = new URLRef;
            tempURLRef->SetURL(*tempURL);

            server = (Server *) servers[tempURL->signature()];
            //
            // if the server isn't already in the list, add it
            //
            if (!server)
            {
                String robotsURL = tempURL->signature();
                robotsURL << "robots.txt";
                StringList *localRobotsFile = GetLocal(robotsURL);

                //
                // create a new server. the ownership of these objects pass to the 'servers'
                // Dictionary object, so no deleting!
                //
                server = new Server(*tempURL, localRobotsFile);
                servers.Add(tempURL->signature(), server);

                delete localRobotsFile;
            }

            //
            // now that everything is set up, try to retrieve the actual document. since
            // this document was specifically requested with the addSingle call, use
            // zero as the time, so it will be retrieved every time.
            //
            status = retrieveDoc(*tempURLRef, 0);

            if (status == Transport::Document_redirect)
            {
                tempURLString = doc->Redirected();
                debug->outlog(2, "%s (redirect)\n", tempURLString.get());
            }

            delete tempURL;
            delete tempURLRef;

        } while (status == Transport::Document_redirect);

        //
        // redirects are all done, and the retrieval was either a success or not
        //
        if (status != Transport::Document_ok)
        {
            //
            // awww... the documents was marked spiderable, but wasn't retrievable
            //
            debug->outlog(2, " FAIL\n");
            return 0;
        }
        debug->outlog(2, " success\n");
        
    }
    else
    {
        //
        // retrieve doc usually does these steps, but since this
        // isn't spiderable, it has to be done manually
        //
        doc->Reset();
        doc->Url((*newDoc)["url"].c_str());
        debug->outlog(2, "Not Retrieving [%s]\n", (*newDoc)["url"].c_str());
    }

    //
    // set up the indexDoc
    //
    if (indexDoc != NULL)
    {
        delete indexDoc;
        indexDoc = NULL;
    }
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
    indexDoc->DocTime(doc->ModTime());
    indexDoc->DocAltTime(altTime);
    // hopcount is irrelevant (as in the default will be fine)
    // backlink count is irrelevant (as in the default will be fine)

    //
    // if the document is to be added to the queue for next time the regular spider
    // runs, then addToSpiderQueue and spiderable need to be set to true.
    //
    if (addToSpiderQueue && spiderable)
    {
        indexDoc->DocSpiderable(1);
    }
    else
    {
        indexDoc->DocSpiderable(0);
    }

    //
    // a new pointer to a Clucene document
    //
    if (CLuceneDoc != NULL)
    {
        delete CLuceneDoc;
        CLuceneDoc = NULL;
    }
    CLuceneDoc = new DocumentRef;

    //
    // the title and meta description need to be added to the CLucene
    // document. however, these might also be discovered during parsing.
    // therefore, wait until after parsing and then replace these
    // parsed-out fields. stemmed and synonm fields are treated as normal.
    //
    if (config->Boolean("use_stemming"))
    {
        CLuceneDoc->appendField("stemmed", (*newDoc)["title"].c_str());
        CLuceneDoc->appendField("stemmed", (*newDoc)["meta-desc"].c_str());
    }

    if (config->Boolean("use_synonyms"))
    {
        CLuceneDoc->appendField("synonym", (*newDoc)["meta-desc"].c_str());
        CLuceneDoc->appendField("synonym", (*newDoc)["title"].c_str());
    }

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

    //
    // Add the URL to the CLucene doc
    //
    CLuceneDoc->insertField("url", (*newDoc)["url"].c_str());

    //
    // add the document ID
    //
    CLuceneDoc->insertField("doc-id", (*newDoc)["id"].c_str());


    //
    // if the document is already in the DBs, erase it now that
    // the parsing is complete (success)
    //
    if (needToDelete)
    {
        DeleteDoc(&(*newDoc)["url"]);
    }

    //
    // commit the new document to both DBs
    //
    commitDocs();

    if (urls_seen)
    {
        fprintf(urls_seen, "%s|%d|%s|%d|%d|1\n",
                (indexDoc->DocURL()).get(), indexDoc->DocSize(),
                doc->ContentType(), (int)indexDoc->DocTime(), currenthopcount);
    }

    if (CLuceneDoc != NULL)
    {
        delete CLuceneDoc;
        CLuceneDoc = NULL;
    }
    if (indexDoc != NULL)
    {
        delete indexDoc;
        indexDoc = NULL;
    }

    return 1;
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

    // Display progress
    debug->outlog(0, "%d %d : %s :", indexCount++, currenthopcount, url.get().get());

    // 
    // search for the URL in the indexDB
    // 
    if (indexDoc != NULL)
    {
        delete indexDoc;
        indexDoc = NULL;
    }
    indexDoc = indexDatabase.Exists(url.get());
    
    if (indexDoc)
    {
        //
        // if the document is marked as not spiderable, don't even try
        //
        if (indexDoc->DocSpiderable() == 0)
        {
            debug->outlog(0, " (marked not spiderable)\n");
            if (indexDoc != NULL)
            {
                delete indexDoc;
                indexDoc = NULL;
            }
            return;
        }

        //
        // if the document has not yet expired, don't spider
        //
        if (indexDoc->DocExpired() > (int)time(NULL))
        {
            debug->outlog(0, " (not expired)\n");
            if (indexDoc != NULL)
            {
                delete indexDoc;
                indexDoc = NULL;
            }
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

        if (indexDoc != NULL)
        {
            delete indexDoc;
            indexDoc = NULL;
        }
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
            string * tempURL = new string(indexDoc->DocURL().get());
            DeleteDoc(tempURL);
            delete tempURL;


            debug->outlog(0, " (changed) ");
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
        if (CLuceneDoc != NULL)
        {
            delete CLuceneDoc;
            CLuceneDoc = NULL;
        }
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
            commitDocs(); 

            debug->outlog(0, " size = %d\n", indexDoc->DocSize());

            if (urls_seen)
            {
                fprintf(urls_seen, "%s|%d|%s|%d|%d|1\n",
                        (indexDoc->DocURL()).get(), indexDoc->DocSize(),
                        doc->ContentType(), (int)indexDoc->DocTime(), currenthopcount);
            }
        }
        else
        {
            //
            // if the document was marked no index, then don't insert into
            // either database to keep them in sync
            //
            debug->outlog(0, "marked 'noindex'\n");
        }
        if (CLuceneDoc != NULL)
        {
            delete CLuceneDoc;
            CLuceneDoc = NULL;
        }

        break;

    case Transport::Document_not_changed:
        debug->outlog(0, " not changed\n");
        break;

    case Transport::Document_not_found:
        debug->outlog(0, " not found\n");
        recordNotFound(url.get(), urlRef.GetReferer().get(), Transport::Document_not_found);
        break;

    case Transport::Document_no_host:
        debug->outlog(0, " host not found\n");
        recordNotFound(url.get(), urlRef.GetReferer().get(), Transport::Document_no_host);

        // Mark the current server as being down
        if (currentServer && mark_dead_servers)
            currentServer->IsDead(1);
        break;

    case Transport::Document_no_port:
        debug->outlog(0, " host not found (port)\n");
        recordNotFound(url.get(), urlRef.GetReferer().get(), Transport::Document_no_port);

        // Mark the current server as being down
        if (currentServer && mark_dead_servers)
            currentServer->IsDead(1);
        break;

    case Transport::Document_not_parsable:
        debug->outlog(0, " not Parsable\n");
        break;

    case Transport::Document_redirect:
        debug->outlog(0, " redirect");
        got_redirect(doc->Redirected(), (urlRef.GetReferer()).get());
        break;

    case Transport::Document_not_authorized:
        debug->outlog(0, " not authorized\n");
        break;

    case Transport::Document_not_local:
        debug->outlog(0, " not local\n");
        break;

    case Transport::Document_no_header:
        debug->outlog(0, " no header\n");
        break;

    case Transport::Document_connection_down:
        debug->outlog(0, " connection down\n");
        break;

    case Transport::Document_no_connection:
        debug->outlog(0, " no connection\n");
        break;

    case Transport::Document_not_recognized_service:
        debug->outlog(0, " service not recognized\n");

        // Mark the current server as being down
        if (currentServer && mark_dead_servers)
            currentServer->IsDead(1);
        break;

    case Transport::Document_other_error:
        debug->outlog(0, " other error\n");
        break;
	}
    if (indexDoc != NULL)
    {
        delete indexDoc;
        indexDoc = NULL;
    }
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
        debug->outlog(1, "Trying local files\n");
        status = doc->RetrieveLocal(date, local_filenames);
        if (status == Transport::Document_not_local)
        {
            if (currentServer && !currentServer->IsDead() && !local_urls_only)
                status = doc->Retrieve(currentServer, date);
            else
                status = Transport::Document_no_host;
            debug->outlog(1, "Local retrieval failed, trying HTTP\n");
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
// Pre:  - doc has ALL of its fields filled
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
    // finalEncoding = doc->convert()
    // if finalEncoding == plaintext
    // {
    //   CLuceneDoc->InsertField("contents", doc->contents);
    // }
    // else if finalEncoding == html
    // {
    //   execute code below
    // }
    // else
    // {
    //   return false
    // } 
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
    sprintf(tempTime, "%d", (int)doc->ModTime());
    CLuceneDoc->insertField("doc-time", tempTime);

    //
    // add the document size to the CLucene doc
    //
    char tempSize[32];
    sprintf(tempSize, "%d", doc->Length());
    CLuceneDoc->insertField("doc-size", tempSize);

    //
    // Add the URL to the CLucene doc
    //
    CLuceneDoc->insertField("url", (doc->Url()->get()).get());

    //
    // parse the actual document, recieving the parsed
    // urls in return
    //
    set<string> URLlist = tparser.parseDoc(contents);

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


int Spider::DeleteDoc(string * input)
{
    //
    // delete fromt he indexDB
    //
    String temp = input->c_str();
    indexDatabase.Delete(temp);

    //
    // delete from CLucene, and return number of documents deleted (hopefully only one)
    //
    return CLuceneDeleteURLFromIndex(input);

}


int Spider::DeleteDoc(int input)
{
    // 
    // TODO: the URL associated with the doc-id will need to be
    // returned (probably from CLucene, meaning the CLuceneAPI will need
    // to change), so the document can be deleted from the indexDB.
    //

    return CLuceneDeleteIDFromIndex(input);
}


// ----------------------------------------------------------
//
// Pre:  - document(s) are ready to insert.
//          NOTE: if the indexDB insert bool is false, then only insert the document
//          into CLucene. This can be useful if the document is to be searchable,
//          but not used in the regualr spidering process. Additionaly, It is the
//          responsibilty of the calling program (indexSingleDoc is currently the only
//          function that uses this option) to keep track of the URL or docID of such a
//          document, since it will need special handling - it will never be updated
//          or deleted thorugh normal crawler operation.
//
// Post: - document(s) have been inserted
//
void Spider::commitDocs()
{
    debug->outlog(2, "IndexDB inserting %s\n", indexDoc->DocURL().get());
    debug->outlog(2, "CLucene inserting %s\n", CLuceneDoc->getField("url"));

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
        debug->outlog(2, "\n   Rejected: item in exclude list ");
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
        debug->outlog(2, "\n   Rejected: item in bad query list ");
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
            debug->outlog(2, "\n   Rejected: Extension is invalid!");
            return (HTDIG_ERROR_TESTURL_EXTENSION);
		}
	}
	//
	// Or NOT in the list of valid ones
	//
	if (ext && valids.Count() > 0 && !valids.Exists(lowerext))
	{
        debug->outlog(2, "\n   Rejected: Extension is not valid!");
        return (HTDIG_ERROR_TESTURL_EXTENSION2);
	}

	//
	// If none of the limits is met, we disallow the URL
	//
	if (limits.match(url, 1, 0) == 0)
	{
        debug->outlog(1, "\n   Rejected: URL not in the limits! ");
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
    //aUrl.normalize();
	if (limitsn.match(aUrl.get(), 1, 0) == 0)
	{
        debug->outlog(2, "\n   Rejected: not in \"limit_normalized\" list!");
        return (HTDIG_ERROR_TESTURL_LIMITSNORM);
	}

	//
	// After that gauntlet, check to see if the server allows it (robots.txt)
	//
	if (server && server->IsDisallowed(url) != 0)
	{
        debug->outlog(2, "\n   Rejected: forbidden by server robots.txt!");
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

    debug->outlog(2, "new href: %s (no description)\n", url->get().get());

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
        debug->outlog(2, "resolving [%s]\n", url->get().get());

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
                debug->outlog(1, "\n   pushing [%s]\n", url->get().get());
                
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

                debug->outlog(0, "+");
            }
			else 
            {
                debug->outlog(0, "*");
            }
        }
    }
    else
    {
        //
        // Not a valid URL
        //
        debug->outlog(1, "\nurl rejected: (level 1) %s\n", url->get().get());
        debug->outlog(0, "-");

        if (urls_seen)
        {
            fprintf(urls_seen, "%s|||||%d\n", (const char *) url->get(), valid_url_code);
        }
    }
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

    debug->outlog(2, "redirect: [%s]\n", url.get().get());

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
        debug->outlog(2, "resolving [%s]\n", url.get().get());

        url.normalize();
        //
        // Now put it in the list of URLs to still visit.
        //
        if (Need2Get(url.get()))
        {
            debug->outlog(1, "   pushing [%s]\n", url.get().get());
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




void Spider::interruptClose(int signal)
{
    Server *server;
    URLRef *ref;

    //
    // close both databases
    //
    closeDBs();

    delete doc;
    
    if (_cookie_jar)
        delete _cookie_jar;

    if (urls_seen)
        fclose(urls_seen);
    if (images_seen)
        fclose(images_seen);

    //
    // create the URL log
    //
    String filelog = config->Find("url_log");
    FILE *urls_parsed = fopen((char *) filelog, "w");

    if (urls_parsed == 0)
    {
        cout << "Unable to create URL log file '" << filelog.get() << "' - dumping to screen" << endl;
        cout << "--- Start URL dump ---" << endl;
    }
        
    servers.Start_Get();
    while ((server = (Server *) servers.Get_NextElement()))
    {
        while (NULL != (ref = server->pop()))
        {
            if (urls_parsed == 0)
            {
                cout << ref->GetURL().get() << endl;
            }
            else
            {
                fprintf(urls_parsed, "%s\n", (const char *) ref->GetURL().get());
            }
            delete ref;
        }
    }

    if (urls_parsed == 0)
    {
        cout << "--- End URL dump ---" << endl;
    }
    else
    {
        fclose(urls_parsed);
    }

    //interruptClose();
    //if (signal == SIGALRM && die_on_timer == 0)
    //{
    //    seturn signal handlers to original states
    //    return control to calling program ???? 
    //}
    //else
    {
        exit(1);
    }
}



//*****************************************************************************
// static void sig_handlers
//   initialise signal handlers
//
void Spider::sig_handlers(int indexTimeout, int indexTimeoutPrecise)
{
#ifndef _MSC_VER /* _WIN32 */
    if (interruptHandlerSpiderPointer == NULL)
    {  
        interruptHandlerSpiderPointer = this;
    }
    else
    {
        //
        // only assign the global variable ONCE
        //
        cerr << "Tried to reassign interruptHandlerSpiderPointer! " << endl;
    }

    //POSIX SIGNALS
    struct sigaction action;

	/* SIGINT, SIGQUIT, SIGTERM */
    action.sa_handler = sigexit;

    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGINT, &action, NULL) != 0)
        cout << "Cannot install SIGINT handler" << endl;
    if (sigaction(SIGABRT, &action, NULL) != 0)
        cout << "Cannot install SIGABRT handler" << endl;
    if (sigaction(SIGQUIT, &action, NULL) != 0)
        cout << "Cannot install SIGQUIT handler" << endl;
    if (sigaction(SIGTERM, &action, NULL) != 0)
         cout << "Cannot install SIGTERM handler" << endl;
    if (sigaction(SIGHUP, &action, NULL) != 0)
        cout << "Cannot install SIGHUP handler" << endl;
    if (sigaction(SIGALRM, &action, NULL) != 0)
        cout << "Cannot install SIGALRM handler" << endl;

    if (indexTimeout > 0)
    {
        struct itimerval itimer;

        //
        // non-repeating
        //
        itimer . it_interval . tv_sec = 0;
        itimer . it_interval . tv_usec = 0;
 
        itimer . it_value . tv_sec = indexTimeout;
        itimer . it_value . tv_usec = 0;

        if (setitimer (ITIMER_REAL, &itimer, NULL) < 0)
        {
            cout << "Could not setitimer for index_timeout" << endl;
        }
    }

    if (indexTimeoutPrecise > 0)
    {
        struct itimerval itimer;

        //
        // non-repeating
        //
        itimer . it_interval . tv_sec = 0;
        itimer . it_interval . tv_usec = 0;
 
        itimer . it_value . tv_sec = indexTimeoutPrecise;
        itimer . it_value . tv_usec = 0;

        if (setitimer (ITIMER_VIRTUAL, &itimer, NULL) < 0)
        {
            cout << "Could not setitimer for index_timeout_precise" << endl;
        }
    }
#else
    //ANSI C signal handling - Limited to supported Windows signals.
    signal(SIGINT, sigexit); 
    signal(SIGTERM, sigexit); 
#endif //_MSC_VER /* _WIN32 */
}



void Spider::sig_phandler(void)
{
#ifndef _MSC_VER /* _WIN32 */
    struct sigaction action;

    sigemptyset(&action.sa_mask);
    action.sa_handler = sigpipe;
    action.sa_flags = SA_RESTART;
    if (sigaction(SIGPIPE, &action, NULL) != 0)
    {
        cout << "Cannot install SIGPIPE handler" << endl;
    }
#endif //_MSC_VER /* _WIN32 */
}


//*****************************************************************************
// static void win32_check_messages
//   Check WIN32 messages!
//
void Spider::win32_check_messages(void)
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

void sigexit(int signal)
{
    interruptHandlerSpiderPointer->interruptClose(signal);
}

void sigpipe(int)
{
}




