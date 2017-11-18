//
// Retriever.cc
//
// Retriever: Crawl from a list of URLs and calls appropriate parsers. The
//            parser notifies the Retriever object that it got something
//            (got_* functions) and the Retriever object feed the databases
//            and statistics accordingly.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: Retriever.cc,v 1.94 2004/05/28 13:15:15 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#ifdef _MSC_VER /* _WIN32 */
# include <sys/types.h>
# include <winsock2.h>
#endif


#include "Retriever.h"
#include "htdig.h"
#include "HtWordList.h"
#include "WordRecord.h"
#include "URLRef.h"
#include "Server.h"
#include "Parsable.h"
#include "Document.h"
#include "StringList.h"
#include "WordType.h"
#include "Transport.h"
#include "HtHTTP.h"        // For HTTP statistics
#include "md5.h"
#include "defaults.h"

#ifndef _MSC_VER /* _WIN32 */
#include <pwd.h>
#endif

#include <signal.h>
#include <stdio.h>


static int noSignal;

// no_store_phrases:
// If true, only store first occurrence of each word in a document
static bool no_store_phrases;

//*****************************************************************************
// Retriever::Retriever()
//
Retriever::Retriever(RetrieverLog flags):
words(*(HtConfiguration::config())),
words_to_add (100, 0.75)
{
  HtConfiguration *config = HtConfiguration::config();
  FILE *urls_parsed;

  currenthopcount = 0;
  max_hop_count = config->Value("max_hop_count", 999999);

  no_store_phrases = !config->Boolean("store_phrases");

  //
  // Initialize the flags for the various HTML factors
  //

  // text_factor
  factor[0] = FLAG_TEXT;
  // title_factor
  factor[1] = FLAG_TITLE;
  // heading factor (now generic)
  factor[2] = FLAG_HEADING;
  factor[3] = FLAG_HEADING;
  factor[4] = FLAG_HEADING;
  factor[5] = FLAG_HEADING;
  factor[6] = FLAG_HEADING;
  factor[7] = FLAG_HEADING;
  // img alt text
  //factor[8] = FLAG_KEYWORDS;
  factor[8] = FLAG_TEXT;    // treat alt text as plain text, until it has
  // its own FLAG and factor.
  // keywords factor
  factor[9] = FLAG_KEYWORDS;
  // META description factor
  factor[10] = FLAG_DESCRIPTION;
  factor[11] = FLAG_AUTHOR;

  doc = new Document();
  minimumWordLength = config->Value("minimum_word_length", 3);

  log = flags;
  // if in restart mode
  if (Retriever_noLog != log)
  {
    String filelog = config->Find("url_log");
    char buffer[1024];
    int l;

    urls_parsed = fopen((char *) filelog, "r");
    if (urls_parsed != 0)
    {
      // read all url discovered but not fetched before 
      while (fgets(buffer, sizeof(buffer), urls_parsed))
      {
        l = strlen(buffer);
        buffer[l - 1] = 0;
        Initial(buffer, 2);
      }
      fclose(urls_parsed);
    }
    unlink((char *) filelog);
  }

  check_unique_md5 = config->Boolean("check_unique_md5", 0);
  check_unique_date = config->Boolean("check_unique_date", 0);

  d_md5 = 0;
  if (check_unique_md5)
  {
    d_md5 = Database::getDatabaseInstance(DB_HASH);

    if (d_md5->OpenReadWrite(config->Find("md5_db"), 0666) != OK)
    {
      cerr << "DocumentDB::Open: " << config->Find("md5_db") << " " << strerror(errno) << "\n";
    }
  }

}


//*****************************************************************************
// Retriever::~Retriever()
//
Retriever::~Retriever()
{
  if (d_md5)
    d_md5->Close();
  delete doc;
}


