//
// Retriever.cc
//
// Implementation of Retriever
//
// $Id: Retriever.cc,v 1.44 1999/05/04 19:45:08 ghutchis Exp $
//

#include "Retriever.h"
#include "htdig.h"
#include "WordList.h"
#include "URLRef.h"
#include "Server.h"
#include "Parsable.h"
#include "Document.h"
#include "StringList.h"
#include "HtWordType.h"
#include <pwd.h>
#include <signal.h>
#include <assert.h>
#include <stdio.h>

static WordList	words;
static int noSignal;


//*****************************************************************************
// Retriever::Retriever()
//
Retriever::Retriever(RetrieverLog flags)
{
    FILE	*urls_parsed;

    currenthopcount = 0;
    max_hop_count = config.Value("max_hop_count", 999999);
		
    //
    // Initialize the weight factors for words in the different
    // HTML headers
    //
    factor[0] = config.Double("text_factor"); // Normal words
    factor[1] = config.Double("title_factor");
    factor[2] = config.Double("heading_factor_1");
    factor[3] = config.Double("heading_factor_2");
    factor[4] = config.Double("heading_factor_3");
    factor[5] = config.Double("heading_factor_4");
    factor[6] = config.Double("heading_factor_5");
    factor[7] = config.Double("heading_factor_6");
    factor[8] = 0;
    factor[9] = 0;
    factor[10] = config.Double("keywords_factor");
    factor[11] = config.Double("meta_description_factor");
	
    //
    // Open the file to which we will append words.
    //
    String	filename = config["word_list"];
    words.WordTempFile(filename);
    words.BadWordFile(config["bad_word_list"]);

    doc = new Document();
    minimumWordLength = config.Value("minimum_word_length", 3);

    log = flags;
    // if in restart mode
    if (Retriever_noLog != log ) 
    {
    String	filelog = config["url_log"];
    char buffer[1000];	// FIXME
    int  l;

	urls_parsed = fopen(filelog,   "r" );
	if (0 != urls_parsed)
        {
  	    // read all url discovered but not fetched before 
	    while (fgets(buffer, sizeof(buffer), urls_parsed))
      	    {
	       l = strlen(buffer);
	       assert(l && buffer[l -1] == '\n');
	       buffer[l -1] = 0;
	       Initial(buffer,2);
            }
            fclose(urls_parsed);
	}
        unlink(filelog);
    }
}


//*****************************************************************************
// Retriever::~Retriever()
//
Retriever::~Retriever()
{
    delete doc;
}


//*****************************************************************************
// void Retriever::setUsernamePassword(char *credentials)
//
void
Retriever::setUsernamePassword(char *credentials)
{
    doc->setUsernamePassword(credentials);
}


//*****************************************************************************
// void Retriever::Initial(char *list, int from)
//   Add a single URL to the list of URLs to visit.
//   Since URLs are stored on a per server basis, we first need to find the
//   the correct server to add the URL's path to.
//
//   from == 0 urls in db.docs and no db.log
//   from == 1 urls in start_url add url only if not already in the list 
//   from == 2 add url from db.log 
//   from == 3 urls in db.docs and there was a db.log 
//
void
Retriever::Initial(char *list, int from)
{
    //
    // Split the list of urls up into individual urls.
    //
    StringList	tokens(list, " \t");
    String	sig;
    String      url;
    Server	*server;

    for (int i = 0; i < tokens.Count(); i++)
    {
	URL	u(tokens[i]);
	server = (Server *) servers[u.signature()];
	url = u.get();
	if (debug > 2)
           cout << "\t" << from << ":" << (int) log << ":" << url;
	if (!server)
	{
	    server = new Server(u.host(), u.port());
	    servers.Add(u.signature(), server);
	}
	else if (from && visited.Exists(url)) 
	{
	    if (debug > 2)
                cout << " skipped" << endl;
	    continue;
	}
        if (Retriever_noLog == log || from != 3) 
        {
	    if (debug > 2)
                cout << " pushed";
     	server->push(u.get(), 0, 0);
        }
	if (debug > 2)
           cout << endl;
	visited.Add(url, 0);
    }
}


