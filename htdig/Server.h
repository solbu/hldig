//
// Server.h
//
// A class to keep track of server specific information.
//
// $Id: Server.h,v 1.2 1997/03/24 04:33:17 turtle Exp $
//
// $Log: Server.h,v $
// Revision 1.2  1997/03/24 04:33:17  turtle
// Renamed the String.h file to htString.h to help compiling under win32
//
// Revision 1.1.1.1  1997/02/03 17:11:06  turtle
// Initial CVS
//
//
#ifndef _Server_h_
#define _Server_h_

#include <Object.h>
#include <htString.h>
#include <Stack.h>
#include <Queue.h>
#include <StringMatch.h>
#include <time.h>
#include "URLRef.h"

class Document;

class Server : public Object
{
public:
	//
	// Construction/Destruction
	//
					Server(char *host, int port);
					~Server();

	//
	// This needs to be called with a document containing the
	// robots.txt file for this server
	//
	void			robotstxt(Document &doc);

	//
	// Provide some way of getting at the host and port for this server
	//
	int				port()							{return _port;}
	char			*host()							{return _host;}
	
	//
	// Add a path to the queue for this server.  This will check to
	// see if the path in the path is allowed.  If it isn't allowed,
	// it simply won't be added.
	//
	void			push(char *path, int hopcount, char *referer);

	//
	// Return the next URL from the queue for this server.
	//
	URLRef			*pop();

	//
	// Given a time, return the number of seconds have to pass before
	// the next request can be made to this server.  If this number
	// less than or equal to 0, the request can be made immediately.
	//
	int				delay(time_t now);

	//
	// Produce statistics for this server.
	//
	void			reportStatistics(String &out, char *name);
	
private:
	String			_host;
	int				_port;
	int				_bad_server;		// TRUE if we shouldn't use this one
	int				_connection_space;	// Seconds between connections
	time_t			_last_connection;	// Time of last connection to this server
	Queue			_paths;
	StringMatch		_disallow;			// This pattern will be used to test paths
	int				_documents;			// Number of documents visited
};

#endif


