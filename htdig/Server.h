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
// $Id: Server.h,v 1.9.2.7 2000/03/02 17:58:47 angus Exp $
//

#ifndef _Server_h_
#define _Server_h_

#include "Object.h"
#include "htString.h"
#include "StringList.h"
#include "Stack.h"
#include "HtHeap.h"
#include "HtRegex.h"
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
	Server(URL u, StringList *local_robots_files = NULL);
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
        void			AllowPersistentConnection() { _persistent_connections = true; }
        void			AvoidPersistentConnection() { _persistent_connections = false; }
        bool			IsPersistentConnectionAllowed () { return _persistent_connections; }

        // Methods for getting info regarding server configuration
        bool			HeadBeforeGet() { return _head_before_get; }
        unsigned int            TimeOut() { return _timeout; }
        unsigned int            TcpWaitTime() { return _tcp_wait_time; }
        unsigned int            TcpMaxRetries() { return _tcp_max_retries; }
        unsigned int            MaxDocuments() { return _max_documents; }
        
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

        bool                    _persistent_connections; // Are pcs allowed

        bool                    _head_before_get; // HEAD call before a GET?

        int                     _timeout;       // Timeout for this server
                                                
        unsigned int            _tcp_wait_time;     // Wait time after a timeout
                                                // has been raised.
                                                
        unsigned int            _tcp_max_retries;   // Max number of retries when
                                                // connection is not possible
                                                // and timeout occurs


        
};

#endif