//*****************************************************************************
// void Retriever::Initial(List &list, int from)
//
void
Retriever::Initial(List &list,int from)
{
    list.Start_Get();
    String	*str;
    // from == 0 is an optimisation for pushing url in update mode
    //  assuming that 
    // 1) there's many more urls in docdb 
    // 2) they're pushed first
    // 3)  there's no duplicate url in docdb
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
	Initial(str->get(),from);
    }
}

//*****************************************************************************
//
static void sigexit(int)
{
 noSignal=0;
}

//*****************************************************************************
// static void sig_handlers
//	initialise signal handlers
//
static void
sig_handlers(void)
{
struct sigaction action;

 /* SIGINT, SIGQUIT, SIGTERM */
 action.sa_handler = sigexit;
 sigemptyset(&action.sa_mask);
 action.sa_flags = 0;
 if (sigaction(SIGINT, &action, NULL) != 0)
	reportError("Cannot install SIGINT handler\n");
 if(sigaction(SIGQUIT, &action, NULL) != 0)
	reportError("Cannot install SIGQUIT handler\n");
 if(sigaction(SIGTERM, &action, NULL) != 0)
	reportError("Cannot install SIGTERM handler\n");
 if(sigaction(SIGHUP, &action, NULL) != 0)
	reportError("Cannot install SIGHUP handler\n");
}


//*****************************************************************************
// void Retriever::Start()
//   This is the main loop of the retriever.  We will go through the
//   list of paths stored for each server.  While parsing the
//   retrieved documents, new paths will be added to the servers.  We
//   return if no more paths need to be retrieved.
//
void
Retriever::Start()
{
    //
    // Main digger loop.  The todo list should initialy have the start
    // URL and all the URLs which were seen in a previous dig.  The
    // loop will continue as long as there are more URLs to visit.
    //
    int		more = 1;
    Server	*server;
    URLRef	*ref;
    
    //  
    // Always sig . The delay bother me but a bad db is worst
    // 
    if ( Retriever_noLog != log ) 
    {
	sig_handlers();
    }
    noSignal = 1;

    while (more && noSignal)
    {
	more = 0;
		
	//
	// Go through all the current servers in sequence.  We take only one
	// URL from each server during this loop.  This ensures that the load
	// on the servers is distributed evenly.
	//
	servers.Start_Get();
	while ( (server = (Server *)servers.Get_NextElement()) && noSignal)
	{
	    if (debug > 1)
		cout << "pick: " << server->host() << ", # servers = " <<
		    servers.Count() << endl;
	    
	    ref = server->pop();
	    if (!ref)
		continue;		      // Nothing on this server
	    // There may be no more documents, or the server
	    // has passed the server_max_docs limit

	    //
	    // We have a URL to index, now.  We need to register the
	    // fact that we are not done yet by setting the 'more'
	    // variable.
	    //
	    more = 1;

	    //
	    // Deal with the actual URL.
	    // We'll check with the server to see if we need to sleep()
	    // before parsing it.
	    //
	    server->delay();   // This will pause if needed and reset the time
	    parse_url(*ref);
            delete ref;
	}
    }
    // if we exited on signal 
    if (Retriever_noLog != log && !noSignal) 
    {
    FILE	*urls_parsed;
    String	filelog = config["url_log"];
        // save url seen but not fetched
	urls_parsed = fopen(filelog,   "w" );
	if (0 == urls_parsed)
	{
	    reportError(form("Unable to create URL log file '%s'",
			     filelog.get()));
	}
        else {
  	   servers.Start_Get();
	   while ((server = (Server *)servers.Get_NextElement()))
	   {
   	      while (NULL != (ref = server->pop())) 
              {
      	          fprintf(urls_parsed, "%s\n", ref->URL());
 		  delete ref;
              }
           }
           fclose(urls_parsed);
        }
    }
}


