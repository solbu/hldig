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
// $Id: Server.cc,v 1.16 1999/10/08 12:05:20 loic Exp $
//

#include "htdig.h"
#include "Server.h"
#include "good_strtok.h"
#include "htString.h"
#include "URL.h"
#include "htdig.h"
#include "Document.h"
#include "URLRef.h"

#include <ctype.h>


//*****************************************************************************
// Server::Server(char *host, int port)
//
Server::Server(char *host, int port)
{
    if (debug > 0)
	cout << endl << "New server: " << host << ", " << port << endl;

    _host = host;
    _port = port;
    _bad_server = 0;
    _documents = 0;
    if (!config.Boolean("case_sensitive"))
      _disallow.IgnoreCase();
    _max_documents = config.Value("server_max_docs", -1);
    _connection_space = config.Value("server_wait_time", 0);
    _last_connection = time(0);  // For getting robots.txt

    //
    // Attempt to get a robots.txt file from the specified server
    //
    String	url = "http://";
    url << host << ':' << port << "/robots.txt";
    Document	doc(url, 0);
    if (debug)
      cout << "     (" << url << ")\n";
    switch (doc.RetrieveHTTP(0))
    {
	case Document::Document_ok:
	    //
	    // Found a robots.txt file.  Go parse it.
	    //
	    robotstxt(doc);
	    break;
			
	case Document::Document_not_found:
	case Document::Document_not_html:
	case Document::Document_redirect:
	case Document::Document_not_authorized:
	    //
	    // These cases are for when there is no robots.txt file.
	    // We will just go on happily without restrictions
	    //
	    break;
			
	case Document::Document_no_server:
	case Document::Document_no_host:
	default:
	    //
	    // In all other cases the server could not be reached.
	    // We will remember this fact so that no more attempts to
	    // contact this server will be made.
	    //
	    _bad_server = 1;
	    break;
    }
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
    String	myname = config["robotstxt_name"];
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
		seen_myname = 1;
		pay_attention = 1;
		pattern = 0;
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
		
    _disallow.Pattern(pattern);
}


//*****************************************************************************
// void Server::push(char *path, int hopcount, char *referer)
//
void Server::push(char *path, int hopcount, char *referer)
{
    if (_bad_server)
	return;

    //
    // Make sure that the path is allowed on this server
    //
    int	which, length;
    char	*serverPath = strchr(path + 7, '/');
    if (!serverPath)
	serverPath = path;
    if (_disallow.Compare(serverPath, which, length))
    {
	if (debug > 1)
	    cout << "robots.txt: discarding '" << path <<
		"', which = " << which << ", length = " << length << endl;
	return;
    }

    // We use -1 as no limit
    if (_max_documents != -1 &&
	_documents >= _max_documents)     // Hey! we only want to get max_docs
      return;
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
  time_t now = time(0);
  time_t how_long = _connection_space + _last_connection - now;
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