//*****************************************************************************
// void Retriever::setUsernamePassword(char *credentials)
//
void Retriever::setUsernamePassword(const char *credentials)
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
void Retriever::Initial(const String & list, int from)
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
    url = u.get();  // get before  u.signature()  resolves aliases
    server = (Server *) servers[u.signature()];
    if (debug > 2)
      cout << "\t" << from << ":" << (int) log << ":" << url;
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

    if (Retriever_noLog == log || from != 3)
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
// void Retriever::Initial(List &list, int from)
//
void Retriever::Initial(List & list, int from)
{
  list.Start_Get();
  String *str;

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
//
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
    reportError("Cannot install SIGPIPE handler\n");
#endif //_MSC_VER /* _WIN32 */
}


//*****************************************************************************
// static void win32_check_messages
//   Check WIN32 messages!
//
#ifdef _MSC_VER /* _WIN32 */
static void win32_check_messages(void)
{
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
}
#endif //_MSC_VER /* _WIN32 */


//*****************************************************************************
// void Retriever::Start()
//   This is the main loop of the retriever.  We will go through the
//   list of paths stored for each server.  While parsing the
//   retrieved documents, new paths will be added to the servers.  We
//   return if no more paths need to be retrieved.
//
void Retriever::Start()
{
  //
  // Main digger loop.  The todo list should initialy have the start
  // URL and all the URLs which were seen in a previous dig.  The
  // loop will continue as long as there are more URLs to visit.
  //
  int more = 1;
  Server *server;
  URLRef *ref;

  HtConfiguration *config = HtConfiguration::config();

  //  
  // Always sig . The delay bother me but a bad db is worst
  // 
  if (Retriever_noLog != log)
  {
    sig_handlers();
  }
  sig_phandler();
  noSignal = 1;


///////
  //    Main loop. We keep on retrieving until a signal is received
  //    or all the servers' queues are empty.
///////

#ifdef _MSC_VER /* _WIN32 */
    win32_check_messages();
#endif

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

#ifdef _MSC_VER /* _WIN32 */
        win32_check_messages();
#endif

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

#ifdef _MSC_VER /* _WIN32 */
            win32_check_messages();
#endif


      while (((max_connection_requests == -1) ||
           (count < max_connection_requests)) && (ref = server->pop()) && noSignal)
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
          server->delay();  // This will pause if needed
        // and reset the time

#ifdef _MSC_VER /* _WIN32 */
                win32_check_messages();
#endif

      }

#ifdef _MSC_VER /* _WIN32 */
            win32_check_messages();
#endif

    }
  }

#ifdef _MSC_VER /* _WIN32 */
    win32_check_messages();