//*****************************************************************************
// void Retriever::parse_url(URL &url)
//
void
Retriever::parse_url(URLRef &urlRef)
{
    URL			url;
    DocumentRef	*ref;
    int			old_document;
    time_t		date;
    static int	index = 0;

//	cout << "**** urlRef URL = '" << urlRef.URL() << "', referer = '" <<
//		urlRef.Referer() << "'\n";
    url.parse(urlRef.URL());
	
    currenthopcount = urlRef.HopCount();
    ref = GetRef(url.get());
    if (ref)
    {
	//
	// We already have an entry for this document in our database.
	// This means we can get the document ID and last modification
	// time from there.
	//
	current_id = ref->DocID();
	date = ref->DocTime();
	if (ref->DocAccessed())
	  old_document = 1;
	else // we haven't retrieved it yet, so we only have the first link
	  old_document = 0;
	ref->DocBackLinks(ref->DocBackLinks() + 1); // we had a new link
	ref->DocAccessed(time(0));
	ref->DocState(Reference_normal);
        currenthopcount=ref->DocHopCount();
    }
    else
    {
	//
	// Never seen this document before.  We need to create an
	// entry for it.  This implies that it gets a new document ID.
	//
	date = 0;
	current_id = docs.NextDocID();
	ref = new DocumentRef;
	ref->DocID(current_id);
	ref->DocURL(url.get());
	ref->DocState(Reference_normal);
	ref->DocAccessed(time(0));
        ref->DocHopCount(currenthopcount);
	ref->DocBackLinks(1); // We had to have a link to get here!
	old_document = 0;
    }

    words.DocumentID(ref->DocID());

    if (debug > 0)
    {
	//
	// Display progress
	//
	cout << index++ <<
	    ':' << current_id <<
	    ':' << currenthopcount <<
	    ':' << url.get() << ": ";
	cout.flush();
    }
    
    // Reset the document to clean out any old data
    doc->Reset();

    doc->Url(url.get());
    doc->Referer(urlRef.Referer());

    base = doc->Url();

    // Retrive document, first trying local file access if possible.
    Document::DocStatus status;
    String *local_filename = IsLocalUser(url.get());
    if (!local_filename)
        local_filename = IsLocal(url.get());
    if (local_filename)
    {  
        if (debug > 1)
	    cout << "Trying local file " << *local_filename << endl;
        status = doc->RetrieveLocal(date, *local_filename);
        if (status == Document::Document_not_local)
        {
            if (debug > 1)
   	        cout << "Local retrieval failed, trying HTTP" << endl;
            status = doc->RetrieveHTTP(date);
        }
        delete local_filename;
    }
    else
        status = doc->RetrieveHTTP(date);

    current_ref = ref;
	
    //
    // Determine what to do by looking at the status code returned by
    // the Document retrieval process.
    //
    switch (status)
    {
	case Document::Document_ok:
	    trackWords = 1;
	    if (old_document)
	    {
	      if (doc->ModTime() == ref->DocTime())
		{
		  if (debug)
		    cout << " retrieved but not changed" << endl;
		  words.MarkScanned();
		  break;
		}
		//
		// Since we already had a record of this document and
		// we were able to retrieve it, it must have changed
		// since the last time we scanned it.  This means that
		// we need to assign a new document ID to it and mark
		// the old one as obsolete.
		//
		words.MarkModified();
	        int backlinks = ref->DocBackLinks();
		delete ref;
		current_id = docs.NextDocID();
		words.DocumentID(current_id);
		ref = new DocumentRef;
		ref->DocID(current_id);
		ref->DocURL(url.get());
		ref->DocState(Reference_normal);
		ref->DocAccessed(time(0));
		ref->DocHopCount(currenthopcount);
		ref->DocBackLinks(backlinks);
		if (debug)
		    cout << " (changed) ";
	    }
	    RetrievedDocument(*doc, url.get(), ref);
	    // Hey! If this document is marked noindex, don't even bother
	    // adding new words. Mark this as gone and get rid of it!
	    if (ref->DocState() == Reference_noindex)
	      words.MarkModified();
	    else
	      words.Flush();
	    if (debug)
		cout << " size = " << doc->Length() << endl;
	    break;

	case Document::Document_not_changed:
	    if (debug)
		cout << " not changed" << endl;
	    words.MarkScanned();
	    break;

	case Document::Document_not_found:
	    ref->DocState(Reference_not_found);
	    if (debug)
		cout << " not found" << endl;
	    recordNotFound(url.get(),
			   urlRef.Referer(),
			   Document::Document_not_found);
	    words.MarkGone();
	    break;

	case Document::Document_no_host:
	    ref->DocState(Reference_not_found);
	    if (debug)
		cout << " host not found" << endl;
	    recordNotFound(url.get(),
			   urlRef.Referer(),
			   Document::Document_no_host);
	    words.MarkGone();
	    break;

	case Document::Document_no_server:
	    ref->DocState(Reference_not_found);
	    if (debug)
		cout << " no server running" << endl;
	    recordNotFound(url.get(),
			   urlRef.Referer(),
			   Document::Document_no_server);
	    words.MarkGone();
	    break;

	case Document::Document_not_html:
	    if (debug)
		cout << " not HTML" << endl;
	    words.MarkGone();
	    break;

	case Document::Document_redirect:
	    if (debug)
		cout << " redirect" << endl;
	    words.MarkGone();
	    got_redirect(doc->Redirected(), ref);
	    break;
	    
       case Document::Document_not_authorized:
	    if (debug)
	      cout << " not authorized" << endl;
	    break;

      case Document::Document_not_local:
	   if (debug)
	     cout << " not local" << endl;
	   break;
    }
    docs.Add(*ref);
    delete ref;
}


