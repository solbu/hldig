//
// Server.h
//
// Server: A class to keep track of server specific information.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: Server.h,v 1.1.2.2 2007/05/01 22:50:44 aarnone Exp $
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
#include "HtDebug.h"

class Document;

class Server : public Object
{
    public:
        //
        // Construction/Destruction
        //
        Server(URL u, StringList *local_robots_files = NULL);
        Server(const Server& rhs);
        ~Server();

        //
        // This needs to be called with a document containing the
        // robots.txt file for this server
        //
        void            robotstxt(Document &doc);

        //
        // Provide some way of getting at the host and port for this server
        //
        int             port() const {return _port;}
        const String    &host() const {return _host;}

        //
        // Provide some way of getting at the status of this server
        //
        int             IsDead()		{return _bad_server;}
        void            IsDead(int flag)	{_bad_server = flag;}

        //
        // Add a path to the queue for this server.
        // This will check to see if the server is up if the URL is not local
        // if it's down, it simply will not be added
        //
        void push(const String &path, int hopcount, facet_list facets, time_t t,
                const String &referer, int local = 0, int newDoc = 1);

        //
        // Return the next URL from the queue for this server.
        //
        URLRef          *pop();

        //
        // Delays the server if necessary. If the time between requests
        // is long enough, the request can occur immediately.
        //
        void            delay();

        //
        // Produce statistics for this server.
        //
        void            reportStatistics(String &out, char *name);

        //
        // Methods for managing persistent connections
        //
        void            AllowPersistentConnection() { _persistent_connections = true; }
        void            AvoidPersistentConnection() { _persistent_connections = false; }
        bool            IsPersistentConnectionAllowed () const { return _persistent_connections; }

        // Methods for getting info regarding server configuration
        bool            HeadBeforeGet() const { return _head_before_get; }
        unsigned int    TimeOut() const { return _timeout; }
        unsigned int    TcpWaitTime() const { return _tcp_wait_time; }
        unsigned int    TcpMaxRetries() const { return _tcp_max_retries; }
        unsigned int    MaxDocuments() const { return _max_documents; }
        const String    &UserAgent() const { return _user_agent; }
        const String    &AcceptLanguage() const { return _accept_language; }
        bool            DisableCookies() const { return _disable_cookies; }

        //
        // Return the URLs to be excluded from this server
        // (for inclusion in the exclude_urls attribute)
        //
        int             IsDisallowed(String url) { return _disallow.match(url, 0, 0); }

    private:
        String          _host;
        int             _port;
        int             _bad_server;        // TRUE if we shouldn't use this one
        int		        _connection_space;  // Seconds between connections
        HtDateTime      _last_connection;   // Time of last connection to this server
        HtHeap          _paths;
        HtRegex         _disallow;          // This pattern will be used to test paths
        int		        _documents;         // Number of documents visited

        int             _max_documents;     // Maximum number of documents from this server

        bool            _persistent_connections; // Are pcs allowed?

        bool            _head_before_get;   // HEAD call before a GET?

        bool            _disable_cookies;   // Should we send cookies?

        int             _timeout;           // Timeout for this server

        unsigned int    _tcp_wait_time;     // Wait time after a timeout has been raised.

        unsigned int    _tcp_max_retries;   // Max # of retries when connection is not possible and timeout occurs

        String          _user_agent;        // User agent to use for this server

        String          _accept_language;   // Accept-language to be sent for the HTTP server

        HtDebug         *debug;             // Debugging output
};

#endif