#endif


  // if we exited on signal 
  if (Retriever_noLog != log && !noSignal)
  {
    FILE *urls_parsed;
    String filelog = config->Find("url_log");
    // save url seen but not fetched
    urls_parsed = fopen((char *) filelog, "w");
    if (0 == urls_parsed)
    {
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
  words.Close();
}


//*****************************************************************************
// void Retriever::parse_url(URLRef &urlRef)
//
void Retriever::parse_url(URLRef & urlRef)
{
  HtConfiguration *config = HtConfiguration::config();
  URL url;
  DocumentRef *ref;
  int old_document;
  time_t date;
  static int index = 0;
  static int local_urls_only = config->Boolean("local_urls_only");
  static int mark_dead_servers = config->Boolean("ignore_dead_servers");
  Server *server;

  url.parse(urlRef.GetURL().get());

  currenthopcount = urlRef.GetHopCount();

  ref = docs[url.get()];    // It might be nice to have just an Exists() here
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
    else          // we haven't retrieved it yet, so we only have the first link
      old_document = 0;
    ref->DocBackLinks(ref->DocBackLinks() + 1);  // we had a new link
    ref->DocAccessed(time(0));
    ref->DocState(Reference_normal);
    currenthopcount = ref->DocHopCount();
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

  word_context.DocID(ref->DocID());

  if (debug > 0)
  {
    //
    // Display progress
    //
    cout << index++ << ':' << current_id << ':' << currenthopcount << ':' << url.get() << ": ";
    cout.flush();
  }

  // Reset the document to clean out any old data
  doc->Reset();
  doc->Url(url.get());
  doc->Referer(urlRef.GetReferer().get());

  base = doc->Url();

  // Retrieve document, first trying local file access if possible.
  Transport::DocStatus status;
  server = (Server *) servers[url.signature()];
  StringList *local_filenames = GetLocal(url.get());
  if (local_filenames)
  {
    if (debug > 1)
      cout << "Trying local files" << endl;
    status = doc->RetrieveLocal(date, local_filenames);
    if (status == Transport::Document_not_local)
    {
      if (debug > 1)
        cout << "Local retrieval failed, trying HTTP" << endl;
      if (server && !server->IsDead() && !local_urls_only)
        status = doc->Retrieve(server, date);
      else
        status = Transport::Document_no_host;
    }
    delete local_filenames;
  }
  else if (server && !server->IsDead() && !local_urls_only)
    status = doc->Retrieve(server, date);
  else
    status = Transport::Document_no_host;

  current_ref = ref;

  //
  // Determine what to do by looking at the status code returned by
  // the Document retrieval process.
  //

  String shash;
  String sx;
  char bhash[16];
  time_t ddate;

  switch (status)
  {

  case Transport::Document_ok:
    trackWords = 1;

    if (check_unique_md5)
    {
      if (doc->StoredLength() > 0)
      {
        if (check_unique_date)
        {
          ddate = doc->ModTime();
          if (ddate < time(NULL) - 10)
          {    // Unknown date was set to current time
            md5(bhash, doc->Contents(), doc->StoredLength(), &ddate, debug);
          }
          else
          {
            md5(bhash, doc->Contents(), doc->StoredLength(), 0, debug);
          }
        }
        else
          md5(bhash, doc->Contents(), doc->StoredLength(), 0, debug);

        shash.append(bhash, MD5_LENGTH);
        d_md5->Get(shash, sx);

        if (!sx.empty())
        {
          if (debug > 1)
          {
            cout << " Detected duplicate by md5 hash" << endl;
          }
          words.Skip();
          break; // Duplicate - don't index
        }
        else
        {
          d_md5->Put(shash, "x");
        }

      }
    }

    if (old_document)
    {
      if (doc->ModTime() == ref->DocTime())
      {
        words.Skip();
        if (debug)
          cout << " retrieved but not changed" << endl;
        words.Skip();
        break;
      }
      //
      // Since we already had a record of this document and
      // we were able to retrieve it, it must have changed
      // since the last time we scanned it.  This means that
      // we need to assign a new document ID to it and mark
      // the old one as obsolete.
      //
      words.Skip();
      int backlinks = ref->DocBackLinks();
      ref->DocState(Reference_obsolete);
      docs.Add(*ref);
      delete ref;

      current_id = docs.NextDocID();
      word_context.DocID(current_id);
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
    {
      if (debug > 1)
        cout << " ( " << ref->DocURL() << " ignored)";
      words.Skip();
    }
    else
      words.Flush();
    if (debug)
      cout << " size = " << doc->Length() << endl;

    if (urls_seen)
    {
      fprintf(urls_seen, "%s|%d|%s|%d|%d|1\n",
           (const char *) url.get(), doc->Length(), doc->ContentType(),
           (int) doc->ModTime(), currenthopcount);
    }
    break;

  case Transport::Document_not_changed:
    if (debug)
      cout << " not changed" << endl;
    words.Skip();
    break;

  case Transport::Document_not_found:
    ref->DocState(Reference_not_found);
    if (debug)
      cout << " not found" << endl;
    recordNotFound(url.get(), urlRef.GetReferer().get(), Transport::Document_not_found);
    words.Skip();
    break;

  case Transport::Document_no_host:
    ref->DocState(Reference_not_found);
    if (debug)
      cout << " host not found" << endl;
    recordNotFound(url.get(), urlRef.GetReferer().get(), Transport::Document_no_host);
    words.Skip();

    // Mark the server as being down
    if (server && mark_dead_servers)
      server->IsDead(1);
    break;

  case Transport::Document_no_port:
    ref->DocState(Reference_not_found);
    if (debug)
      cout << " host not found (port)" << endl;
    recordNotFound(url.get(), urlRef.GetReferer().get(), Transport::Document_no_port);
    words.Skip();

    // Mark the server as being down
    if (server && mark_dead_servers)
      server->IsDead(1);
    break;

  case Transport::Document_not_parsable:
    ref->DocState(Reference_noindex);
    if (debug)
      cout << " not Parsable" << endl;
    words.Skip();
    break;

  case Transport::Document_redirect:
    if (debug)
      cout << " redirect" << endl;
    ref->DocState(Reference_obsolete);
    words.Skip();
    got_redirect(doc->Redirected(), ref, (urlRef.GetReferer()).get());
    break;

  case Transport::Document_not_authorized:
    ref->DocState(Reference_not_found);
    if (debug)
      cout << " not authorized" << endl;
    words.Skip();
    break;

  case Transport::Document_not_local:
    ref->DocState(Reference_not_found);
    if (debug)
      cout << " not local" << endl;
    words.Skip();
    break;

  case Transport::Document_no_header:
    ref->DocState(Reference_not_found);
    if (debug)
      cout << " no header" << endl;
    words.Skip();
    break;

  case Transport::Document_connection_down:
    ref->DocState(Reference_not_found);
    if (debug)
      cout << " connection down" << endl;
    words.Skip();
    break;

  case Transport::Document_no_connection:
    ref->DocState(Reference_not_found);
    if (debug)
      cout << " no connection" << endl;
    words.Skip();
    break;

  case Transport::Document_not_recognized_service:
    ref->DocState(Reference_not_found);
    if (debug)
      cout << " service not recognized" << endl;

    // Mark the server as being down
    if (server && mark_dead_servers)
      server->IsDead(1);
    words.Skip();
    break;

  case Transport::Document_other_error:
    ref->DocState(Reference_not_found);
    if (debug)
      cout << " other error" << endl;
    words.Skip();
    break;
  }
  docs.Add(*ref);
  delete ref;
}


//*****************************************************************************
// void Retriever::RetrievedDocument(Document &doc, const String &url, DocumentRef *ref)
//   We found a document that needs to be parsed.  Since we don't know the
//   document type, we'll let the Document itself return an appropriate
//   Parsable object which we can call upon to parse the document contents.
//
void Retriever::RetrievedDocument(Document & doc, const String & url, DocumentRef * ref)
{
  n_links = 0;
  current_ref = ref;
  current_title = 0;
  word_context.Anchor(0);
  current_time = 0;
  current_head = 0;
  current_meta_dsc = 0;

  //
  // Create a parser object and let it have a go at the document.
  // We will pass ourselves as a callback object for all the got_*()
  // routines.
  // This will generate the Parsable object as a specific parser
  //
  Parsable *parsable = doc.getParsable();
  if (parsable)
    parsable->parse(*this, *base);
  else
  {            // If we didn't get a parser, then we should get rid of this!
    ref->DocState(Reference_noindex);
    return;
  }

  // If just storing the first occurrence of each word in a document,
  // we must now flush the words we saw in that document
  if (no_store_phrases)
  {
      DictionaryCursor cursor;
      char *key;
      HtWordReference wordRef;
      for (words_to_add.Start_Get (cursor);
        (key = words_to_add.Get_Next(cursor)); )
      {
    word_entry *entry = (word_entry*) (words_to_add [key]);

    wordRef.Location(entry->location);
    wordRef.Flags(entry->flags);
    wordRef.Word(key);
    words.Replace(WordReference::Merge(wordRef, entry->context));
    // How do I clean up properly?
    delete entry;
      }
      words_to_add.Release ();
  }

  //
  // We don't need to dispose of the parsable object since it will
  // automatically be reused.
  //

  //
  // Update the document reference
  //
  ref->DocHead((char *) current_head);
  ref->DocMetaDsc((char *) current_meta_dsc);
  if (current_time == 0)
    ref->DocTime(doc.ModTime());
  else
    ref->DocTime(current_time);
  ref->DocTitle((char *) current_title);
  ref->DocSize(doc.Length());
  ref->DocAccessed(time(0));
  ref->DocLinks(n_links);
}


//*****************************************************************************
// int Retriever::Need2Get(const String &u)
//   Return TRUE if we need to retrieve the given url.  This will
//   check the list of urls we have already visited.
//
int Retriever::Need2Get(const String & u)
{
  static String url;
  url = u;

  return !visited.Exists(url);
}



//*****************************************************************************
// int Retriever::IsValidURL(const String &u)
//   Return TRUE if we need to retrieve the given url.  We will check
//   for limits here.
//
int Retriever::IsValidURL(const String & u)
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
  int parm = urlpath.indexOf('?');  // chop off URL parameter
  if (parm >= 0)
    urlpath.chop(urlpath.length() - parm);
  ext = strrchr((char *) urlpath.get(), '.');
  String lowerext;
  if (ext && strchr(ext, '/'))  // Ignore a dot if it's not in the
    ext = NULL;      // final component of the path.
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
//  aUrl.normalize();
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
// StringList* Retriever::GetLocal(const String &url)
//   Returns a list of strings containing the (possible) local filenames
//   of the given url, or 0 if it's definitely not local.
//   THE CALLER MUST FREE THE STRINGLIST AFTER USE!
//   Returned strings are not hex encoded.
//
StringList *Retriever::GetLocal(const String & strurl)
{
  HtConfiguration *config = HtConfiguration::config();
  static StringList *prefixes = 0;
  String url = strurl;

  static StringList *paths = 0;
  StringList *defaultdocs = 0;
  URL aUrl(url);
  url = aUrl.get();      // make sure we look at a parsed URL

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
// StringList* Retriever::GetLocalUser(const String &url, StringList *defaultdocs)
//   If the URL has ~user part, return a list of strings containing the
//   (possible) local filenames of the given url, or 0 if it's
//   definitely not local.
//   THE CALLER MUST FREE THE STRINGLIST AFTER USE!
//
StringList *Retriever::GetLocalUser(const String & url, StringList * defaultdocs)
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
// int Retriever::IsLocalURL(const String &url)
//   Returns 1 if the given url has a (possible) local filename
//   or 0 if it's definitely not local.
//
int Retriever::IsLocalURL(const String & url)
{
  int ret;

  StringList *local_filename = GetLocal(url);
  ret = (local_filename != 0);
  if (local_filename)
    delete local_filename;

  return ret;
}


//*****************************************************************************
// void Retriever::got_word(char *word, int location, int heading)
//   The location is normalized to be in the range 0 - 1000.
//
void Retriever::got_word(const char *word, int location, int heading)
{
  if (debug > 3)
    cout << "word: " << word << '@' << location << endl;
  if (heading >= (int) (sizeof(factor) / sizeof(factor[0])) || heading < 0)
    heading = 0;      // Assume it's just normal text
  if (trackWords && strlen(word) >= (unsigned int) minimumWordLength)
  {
    String w = word;
    HtWordReference wordRef;

    if (no_store_phrases)
    {
        // Add new word, or mark existing word as also being at
        // this heading level
        word_entry *entry;
        if ((entry = (word_entry*)words_to_add.Find (w)) == NULL)
        {
      words_to_add.Add(w, new word_entry (location, factor[heading], word_context));
        } else
        {
      entry->flags |= factor[heading];
        }
    } else
    {
        wordRef.Location(location);
        wordRef.Flags(factor[heading]);
        wordRef.Word(w);
        words.Replace(WordReference::Merge(wordRef, word_context));
    }

    // Check for compound words...
    String parts = word;
    int added;
    int nparts = 1;
    do
    {
      added = 0;
      char *start = parts.get();
      char *punctp = 0, *nextp = 0, *p;
      char punct;
      int n;
      while (*start)
      {
        p = start;
        for (n = 0; n < nparts; n++)
        {
          while (HtIsStrictWordChar((unsigned char) *p))
            p++;
          punctp = p;
          if (!*punctp && n + 1 < nparts)
            break;
          while (*p && !HtIsStrictWordChar((unsigned char) *p))
            p++;
          if (n == 0)
            nextp = p;
        }
        if (n < nparts)
          break;
        punct = *punctp;
        *punctp = '\0';
        if (*start && (*p || start > parts.get()))
        {
          w = start;
          HtStripPunctuation(w);
          if (w.length() >= minimumWordLength)
          {
                  if (no_store_phrases)
            {
                // Add new word, or mark existing word as also being at
                // this heading level
                word_entry *entry;
                if ((entry = (word_entry*)words_to_add.Find (w)) == NULL)
                {
              words_to_add.Add(w, new word_entry (location, factor[heading], word_context));
                } else
                {
              entry->flags |= factor[heading];
                }
            } else
            {
                wordRef.Word(w);
                words.Replace(WordReference::Merge(wordRef, word_context));
            }
            if (debug > 3)
              cout << "word part: " << start << '@' << location << endl;
          }
          added++;
        }
        start = nextp;
        *punctp = punct;
      }
      nparts++;
    }
    while (added > 2);
  }
}


//*****************************************************************************
// void Retriever::got_title(const char *title)
//
void Retriever::got_title(const char *title)
{
  if (debug > 1)
    cout << "\ntitle: " << title << endl;
  current_title = title;
}


//*****************************************************************************
// void Retriever::got_author(const char *e)
//
void Retriever::got_author(const char *author)
{
  if (debug > 1)
    cout << "\nauthor: " << author << endl;
  current_ref->DocAuthor(author);
}


//*****************************************************************************
// void Retriever::got_time(const char *time)
//
void Retriever::got_time(const char *time)
{
  HtDateTime new_time(current_time);

  if (debug > 1)
    cout << "\ntime: " << time << endl;

  //
  // As defined by the Dublin Core, this should be YYYY-MM-DD
  // In the future, we'll need to deal with the scheme portion
  //  in case someone picks a different format.
  //
  new_time.SetFTime(time, "%Y-%m-%d");
  current_time = new_time.GetTime_t();

  // If we can't convert it, current_time stays the same and we get
  // the default--the date returned by the server...
}

//*****************************************************************************
// void Retriever::got_anchor(const char *anchor)
//
void Retriever::got_anchor(const char *anchor)
{
  if (debug > 2)
    cout << "anchor: " << anchor << endl;
  current_ref->AddAnchor(anchor);
  word_context.Anchor(word_context.Anchor() + 1);
}


//*****************************************************************************
// void Retriever::got_image(const char *src)
//
void Retriever::got_image(const char *src)
{
  URL url(src, *base);
  const char *image = (const char *) url.get();

  if (debug > 2)
    cout << "image: " << image << endl;

  if (images_seen)
    fprintf(images_seen, "%s\n", image);
}


//*****************************************************************************
//
void Retriever::got_href(URL & url, const char *description, int hops)
{
  DocumentRef *ref = 0;
  Server *server = 0;
  int valid_url_code = 0;

  // Rewrite the URL (if need be) before we do anything to it.
  url.rewrite();

  if (debug > 2)
    cout << "href: " << url.get() << " (" << description << ')' << endl;

  n_links++;

  if (urls_seen)
    fprintf(urls_seen, "%s\n", (const char *) url.get());

  //
  // Check if this URL falls within the valid range of URLs.
  //
  valid_url_code = IsValidURL(url.get());
  if (valid_url_code > 0)
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
      current_ref->AddDescription(description, words);
    }
    else
    {

      //
      // First add it to the document database
      //
      ref = docs[url.get()];
      // if ref exists we have to call AddDescription even
      // if max_hop_count is reached
      if (!ref && currenthopcount + hops > max_hop_count)
        return;

      if (!ref)
      {
        //
        // Didn't see this one, yet.  Create a new reference
        // for it with a unique document ID
        //
        ref = new DocumentRef;
        ref->DocID(docs.NextDocID());
        ref->DocHopCount(currenthopcount + hops);
        ref->DocURL(url.get());
      }
      ref->DocBackLinks(ref->DocBackLinks() + 1);  // This one!
      ref->AddDescription(description, words);

      //
      // If the dig is restricting by hop count, perform the check here 
      // too
      if (currenthopcount + hops > max_hop_count)
      {
        delete ref;
        return;
      }

      if (ref->DocHopCount() > currenthopcount + hops)
        ref->DocHopCount(currenthopcount + hops);

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
          String robotsURL = url.signature();
          robotsURL << "robots.txt";
          StringList *localRobotsFile = GetLocal(robotsURL.get());

          server = new Server(url, localRobotsFile);
          servers.Add(url.signature(), server);
          delete localRobotsFile;
        }
        //
        // Let's just be sure we're not pushing an empty URL
        //
        if (strlen(url.get()))
          server->push(url.get(), ref->DocHopCount(), base->get(), IsLocalURL(url.get()));

        String temp = url.get();
        visited.Add(temp, 0);
        if (debug)
          cout << '+';
      }
      else if (debug)
        cout << '*';
      delete ref;
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

    if (urls_seen)
    {
      fprintf(urls_seen, "%s|||||%d\n", (const char *) url.get(), valid_url_code);
    }

  }
  if (debug)
    cout.flush();
}