//*****************************************************************************
// void Retriever::RetrievedDocument(Document &doc, char *url, DocumentRef *ref)
//   We found a document that needs to be parsed.  Since we don't know the
//   document type, we'll let the Document itself return an appropriate
//   Parsable object which we can call upon to parse the document contents.
//
void
Retriever::RetrievedDocument(Document &doc, char *, DocumentRef *ref)
{
    n_links = 0;
    current_ref = ref;
    current_anchor_number = 0;
    current_title = 0;
    current_time = 0;
    current_head = 0;
    current_meta_dsc = 0;

    //
    // Create a parser object and let it have a go at the document.
    // We will pass ourselves as a callback object for all the got_*()
    // routines.
    // This will generate the Parsable object as a specific parser
    //
    Parsable	*parsable = doc.getParsable();
    parsable->parse(*this, *base);

    //
    // We don't need to dispose of the parsable object since it will
    // automatically be reused.
    //

    //
    // Update the document reference
    //
    ref->DocHead(current_head);
    ref->DocMetaDsc(current_meta_dsc);
    if (current_time == 0)
      ref->DocTime(doc.ModTime());
    else
      ref->DocTime(current_time);
    ref->DocTitle(current_title);
    ref->DocSize(doc.Length());
    ref->DocAccessed(time(0));
    ref->DocLinks(n_links);
    ref->DocImageSize(ref->DocImageSize() + doc.Length());
}


//*****************************************************************************
// int Retriever::Need2Get(char *u)
//   Return TRUE if we need to retrieve the given url.  This will
//   check the list of urls we have already visited.
//
int
Retriever::Need2Get(char *u)
{
    static String	url;
    url = u;

    return !visited.Exists(url);
}



