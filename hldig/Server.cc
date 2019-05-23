//
// Server.cc
//
// Server: A class to keep track of server specific information.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: Server.cc,v 1.29 2004/05/28 13:15:16 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "hlconfig.h"
#endif /* HAVE_CONFIG_H */

#include "hldig.h"
#include "Server.h"
#include "good_strtok.h"
#include "htString.h"
#include "URL.h"
#include "Document.h"
#include "URLRef.h"
#include "Transport.h"
#include "HtHTTP.h"             // for checking persistent connections
#include "StringList.h"

#include <ctype.h>
#include "defaults.h"


//*****************************************************************************
// Server::Server(URL u, StringList *local_robots_files)
//  u is the base URL for this server
//
Server::Server (URL u, StringList * local_robots_files):
_host (u.host ()),
_port (u.port ()), _bad_server (0), _documents (0), _accept_language (0)
{
  HtConfiguration *config = HtConfiguration::config ();
  if (debug)
    cout << endl << "New server: " << _host << ", " << _port << endl;

  // We take it from the configuration
  _persistent_connections =
    config->Boolean ("server", _host.get (), "persistent_connections");
  _head_before_get =
    config->Boolean ("server", _host.get (), "head_before_get");

  _max_documents = config->Value ("server", _host.get (), "server_max_docs");
  _connection_space =
    config->Value ("server", _host.get (), "server_wait_time");
  _user_agent = config->Find ("server", _host.get (), "user_agent");
  _disable_cookies =
    config->Boolean ("server", _host.get (), "disable_cookies");

  // Accept-Language directive
  StringList _accept_language_list (config->Find ("server", _host.get (),
                                                  "accept_language"), " \t");

  _accept_language.trunc ();    // maybe not needed

  for (int i = 0; i < _accept_language_list.Count (); i++)
  {
    if (i > 0)
      _accept_language << ",";  // for multiple choices

    _accept_language << _accept_language_list[i];
  }

  // Timeout setting
  _timeout = config->Value ("server", _host.get (), "timeout");

  // Number of consecutive attempts to establish a TCP connection
  _tcp_max_retries =
    config->Value ("server", _host.get (), "tcp_max_retries");

  // Seconds to wait after a timeout occurs
  _tcp_wait_time = config->Value ("server", _host.get (), "tcp_wait_time");


  if (debug > 1)
  {
    cout << " - Persistent connections: " <<
      (_persistent_connections ? "enabled" : "disabled") << endl;

    cout << " - HEAD before GET: " <<
      (_head_before_get ? "enabled" : "disabled") << endl;

    cout << " - Timeout: " << _timeout << endl;
    cout << " - Connection space: " << _connection_space << endl;
    cout << " - Max Documents: " << _max_documents << endl;
    cout << " - TCP retries: " << _tcp_max_retries << endl;
    cout << " - TCP wait time: " << _tcp_wait_time << endl;
    cout << " - Accept-Language: " << _accept_language << endl;

  }

  _last_connection.SettoNow (); // For getting robots.txt

  if (strcmp (u.service (), "http") == 0
      || strcmp (u.service (), "https") == 0)
  {
    //
    // Attempt to get a robots.txt file from the specified server
    //
    String url;
    url.trunc ();

    if (debug > 1)
      cout << "Trying to retrieve robots.txt file" << endl;
    url << u.signature () << "robots.txt";

    static int local_urls_only = config->Boolean ("local_urls_only");
    time_t timeZero = 0;        // Right now we want to get this every time
    Document doc (url, 0);
    Transport::DocStatus status;
    if (local_robots_files)
    {
      if (debug > 1)
        cout << "Trying local files" << endl;
      status = doc.RetrieveLocal (timeZero, local_robots_files);
      if (status == Transport::Document_not_local)
      {
        if (local_urls_only)
          status = Transport::Document_not_found;
        else
        {
          if (debug > 1)
            cout << "Local retrieval failed, trying HTTP" << endl;
          status = doc.Retrieve (this, timeZero);
        }
      }
    }
    else if (!local_urls_only)
    {
      status = doc.Retrieve (this, timeZero);

      // Let's check if persistent connections are both
      // allowed by the configuration and possible after
      // having requested the robots.txt file.

      HtHTTP *http;
      if (IsPersistentConnectionAllowed () && (http = doc.GetHTTPHandler ()))
      {
        if (!http->isPersistentConnectionPossible ())
          _persistent_connections = 0;  // not possible. Let's disable
        // them on this server.
      }

    }
    else
      status = Transport::Document_not_found;

    switch (status)
    {
    case Transport::Document_ok:
      //
      // Found a robots.txt file.  Go parse it.
      //
      robotstxt (doc);
      break;

    case Transport::Document_not_found:
    case Transport::Document_not_parsable:
    case Transport::Document_redirect:
    case Transport::Document_not_authorized:
      //
      // These cases are for when there is no robots.txt file.
      // We will just go on happily without restrictions
      //
      break;

    case Transport::Document_no_host:
    default:
      //
      // In all other cases the server could not be reached.
      // We will remember this fact so that no more attempts to
      // contact this server will be made.
      //
      _bad_server = 1;
      break;
    }                           // end switch
  }                             // end if (http || https)
}

// Copy constructor
Server::Server (const Server & rhs):
_host (rhs._host),
_port (rhs._port),
_bad_server (rhs._bad_server),
_connection_space (rhs._connection_space),
_last_connection (rhs._last_connection),
_paths (rhs._paths),
_disallow (rhs._disallow),
_documents (rhs._documents),
_max_documents (rhs._max_documents),
_persistent_connections (rhs._persistent_connections),
_head_before_get (rhs._head_before_get),
_disable_cookies (rhs._disable_cookies),
_timeout (rhs._timeout),
_tcp_wait_time (rhs._tcp_wait_time),
_tcp_max_retries (rhs._tcp_max_retries),
_user_agent (rhs._user_agent),
_accept_language (rhs._accept_language)
{
}


