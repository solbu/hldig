//
// Retriever.cc
//
// Implementation of Retriever
//
// $Log: Retriever.cc,v $
// Revision 1.4  1998/08/03 16:50:34  ghutchis
//
// Fixed compiler warnings under -Wall
//
// Revision 1.3  1998/07/09 09:38:59  ghutchis
//
//
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
    factor[8] = factor[9] = 0;
    factor[10] = config.Double("keywords_factor");
	
    //
    // Open the file to which we will append words.
    //
    String	filename = config["word_list"];
    words.WordTempFile(filename);
    words.BadWordFile(config["bad_word_list"]);

    doc = new Document();
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
// void Retriever::Initial(char *list)
//   Add a signel URL to the list of URLs to visit.
//   Since URLs are stored on a per server basis, we first need to find the
//   the correct server to add the URL's path to.
//
void
Retriever::Initial(char *list)
{
    //
    // Split the list of urls up into individual urls.
    //
    StringList	tokens(list, " \t");
    String	sig;
    Server	*server;

    for (int i = 0; i < tokens.Count(); i++)
    {
	URL	u(tokens[i]);
	sig = u.signature();
	server = (Server *) servers[sig];
	if (!server)
	{
	    server = new Server(u.host(), u.port());
	    servers.Add(sig, server);
	}
	server->push(u.get(), 0, 0);
	sig = u.get();
	sig.lowercase();
	visited.Add(sig, 0);
    }
}


//*****************************************************************************
// void Retriever::Initial(List &list)
//
void
Retriever::Initial(List &list)
{
    list.Start_Get();
    String	*str;
    while ((str = (String *) list.Get_Next()))
    {
	Initial(str->get());
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
    char	*server_sig;
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
	while ((server_sig = servers.Get_Next()))
	{
	    if (debug > 1)
		cout << "pick: " << server_sig << ", # servers = " <<
		    servers.Count() << endl;
	    server = (Server *) servers[server_sig];
	    ref = server->pop();
	    if (!ref)
		continue;				// Nothing on this server
			
	    //
	    // We have a URL to index, now.  We need to register the
	    // fact that we are not done yet by setting the 'more'
	    // variable.
	    //
	    more = 1;

	    //
	    // Deal with the actual URL.
	    //
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
	else
	    old_document = 0;
	ref->DocAccessed(time(0));
	ref->DocState(Reference_normal);
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

    if (status == Document::Document_not_found)
    {
	//
	// Maybe the URL we gave was incomplete.  See if adding a '/'
	// will help.
	//
	String tempurl = doc->Url()->get();
	if (tempurl.last() != '/')
	{
	    tempurl << '/';
	    doc->Url(tempurl);
	    base = doc->Url();
	    status = doc->RetrieveHTTP(date);
	}
    }
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
		delete ref;
		current_id = docs.NextDocID();
		words.DocumentID(current_id);
		ref = new DocumentRef;
		ref->DocID(current_id);
		ref->DocURL(url.get());
		ref->DocState(Reference_normal);
		ref->DocAccessed(time(0));
		if (debug)
		    cout << " (changed) ";
	    }
	    RetrievedDocument(*doc, url.get(), ref);
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

    //
    // Create a parser object and let it have a go at the document.
    // We will pass ourselves as a callback object for all the got_*()
    // routines.
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
	return FALSE;

    //
    // If the URL contains any of the patterns in the exclude list,
    // mark it as invalid
    //
    if (excludes.FindFirst(url) >= 0)
	return FALSE;

    //
    // See if the path extension is in the list of invalid ones
    //
    char	*ext = strrchr(url, '.');
    if (ext && invalids->Exists(ext))
	return FALSE;

    //
    // If any of the limits are met, we allow the URL
    //
    if (limits.FindFirst(url) >= 0)
	return TRUE;
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
        if (strncasecmp(*prefix, url, prefix->length()) == 0)
	{
	    int l = strlen(url)-prefix->length()+path->length()+4;
	    String *local = new String(*path, l);
	    *local += &url[prefix->length()];
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
        if (strcasecmp(*prefix, tmp) != 0)
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
	words.Word(word, location, current_anchor_number, factor[heading]);
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
    // If the dig is restricting by hop count, perform the check here.
    //
    if (currenthopcount + 1 > max_hop_count)
	return;

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
	if (IsValidURL(url.get()))
	{
	    //
	    // First add it to the document database
	    //
	    ref = docs[url.get()];
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
	    ref->DocURL(url.get());
	    ref->AddDescription(description);
	    if (ref->DocHopCount() > currenthopcount + 1)
		ref->DocHopCount(currenthopcount + 1);

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
	if (IsValidURL(url.get()))
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
		ref->DocHopCount(currenthopcount + 1);
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
    char		*server_sig;
    String		buffer;
    StringList	results;
    String		newname = name;

    newname << ":    ";
	
    servers.Start_Get();
    while ((server_sig = servers.Get_Next()))
    {
	buffer = 0;
	server = (Server *) servers[server_sig];
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

