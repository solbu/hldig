//
// Server.h
//
// A class to keep track of server specific information.
//
// $Id: Server.h,v 1.3.2.4 2000/02/15 22:42:20 grdetil Exp $
//
//
#ifndef _Server_h_
#define _Server_h_

#include "Object.h"
#include "htString.h"
#include "StringList.h"
#include "Stack.h"
#include "Queue.h"
#include "StringMatch.h"
#include <time.h>
#include "URLRef.h"

class Document;

class Server : public Object
{
public:
	//
	// Construction/Destruction
	//
	Server(char *host, int port, StringList *local_robots_files = NULL);
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
	// Add a path to the queue for this server.  This will check to
	// see if the path in the path is allowed.  If it isn't allowed,
	// it simply won't be added.
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
	
private:
	String			_host;
	int			_port;
	int			_bad_server;		// TRUE if we shouldn't use this one
	int		        _connection_space;	// Seconds between connections
	time_t			_last_connection;	// Time of last connection to this server
	Queue			_paths;
	StringMatch		_disallow;	// This pattern will be used to test paths
	int		        _documents;	// Number of documents visited
	int                     _max_documents;  // Maximum number of documents from this server
};

#endif