//*****************************************************************************
// int Retriever::IsValidURL(char *u)
//   Return TRUE if we need to retrieve the given url.  We will check
//   for limits here.
//
int
Retriever::IsValidURL(char *u)
{
    static Dictionary	*invalids = 0;

    //
    // Invalid extensions will be kept in a dictionary for quick
    // lookup.  Since the dictionary is static to this function, we
    // need to initialize it the first time we get here.
    //
    if (!invalids)
    {
	// A list of bad extensions, separated by spaces or tabs
	String	t = config["bad_extensions"];
	String lowerp;
	char	*p = strtok(t, " \t");
	invalids = new Dictionary;
	while (p)
	{
	  // Extensions are case insensitive
	  lowerp = p;
	  lowerp.lowercase();
	  invalids->Add(lowerp, 0);
	  p = strtok(0, " \t");
	}
    }

    static String	url;
    url = u;

    //
    // Currently, we only deal with HTTP URLs.  Gopher and ftp will
    // come later...  ***FIX***
    //
    if (strstr(u, "..") || strncmp(u, "http://", 7) != 0)
      {
	if (debug > 2)
	  cout << endl <<"   Rejected: Not an http or relative link!";
	return FALSE;
      }

    //
    // If the URL contains any of the patterns in the exclude list,
    // mark it as invalid
    //
#ifdef  REGEX
    if (excludes.match(url, 0, 0) != 0)
      {
                if (debug >= 2)
		  cout << endl << "   Rejected: intem in exclude list ";
                return(FALSE);
      }
#else
    if (excludes.hasPattern()) // Make sure there's an exclude list!
      {
	int retValue;     // Returned value of findFirst
	int myWhich = 0;    // Item # that matched [0 .. n]
	int myLength = 0;   // Length of the matching value
	retValue = excludes.FindFirst(url, myWhich, myLength);
	if (retValue >= 0)
	  {
	    if (debug > 2)
	      {
		myWhich++;         // [0 .. n] --> [1 .. n+1]
		cout << endl <<"  Rejected: Item in the exclude list: item # ";
		cout << myWhich << " length: " << myLength << endl;
	      }
	    return FALSE;
	  }
      }
#endif
    //
    // See if the path extension is in the list of invalid ones
    //
    char	*ext = strrchr(url, '.');
    String	lowerext = ext;
    lowerext.lowercase();
    if (ext && invalids->Exists(lowerext))
      {
	if (debug > 2)
	  cout << endl <<"   Rejected: Extension is invalid!";
	return FALSE;
      }

    ext = strrchr(url, '?');
    if (ext && badquerystr.hasPattern() &&
       (badquerystr.FindFirst(ext) >= 0))
    {
      if (debug > 2)
	  cout << endl <<"   Rejected: Invalid Querystring!";
       return FALSE;
    }

    //
    // If any of the limits are met, we allow the URL
    //
#ifdef  REGEX
    if (limits.match(url, 1, 0) != 0) return(TRUE);
#else
    if (limits.FindFirst(url) >= 0)
	return TRUE;
#endif

    if (debug > 2)
      cout << endl <<"   Rejected: URL not in the limits!";

    return FALSE;
}


//*****************************************************************************
// String* Retriever::IsLocal(char *url)
//   Returns a string containing the (possible) local filename
//   of the given url, or 0 if it's definitely not local.
//   THE CALLER MUST FREE THE STRING AFTER USE!
//
String*
Retriever::IsLocal(char *url)
{
    static StringList *prefixes = 0;
    static StringList *paths = 0;

    //
    // Initialize prefix/path list if this is the first time.
    // The list is given in format "prefix1=path1 prefix2=path2 ..."
    //
    if (!prefixes)
    {
    	prefixes = new StringList();
	paths = new StringList();

	String t = config["local_urls"];
	char *p = strtok(t, " \t");
	while (p)	
	{
   	    char *path = strchr(p, '=');
   	    if (!path)
   		continue;
   	    *path++ = '\0';
            prefixes->Add(p);
            paths->Add(path);
	    p = strtok(0, " \t");
	}
    }

    // This shouldn't happen, but check anyway...
    if (strstr(url, ".."))
        return 0;
    
    String *prefix, *path;
    prefixes->Start_Get();
    paths->Start_Get();
    while ((prefix = (String*) prefixes->Get_Next()))
    {
	path = (String*) paths->Get_Next();
        if (mystrncasecmp(*prefix, url, prefix->length()) == 0)
	{
	    int l = strlen(url)-prefix->length()+path->length()+4;
	    String *local = new String(*path, l);
	    *local += &url[prefix->length()];
	    if (local->last() == '/' && config["local_default_doc"] != "")
	      *local += config["local_default_doc"];
	    return local;
	}	
    }
    return 0;
}


