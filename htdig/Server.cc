//
// Server.cc
//
// Server: A class to keep track of server specific information.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Server.cc,v 1.17.2.16 2000/09/03 21:55:08 ghutchis Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "htdig.h"
#include "Server.h"
#include "good_strtok.h"
#include "htString.h"
#include "URL.h"
#include "Document.h"
#include "URLRef.h"
#include "Transport.h"
#include "HtHTTP.h"    // for checking persistent connections

#include <ctype.h>


//*****************************************************************************
// Server::Server(URL u, StringList *local_robots_files)
//  u is the base URL for this server
//
Server::Server(URL u, StringList *local_robots_files)
{
    if (debug)
      cout << endl << "New server: " << u.host() << ", " << u.port() << endl;

    _host = u.host();
    _port = u.port();
    _bad_server = 0;
    _documents = 0;

    // We take it from the configuration
    _persistent_connections = config.Boolean("server", _host.get() ,"persistent_connections");
    _head_before_get = config.Boolean("server", _host.get() ,"head_before_get");

    _max_documents = config.Value("server",_host.get(),"server_max_docs");
    _connection_space = config.Value("server",_host.get(),"server_wait_time");
    _user_agent = config.Find("server", _host, "user_agent");

    // Timeout setting
    _timeout = config.Value("server",_host,"timeout");

    // Number of consecutive attempts to establish a TCP connection
    _tcp_max_retries = config.Value("server",_host,"tcp_max_retries");

    // Seconds to wait after a timeout occurs
    _tcp_wait_time = config.Value("server",_host,"tcp_wait_time");


    if (debug)
    {
      cout << " - Persistent connections: " <<
         (_persistent_connections?"enabled":"disabled") << endl;

      cout << " - HEAD before GET: " <<
         (_head_before_get?"enabled":"disabled") << endl;

      cout << " - Timeout: " << _timeout << endl;
      cout << " - Connection space: " << _connection_space << endl;
      cout << " - Max Documents: " << _max_documents << endl;
      cout << " - TCP retries: " << _tcp_max_retries << endl;
      cout << " - TCP wait time: " << _tcp_wait_time << endl;

    }

    _last_connection.SettoNow();  // For getting robots.txt

    if (strcmp(u.service(),"http") == 0 || strcmp(u.service(),"https") == 0)
      {
	//
	// Attempt to get a robots.txt file from the specified server
	//
	String	url;
	url.trunc();

	if (debug>1)
	  cout << "Trying to retrieve robots.txt file" << endl;        
	url << u.signature() << "robots.txt";
	
	static int	local_urls_only = config.Boolean("local_urls_only");
	time_t 		timeZero = 0; // Right now we want to get this every time
	Document	doc(url, 0);
	Transport::DocStatus	status;
	if (local_robots_files)
	  {  
	    if (debug > 1)
	      cout << "Trying local files" << endl;
	    status = doc.RetrieveLocal(timeZero, local_robots_files);
	    if (status == Transport::Document_not_local)
	      {
		if (local_urls_only)
		  status = Transport::Document_not_found;
		else
		  {
		    if (debug > 1)
		      cout << "Local retrieval failed, trying HTTP" << endl;
		    status = doc.Retrieve(this, timeZero);
		  }
	      }
	  }
	else if (!local_urls_only)
        {
	  status = doc.Retrieve(this, timeZero);

          // Let's check if persistent connections are both
          // allowed by the configuration and possible after
          // having requested the robots.txt file.

          HtHTTP * http;
          if (IsPersistentConnectionAllowed() &&
                  ( http = doc.GetHTTPHandler()))
          {
              if (! http->isPersistentConnectionPossible())
                  _persistent_connections=0;  // not possible. Let's disable
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
	    robotstxt(doc);
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
	  } // end switch
      } // end if (http || https)
}


//*****************************************************************************
// Server::~Server()
//
Server::~Server()
{
}


//*****************************************************************************
// void Server::robotstxt(Document &doc)
//   This will parse the robots.txt file which is contained in the document.
//
void Server::robotstxt(Document &doc)
{
    String	contents = doc.Contents();
    int		length;
    int		pay_attention = 0;
    String	pattern;
    String	myname = config.Find("server", _host.get(), "robotstxt_name");
    int		seen_myname = 0;
    char	*name, *rest;
    
    if (debug > 1)
	cout << "Parsing robots.txt file using myname = " << myname << "\n";

    //
    // Go through the lines in the file and determine if we need to
    // pay attention to them
    //
    for (char *line = strtok(contents, "\r\n"); line; line = strtok(0, "\r\n"))
    {
	if (debug > 2)
	    cout << "Robots.txt line: " << line << endl;

	//
	// Strip comments
	//
	if (strchr(line, '#'))
	{
	    *(strchr(line, '#')) = '\0';
	}
	
	name = good_strtok(line, ':');
	if (!name)
	    continue;
	while (name && isspace(*name))  name++;
	rest = good_strtok(NULL, '\r');
	if (!rest)
	    rest = "";

	while (rest && isspace(*rest))
	    rest++;
			
	length = strlen(rest);
	if (length > 0)
	{
	    while (length > 0 && isspace(rest[length - 1]))
		length--;
	    rest[length] = '\0';
	}

	if (mystrcasecmp(name, "user-agent") == 0)
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
	    else if (mystrncasecmp(rest, (char*)myname, myname.length()) == 0)
	    {
		//
		// This is for us!  This will override any previous patterns
		// that may have been set.
		//
		if (!seen_myname)	// only take first section with our name
		{
		    seen_myname = 1;
		    pay_attention = 1;
		    pattern = 0;	// ignore previous User-agent: *
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
	else if (pay_attention && mystrcasecmp(name, "disallow") == 0)
	{
	    if (debug > 1)
		cout << "Found 'disallow' line: " << rest << endl;
				
	    //
	    // Add this path to our list to ignore
	    //
	    if (*rest)
	    {
		if (pattern.length())
		    pattern << '|' << rest;
		else
		    pattern = rest;
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
		
    _disallow.set(pattern, config.Boolean("case_sensitive"));
}


//*****************************************************************************
// void Server::push(String &path, int hopcount, char *referer, int local)
//
void Server::push(const String &path, int hopcount, const String &referer, int local)
{
    if (_bad_server && !local)
	return;

    if (IsDisallowed(path) != 0)
      {
	if (debug > 2)
	  cout << endl << "   Rejected: forbidden by server robots.txt!";

	return;
      }

    // We use -1 as no limit
    if (_max_documents != -1 &&
	_documents >= _max_documents)
    {
       if (debug>2)     // Hey! we only want to get max_docs
          cout << "Limit of " << _max_documents << " reached for " << _host << endl;
        
       return;
    }

    URLRef	*ref = new URLRef();
    ref->SetURL(path);
    ref->SetHopCount(hopcount);
    ref->SetReferer(referer);
    _paths.Add(ref);

    _documents++;

//     cout << "***** pushing '" << path << "' with '" << referer << "'\n";
}


//*****************************************************************************
// URLRef *Server::pop()
//
URLRef *Server::pop()
{
    URLRef	*ref = (URLRef *) _paths.Remove();

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
void Server::delay()
{
  HtDateTime now;
  int how_long = _connection_space;

  how_long += HtDateTime::GetDiff(_last_connection, now);  

  _last_connection = now;  // Reset the clock for the next delay!

  if (how_long > 0)
    sleep(how_long);

  return;
}


//*****************************************************************************
// void Server::reportStatistics(String &out, char *name)
//
void Server::reportStatistics(String &out, char *name)
{
    out << name << " " << _host << ":" << _port;
    out << " " << _documents << " document";
    if (_documents != 1)
	out << "s";
}