//*****************************************************************************
// Server::~Server()
//
Server::~Server ()
{
}


//*****************************************************************************
// void Server::robotstxt(Document &doc)
//   This will parse the robots.txt file which is contained in the document.
//
void
Server::robotstxt (Document & doc)
{
  HtConfiguration *config = HtConfiguration::config ();
  String contents = doc.Contents ();
  int length;
  int pay_attention = 0;
  String pattern;
  String myname = config->Find ("server", _host.get (), "robotstxt_name");
  int seen_myname = 0;
  char *name, *rest;

  if (debug > 1)
    cout << "Parsing robots.txt file using myname = " << myname << "\n";

  //
  // Go through the lines in the file and determine if we need to
  // pay attention to them
  //
  for (char *line = strtok (contents, "\r\n"); line;
       line = strtok (0, "\r\n"))
  {
    if (debug > 2)
      cout << "Robots.txt line: " << line << endl;

    //
    // Strip comments
    //
    if (strchr (line, '#'))
    {
      *(strchr (line, '#')) = '\0';
    }

    name = good_strtok (line, ':');
    if (!name)
      continue;
    while (name && isspace (*name))
      name++;
    rest = good_strtok (NULL, '\r');
    if (!rest)
      rest = (char *)"";

    while (rest && isspace (*rest))
      rest++;

    length = strlen (rest);
    if (length > 0)
    {
      while (length > 0 && isspace (rest[length - 1]))
        length--;
      rest[length] = '\0';
    }

    if (mystrcasecmp (name, "user-agent") == 0)
    {
      if (debug > 1)
        cout << "Found 'user-agent' line: " << rest << endl;

      if (*rest == '*' && !seen_myname)
      {
        //
        // This matches all search engines...
        //
        pay_attention = 1;
      }
      else if (mystrncasecmp (rest, (char *) myname, myname.length ()) == 0)
      {
        //
        // This is for us!  This will override any previous patterns
        // that may have been set.
        //
        if (!seen_myname)       // only take first section with our name
        {
          seen_myname = 1;
          pay_attention = 1;
          pattern = 0;          // ignore previous User-agent: *
        }
        else
          pay_attention = 0;
      }
      else
      {
        //
        // This doesn't concern us
        //
        pay_attention = 0;
      }
    }
    else if (pay_attention && mystrcasecmp (name, "disallow") == 0)
    {
      if (debug > 1)
        cout << "Found 'disallow' line: " << rest << endl;

      //
      // Add this path to our list to ignore
      //
      if (*rest)
      {
        if (pattern.length ())
          pattern << '|';
        while (*rest)
        {
          if (strchr ("^.[$()|*+?{\\", *rest))
            pattern << '\\';
          pattern << *rest++;
        }
      }
    }
    //
    // Ignore anything else (comments)
    //
  }

  //
  // Compile the pattern (if any...)
  //
  if (debug > 1)
    cout << "Pattern: " << pattern << endl;

  // Empty "disallow" allows all, so don't make entry which matches all.
  if (!pattern.empty ())
  {
    String fullpatt = "^[^:]*://[^/]*(";
    fullpatt << pattern << ')';
    if (pattern.length () == 0)
      fullpatt = "";
    _disallow.set (fullpatt, config->Boolean ("case_sensitive"));
  }
}


//*****************************************************************************
// void Server::push(String &path, int hopcount, char *referer, int local, int newDoc)
//
void
Server::push (const String & path, int hopcount, const String & referer,
              int local, int newDoc)
{
  if (_bad_server && !local)
    return;

  if (IsDisallowed (path) != 0)
  {
    if (debug > 2)
      cout << endl << "   Rejected: forbidden by server robots.txt!";

    return;
  }

  // We use -1 as no limit, but we also don't want
  // to forbid redirects from old places
  if (_max_documents != -1 && newDoc && _documents >= _max_documents)
  {
    if (debug > 2)              // Hey! we only want to get max_docs
      cout << "Limit of " << _max_documents << " reached for " << _host <<
        endl;

    return;
  }

  URLRef *ref = new URLRef ();
  ref->SetURL (path);
  ref->SetHopCount (hopcount);
  ref->SetReferer (referer);
  _paths.Add (ref);

  if (newDoc)
    _documents++;

//     cout << "***** pushing '" << path << "' with '" << referer << "'\n";
}


//*****************************************************************************
// URLRef *Server::pop()
//
URLRef *
Server::pop ()
{
  URLRef *ref = (URLRef *) _paths.Remove ();

  if (!ref)
    return 0;

  return ref;
}


//*****************************************************************************
// void Server::delay()
//
// Keeps track of how long it's been since we've seen this server
// and call sleep if necessary
//
void
Server::delay ()
{
  HtDateTime now;

  int time_taken = HtDateTime::GetDiff (now, _last_connection); // arg1-arg2 > 0

  if (time_taken < _connection_space)
    sleep (_connection_space - time_taken);

  now.SettoNow ();
  _last_connection = now;       // Reset the clock for the next delay!

  return;
}


//*****************************************************************************
// void Server::reportStatistics(String &out, char *name)
//
void
Server::reportStatistics (String & out, char *name)
{
  out << name << " " << _host << ":" << _port;
  out << " " << _documents << _(" document");
  if (_documents != 1)
    out << "s";
}