//*****************************************************************************
// String* Retriever::IsLocalUser(char *url)
//   If the URL has ~user part, returns a string containing the
//   (possible) local filename of the given url, or 0 if it's
//   definitely not local.
//   THE CALLER MUST FREE THE STRING AFTER USE!
//
String*
Retriever::IsLocalUser(char *url)
{
    static StringList *prefixes = 0, *paths = 0, *dirs = 0;
    static Dictionary home_cache;

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
	String t = config["local_user_urls"];
	char *p = strtok(t, " \t");
	while (p)
	{
	    char *path = strchr(p, '=');
	    if (!path)
	        continue;
	    *path++ = '\0';
	    char *dir = strchr(path, ',');
	    if (!dir)
	        continue;
	    *dir++ = '\0';
	    prefixes->Add(p);
	    paths->Add(path);
	    dirs->Add(dir);
	    p = strtok(0, " \t");
	}
    }

    // Can we do anything about this?
    if (!strchr(url, '~') || !prefixes->Count() || strstr(url, ".."))
        return 0;

    // Split the URL to components
    String tmp = url;
    char *name = strchr(tmp, '~');
    *name++ = '\0';
    char *rest = strchr(name, '/');
    if (!rest || (rest-name <= 1) || (rest-name > 32))
        return 0;
    *rest++ = '\0';

    // Look it up in the prefix/path/dir table
    prefixes->Start_Get();
    paths->Start_Get();
    dirs->Start_Get();
    String *prefix, *path, *dir;
    while ((prefix = (String*) prefixes->Get_Next()))
    {
        path = (String*) paths->Get_Next();
	dir = (String*) dirs->Get_Next();
        if (mystrcasecmp(*prefix, tmp) != 0)
  	    continue;

	String *local = new String;
	// No path, look up home directory
	if (path->length() == 0)
	{
	    String *home = (String*) home_cache[name];
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
	        return 0;
	}
	else
	{
	    *local += *path;
	    *local += name;
	}
	*local += *dir;
	*local += rest;
	if (local->last() == '/' && config["local_default_doc"] != "")
	  *local += config["local_default_doc"];
	return local;
    }
    return 0;
}

 
//*****************************************************************************
// DocumentRef *Retriever::GetRef(char *u)
//   Extract the date field from the given url.  This will require a
//   lookup in the current document database.
//
DocumentRef*
Retriever::GetRef(char *u)
{
    static String	url;
    url = u;

    return docs[url];
}


//*****************************************************************************
// void Retriever::got_word(char *word, int location, int heading)
//   The location is normalized to be in the range 0 - 1000.
//
void
Retriever::got_word(char *word, int location, int heading)
{
    if (debug > 3)
	cout << "word: " << word << '@' << location << endl;
    if (heading > 11 || heading < 0) // Current limits for headings
      heading = 0;  // Assume it's just normal text
    if (trackWords)
    {
      String w = word;
      HtStripPunctuation(w);
      if (w.length() >= minimumWordLength)
	words.Word(w, location, current_anchor_number, factor[heading]);
    }
}


//*****************************************************************************
// void Retriever::got_title(char *title)
//
void
Retriever::got_title(char *title)
{
    if (debug > 1)
	cout << "\ntitle: " << title << endl;
    current_title = title;
}

//*****************************************************************************
// void Retriever::got_time(char *time)
//
void
Retriever::got_time(char *time)
{
    time_t   new_time;
    struct tm   tm;

    //	Initialize the tm construct to ensure correct results
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    tm.tm_mon = 0;
    tm.tm_mday = 1;
    tm.tm_year = 0;

    if (debug > 1)
      cout << "\ntime: " << time << endl;

    //
    // As defined by the Dublin Core, this should be YYYY-MM-DD
    // In the future, we'll need to deal with the scheme portion
    //  in case someone picks a different format.
    //
    if (Htstrptime(time, "%Y-%m-%d", &tm))
      {
#if HAVE_TIMEGM
        new_time = timegm(&tm);
#else
        new_time = Httimegm(&tm);
#endif
	current_time = new_time;
      }

    // If we can't convert it, current_time stays the same and we get
    // the default--the date returned by the server...
}

//*****************************************************************************
// void Retriever::got_anchor(char *anchor)
//
void
Retriever::got_anchor(char *anchor)
{
    if (debug > 2)
	cout << "anchor: " << anchor << endl;
    current_ref->AddAnchor(anchor);
    current_anchor_number++;
}


//*****************************************************************************
// void Retriever::got_image(char *src)
//
void
Retriever::got_image(char *src)
{
    URL	url(src, *base);
    char	*image = url.get();
	
    if (debug > 2)
	cout << "image: " << image << endl;

    if (images_seen)
	fprintf(images_seen, "%s\n", image);
//	current_ref->DocImageSize(current_ref->DocImageSize() + images.Sizeof(image));
}


