//
// Retriever.cc
//
// Implementation of Retriever
//
// $Log: Retriever.cc,v $
// Revision 1.32  1999/01/14 01:09:12  ghutchis
// Small speed improvements based on gprof.
//
// Revision 1.31  1999/01/14 00:35:22  ghutchis
// Call AddDescription even if max_hop_count is reached.
//
// Revision 1.30  1999/01/07 04:05:25  ghutchis
// Skip changing to lowercase in got_word, we do it in WordList::Word.
//
// Revision 1.29  1999/01/05 19:12:59  bergolth
// fixed bug in bad_querystring detection
//
// Revision 1.28  1998/12/19 18:09:03  bergolth
// Added bad_querystr option.
//
// Revision 1.27  1998/12/13 05:38:14  ghutchis
// Added check to prevent currenthopcount from becoming -1.
//
// Revision 1.26  1998/12/12 01:44:33  ghutchis
// Added additional debugging info on the reason for excluding a URL, based on
// a patch by Benoit Majeau <Benoit.Majeau@nrc.ca>.
//
// Revision 1.25  1998/12/11 02:54:07  ghutchis
// Changed support for server_wait_time to use delay() method in Server. Delay
// is from beginning of last connection to this one.
//
// Revision 1.24  1998/12/08 02:54:24  ghutchis
// Use server_wait_time to call sleep() before requests. Should help prevent
// server abuse. :-)
//
// Revision 1.23  1998/12/06 18:44:00  ghutchis
// Don't add the text of descriptions to the word db here, it's better to do it
// in the DocumentRef itself.
//
// Revision 1.22  1998/12/05 00:52:55  ghutchis
// Added a parameter to Initial function to prevent URLs from being checked
// twice during an update dig.
//
// Revision 1.21  1998/12/02 02:45:10  ghutchis
// Update hopcount correctly by taking the shortest paths to documents.
//
// Revision 1.20  1998/11/27 18:33:37  ghutchis
// Changed Retriever::got_word to check for small words, valid_punctuation to
// remove bugs in HTML.cc.
//
// Revision 1.18  1998/11/22 19:14:16  ghutchis
// Use "description_factor" to weight link descriptions with the documents at
// the end of the link.
//
// Revision 1.17  1998/11/16 16:10:17  ghutchis
// Fix back link count.
//
// Revision 1.16  1998/11/15 22:29:27  ghutchis
// Implement docBackLinks backlink count.
//
// Revision 1.15  1998/11/01 00:00:40  ghutchis
// Replaced system calls with htlib/my* functions.
//
// Revision 1.14  1998/10/26 20:43:31  ghutchis
// Fixed bug introduced by Oct 18 change. Authorization will not be cleared.
//
// Revision 1.13  1998/10/21 16:34:19  bergolth
// Added translation of server names. Additional limiting after normalization
// of the URL.
//
// Revision 1.12  1998/10/18 20:37:41  ghutchis
// Fixed database corruption bug and other misc. cleanups.
//
// Revision 1.11  1998/10/09 04:34:06  ghutchis
// Fixed typos
//
// Revision 1.10  1998/10/02 17:17:20  ghutchis
// Added check for docs marked noindex--words aren't indexed anymore
//
// Revision 1.8  1998/09/08 03:29:09  ghutchis
// Clean up for 3.1.0b1.
//
// Revision 1.7  1998/09/07 04:37:16  ghutchis
// Added DocState for documents marked as "noindex".
//
// Revision 1.6  1998/08/11 08:58:31  ghutchis
// Second patch for META description tags. New field in DocDB for the
// desc., space in word DB w/ proper factor.
//
// Revision 1.5  1998/08/06 14:18:32  ghutchis
// Added config option "local_default_doc" for default filename in a local
// directory. Fixed spelling mistake in "elipses" attributes.
//
// Revision 1.4  1998/08/03 16:50:34  ghutchis
// Fixed compiler warnings under -Wall
//
// Revision 1.3  1998/07/09 09:38:59  ghutchis
// Added support for local file digging using patches by Pasi. Patches
// include support for local user (~username) digging.
//
// Revision 1.2  1998/01/05 05:14:16  turtle
// fixed memory leak
//
// Revision 1.1.1.1  1997/02/03 17:11:06  turtle
// Initial CVS
//
// Revision 1.1  1995/12/11 22:46:24  turtle
// This uses the backwards model of only parsing HTML
//
// Revision 1.0  1995/08/18 16:27:37  turtle
// Before change to use Server class
//
//