//*****************************************************************************
// void Retriever::got_redirect(const char *new_url, DocumentRef *old_ref)
//
void Retriever::got_redirect(const char *new_url, DocumentRef * old_ref, const char *referer)
{
  // First we must piece together the new URL, which may be relative
  URL parent(old_ref->DocURL());
  URL url(new_url, parent);

  // Rewrite the URL (if need be) before we do anything to it.
  url.rewrite();

  if (debug > 2)
    cout << "redirect: " << url.get() << endl;

  n_links++;

  if (urls_seen)
    fprintf(urls_seen, "%s\n", (const char *) url.get());

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
      cout << "resolving '" << url.get() << "'\n";
      cout.flush();
    }

    url.normalize();
    //
    // First add it to the document database
    //
    DocumentRef *ref = docs[url.get()];
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
    List *d = old_ref->Descriptions();
    if (d)
    {
      d->Start_Get();
      String *str;
      while ((str = (String *) d->Get_Next()))
      {
        ref->AddDescription(str->get(), words);
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
        server->push(url.get(), ref->DocHopCount(), base->get(), IsLocalURL(url.get()), 0);
      else
        server->push(url.get(), ref->DocHopCount(), referer, IsLocalURL(url.get()), 0);

      String temp = url.get();
      visited.Add(temp, 0);
    }

    delete ref;
  }
}