//*****************************************************************************
// void Retriever::got_href(char *href, char *description)
//
void
Retriever::got_href(URL &url, char *description)
{
    DocumentRef		*ref;
    Server		*server;

    if (debug > 2)
	cout << "href: " << url.get() << " (" << description << ')' << endl;

    n_links++;

    if (urls_seen)
	fprintf(urls_seen, "%s\n", url.get());

    //
    // Check if this URL falls within the valid range of URLs.
    //
    if (IsValidURL(url.get()))
    {
	//
	// It is valid.  Normalize it (resolve cnames for the server)
	// and check again...
	//
	if (debug > 2)
	{
	    cout << "resolving '" << url.get() << "'\n";
	    cout.flush();
	}

	url.normalize();

	// If it is a backlink from the current document,
	// just update that field.  Writing to the database
	// is meaningless, as it will be overwritten.
	// Adding it as a new document may even be harmful, as
	// that will be a duplicate.  This can happen if the
	// current document is never referenced before, as in a
	// start_url.

	if (strcmp(url.get(), current_ref->DocURL()) == 0)
	{
	    current_ref->DocBackLinks(current_ref->DocBackLinks() + 1);
	    current_ref->AddDescription(description);
	}
#ifdef  REGEX
        else if (limitsn.match(url.get(), 1, 0) != 0)
#else
	else if (limitsn.FindFirst(url.get()) >= 0)
#endif
	{
	    //
	    // First add it to the document database
	    //
	    ref = docs[url.get()];
	    // if ref exists we have to call AddDescription even
            // if max_hop_count is reached
    	    if (!ref && currenthopcount + 1 > max_hop_count)
		return;

	    if (!ref)
	    {
		//
		// Didn't see this one, yet.  Create a new reference
		// for it with a unique document ID
		//
		ref = new DocumentRef;
		ref->DocID(docs.NextDocID());
		ref->DocHopCount(currenthopcount + 1);
	    }
	    ref->DocBackLinks(ref->DocBackLinks() + 1); // This one!
	    ref->DocURL(url.get());
	    ref->AddDescription(description);

	    //
    	    // If the dig is restricting by hop count, perform the check here 
	    // too
    	    if (currenthopcount + 1 > max_hop_count)
	    {
		delete ref;
		return;
	    }

	    if (ref->DocHopCount() != -1 &&
		ref->DocHopCount() < currenthopcount + 1)
	       // If we had taken the path through this ref
	       // We'd be here faster than currenthopcount
	       currenthopcount = ref->DocHopCount();  // So update it!

	    docs.Add(*ref);

	    //
	    // Now put it in the list of URLs to still visit.
	    //
	    if (Need2Get(url.get()))
	    {
		if (debug > 1)
		    cout << "\n   pushing " << url.get() << endl;
		server = (Server *) servers[url.signature()];
		if (!server)
		{
		    //
		    // Hadn't seen this server, yet.  Register it
		    //
		    server = new Server(url.host(), url.port());
		    servers.Add(url.signature(), server);
		}
		//
		// Let's just be sure we're not pushing an empty URL
		//
		if (strlen(url.get()))
		  server->push(url.get(), ref->DocHopCount(), base->get());

		String	temp = url.get();
		visited.Add(temp, 0);
		if (debug)
		    cout << '+';
	    }
	    else if (debug)
		cout << '*';
	    delete ref;
	}
	else
	{
	    //
	    // Not a valid URL
	    //
	    if (debug > 1)
		cout << "\nurl rejected: (level 2)" << url.get() << endl;
	    if (debug == 1)
		cout << '-';
	}
    }
    else
    {
	//
	// Not a valid URL
	//
	if (debug > 1)
	    cout << "\nurl rejected: (level 1)" << url.get() << endl;
	if (debug == 1)
	    cout << '-';
    }
    if (debug)
	cout.flush();
}


