//
// Server.h
//
// Server: A class to keep track of server specific information.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Server.h,v 1.9.2.5 1999/12/02 23:11:42 ghutchis Exp $
//

#ifndef _Server_h_
#define _Server_h_

#include "Object.h"
#include "htString.h"
#include "Stack.h"
#include "HtHeap.h"
#include "StringMatch.h"
#include "URLRef.h"
#include "HtDateTime.h"


class Document;

class Server : public Object
{
public:
	//
	// Construction/Destruction
	//
	Server(URL u, String *local_robots_file = NULL);
	~Server();

	//
	// This needs to be called with a document containing the
	// robots.txt file for this server
	//
	void			robotstxt(Document &doc);

	//
	// Provide some way of getting at the host and port for this server
	//
	int			port()	{return _port;}
	char			*host()	{return _host;}
	
	//
	// Provide some way of getting at the status of this server
	//
	int			IsDead()		{return _bad_server;}
	void			IsDead(int flag)	{_bad_server = flag;}

	//
	// Add a path to the queue for this server.
	// This will check to see if the server is up if the URL is not local
	// if it's down, it simply will not be added
	//
	void			push(char *path, int hopcount, char *referer, int local = 0);

	//
	// Return the next URL from the queue for this server.
	//
	URLRef			*pop();

	//
	// Delays the server if necessary. If the time between requests
	// is long enough, the request can occur immediately.
	//
	void			delay();

	//
	// Produce statistics for this server.
	//
	void			reportStatistics(String &out, char *name);

	//
        // Methods for managing persistent connections
	//
        void			AllowPersistentConnection() { _persistent_connections = 1; }
        void			AvoidPersistentConnection() { _persistent_connections = 0; }
        int			IsPersistentConnectionAllowed () { return _persistent_connections; }

	//
	// Return the URLs to be excluded from this server
	// (for inclusion in the exclude_urls attribute)
	//
	int			IsDisallowed(String url) { return _disallow.match(url, 0, 0); }
	
private:
	String			_host;
	int			_port;
	int			_bad_server;		// TRUE if we shouldn't use this one
	int		        _connection_space;	// Seconds between connections
	HtDateTime		_last_connection;	// Time of last connection to this server
	HtHeap			_paths;
	HtRegex			_disallow;	// This pattern will be used to test paths
	int		        _documents;	// Number of documents visited
	int                     _max_documents;  // Maximum number of documents from this server
        int                     _persistent_connections; // Are pcs allowed

        
};

#endif