#include "Retriever.h"
#include "htdig.h"
#include "WordList.h"
#include "URLRef.h"
#include "Server.h"
#include "Parsable.h"
#include "Document.h"
#include <StringList.h>
#include <pwd.h>

static WordList	words;


//*****************************************************************************
// Retriever::Retriever()
//
Retriever::Retriever()
{
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
    valid_punctuation = config["valid_punctuation"];
    minimumWordLength = config.Value("minimum_word_length", 3);
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
// void Retriever::Initial(char *list, int check)
//   Add a single URL to the list of URLs to visit.
//   Since URLs are stored on a per server basis, we first need to find the
//   the correct server to add the URL's path to.
//
void
Retriever::Initial(char *list, int check)
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
	sig = u.signature();
	server = (Server *) servers[sig];
	url = u.get();
	url.lowercase();
	if (!server)
	{
	    server = new Server(u.host(), u.port());
	    servers.Add(sig, server);
	}
	else if (check && visited.Exists(url)) 
	{
	    continue;
	}
     	server->push(u.get(), 0, 0);
	visited.Add(url, 0);
    }
}


//*****************************************************************************
// void Retriever::Initial(List &list, int check)
//
void
Retriever::Initial(List &list, int check)
{
    list.Start_Get();
    String	*str;
    while ((str = (String *) list.Get_Next()))
    {
	Initial(str->get(), check);
    }
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
    
    while (more)
    {
	more = 0;
		
	//
	// Go through all the current servers in sequence.  We take only one
	// URL from each server during this loop.  This ensures that the load
	// on the servers is distributed evenly.
	//
	servers.Start_Get();
	while ( (server = (Server *)servers.Get_NextElement()) )
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
    ref->DocTime(doc.ModTime());
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
    url.lowercase();

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
	char	*p = strtok(t, " \t");
	invalids = new Dictionary;
	while (p)
	{
	    invalids->Add(p, 0);
	    p = strtok(0, " \t");
	}
    }

    static String	url;
    url = u;
    url.lowercase();

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
    int retValue;     // Returned value of findFirst
    int myWhich = 0;    // Item # that matched [0 .. n]
    int myLength = 0;   // Length of the matching value
    retValue = excludes.FindFirst(url, myWhich, myLength);
    if (retValue >= 0)
      {
      if (debug > 2)
	{
	  myWhich++;         // [0 .. n] --> [1 .. n+1]
	  cout << endl <<"   Rejected: Item in the exclude list: item # ";
	  cout << myWhich << " length: " << myLength << endl;
      }
      return FALSE;
    }

    //
    // See if the path extension is in the list of invalid ones
    //
    char	*ext = strrchr(url, '.');
    if (ext && invalids->Exists(ext))
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
    if (limits.FindFirst(url) >= 0)
	return TRUE;

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
    url.lowercase();

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
    if (trackWords)
    {
      String w = word;
      w.remove(valid_punctuation);
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
	if (limitsn.FindFirst(url.get()) >= 0)
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
		char	*sig = url.signature();
		server = (Server *) servers[sig];
		if (!server)
		{
		    //
		    // Hadn't seen this server, yet.  Register it
		    //
		    server = new Server(url.host(), url.port());
		    servers.Add(sig, server);
		}
		//
		// Let's just be sure we're not pushing an empty URL
		//
		if (strlen(url.get()))
		  server->push(url.get(), ref->DocHopCount(), base->get());

		String	temp = url.get();
		temp.lowercase();
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
	if (limitsn.FindFirst(url.get()) >= 0)
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
		char	*sig = url.signature();
		Server	*server = (Server *) servers[sig];
		if (!server)
		{
		    //
		    // Hadn't seen this server, yet.  Register it
		    //
		    server = new Server(url.host(), url.port());
		    servers.Add(sig, server);
		}
		server->push(url.get(), ref->DocHopCount(), base->get());

		String	temp = url.get();
		temp.lowercase();
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