//*****************************************************************************
// void Retriever::got_redirect(char *new_url, DocumentRef *old_ref)
//
void
Retriever::got_redirect(char *new_url, DocumentRef *old_ref)
{
    URL	url(new_url);

    if (debug > 2)
	cout << "redirect: " << url.get() << endl;

    n_links++;

    if (urls_seen)
	fprintf(urls_seen, "%s\n", url.get());

    //
    // Check if this URL falls within the valid range of URLs.
    //
    if (IsValidURL(url.get()))
    {
	//
	// It is valid.  Normalize it (resolve cnames for the server)
	// and check again...
	//
	if (debug > 2)
	{
	    cout << "resolving '" << url.get() << "'\n";
	    cout.flush();
	}

	url.normalize();
#ifdef  REGEX
        if (limitsn.match(url.get(), 1, 0) != 0)
#else
	if (limitsn.FindFirst(url.get()) >= 0)
#endif
	{
	    //
	    // First add it to the document database
	    //
	    DocumentRef	*ref = docs[url.get()];
	    if (!ref)
	    {
		//
		// Didn't see this one, yet.  Create a new reference
		// for it with a unique document ID
		//
		ref = new DocumentRef;
		ref->DocID(docs.NextDocID());
		ref->DocHopCount(currenthopcount);
	    }
	    ref->DocURL(url.get());
			
	    //
	    // Copy the descriptions of the old DocRef to this one
	    //
	    List	*d = old_ref->Descriptions();
	    if (d)
	    {
		d->Start_Get();
		String	*str;
		while ((str = (String *) d->Get_Next()))
		{
		    ref->AddDescription(str->get());
		}
	    }
	    if (ref->DocHopCount() > old_ref->DocHopCount())
		ref->DocHopCount(old_ref->DocHopCount());

	    // Copy the number of backlinks
	    ref->DocBackLinks(old_ref->DocBackLinks());

	    docs.Add(*ref);

	    //
	    // Now put it in the list of URLs to still visit.
	    //
	    if (Need2Get(url.get()))
	    {
		if (debug > 1)
		    cout << "   pushing " << url.get() << endl;
		Server	*server = (Server *) servers[url.signature()];
		if (!server)
		{
		    //
		    // Hadn't seen this server, yet.  Register it
		    //
		    server = new Server(url.host(), url.port());
		    servers.Add(url.signature(), server);
		}
		server->push(url.get(), ref->DocHopCount(), base->get());

		String	temp = url.get();
		visited.Add(temp, 0);
	    }

	    delete ref;
	}
    }
}


//*****************************************************************************
// void Retriever::got_head(char *head)
//
void
Retriever::got_head(char *head)
{
    if (debug > 4)
	cout << "head: " << head << endl;
    current_head = head;
}

//*****************************************************************************
// void Retriever::got_meta_dsc(char *md)
//
void
Retriever::got_meta_dsc(char *md)
{
    if (debug > 4)
	cout << "meta description: " << md << endl;
    current_meta_dsc = md;
}


//*****************************************************************************
// void Retriever::got_meta_email(char *e)
//
void
Retriever::got_meta_email(char *e)
{
    if (debug > 1)
	cout << "\nmeta email: " << e << endl;
    current_ref->DocEmail(e);
}


//*****************************************************************************
// void Retriever::got_meta_notification(char *e)
//
void
Retriever::got_meta_notification(char *e)
{
    if (debug > 1)
	cout << "\nmeta notification date: " << e << endl;
    current_ref->DocNotification(e);
}


//*****************************************************************************
// void Retriever::got_meta_subject(char *e)
//
void
Retriever::got_meta_subject(char *e)
{
    if (debug > 1)
	cout << "\nmeta subect: " << e << endl;
    current_ref->DocSubject(e);
}


//*****************************************************************************
// void Retriever::got_noindex()
//
void
Retriever::got_noindex()
{
    if (debug > 1)
      cout << "\nMETA ROBOT: Noindex " << endl;
    current_ref->DocState(Reference_noindex);
}


//*****************************************************************************
//
void
Retriever::recordNotFound(char *url, char *referer, int reason)
{
    char	*message = "";
    
    switch (reason)
    {
	case Document::Document_not_found:
	    message = "Not found";
	    break;
	
	case Document::Document_no_host:
	    message = "Unknown host";
	    break;
	
	case Document::Document_no_server:
	    message = "Unable to contact server";
	    break;
    }

    notFound << message << ": " << url << " Ref: " << referer << '\n';
}

//*****************************************************************************
// void Retriever::ReportStatistics(char *name)
//
void
Retriever::ReportStatistics(char *name)
{
    cout << name << ": Run complete\n";
    cout << name << ": " << servers.Count() << " server";
    if (servers.Count() > 1)
	cout << "s";
    cout << " seen:\n";

    Server		*server;
    String		buffer;
    StringList	results;
    String		newname = name;

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
}