//*****************************************************************************
// void Retriever::got_head(const char *head)
//
void Retriever::got_head(const char *head)
{
  if (debug > 4)
    cout << "head: " << head << endl;
  current_head = head;
}

//*****************************************************************************
// void Retriever::got_meta_dsc(const char *md)
//
void Retriever::got_meta_dsc(const char *md)
{
  if (debug > 4)
    cout << "meta description: " << md << endl;
  current_meta_dsc = md;
}


//*****************************************************************************
// void Retriever::got_meta_email(const char *e)
//
void Retriever::got_meta_email(const char *e)
{
  if (debug > 1)
    cout << "\nmeta email: " << e << endl;
  current_ref->DocEmail(e);
}


//*****************************************************************************
// void Retriever::got_meta_notification(const char *e)
//
void Retriever::got_meta_notification(const char *e)
{
  if (debug > 1)
    cout << "\nmeta notification date: " << e << endl;
  current_ref->DocNotification(e);
}


//*****************************************************************************
// void Retriever::got_meta_subject(const char *e)
//
void Retriever::got_meta_subject(const char *e)
{
  if (debug > 1)
    cout << "\nmeta subect: " << e << endl;
  current_ref->DocSubject(e);
}


//*****************************************************************************
// void Retriever::got_noindex()
//
void Retriever::got_noindex()
{
  if (debug > 1)
    cout << "\nMETA ROBOT: Noindex " << current_ref->DocURL() << endl;
  current_ref->DocState(Reference_noindex);
}


//*****************************************************************************
//
void Retriever::recordNotFound(const String & url, const String & referer, int reason)
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
// void Retriever::ReportStatistics(char *name)
//
void Retriever::ReportStatistics(const String & name)
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

  HtHTTP::ShowStatistics(cout) << endl;

}
