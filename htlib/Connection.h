//
// Connection.h
//
// (c) Copyright 1993, San Diego State University -- College of Sciences
//       (See the COPYRIGHT file for more Copyright information)
//
// This class forms a easy to use interface to the berkeley tcp socket library.
// All the calls are basically the same, but the parameters do not have any
// stray _addr or _in mixed in...
//
// $Id: Connection.h,v 1.3.2.1 2001/11/04 23:23:41 ghutchis Exp $
//
// $Log: Connection.h,v $
// Revision 1.3.2.1  2001/11/04 23:23:41  ghutchis
// 	* htlib/Connection.h, htlib/Connection.cc: Backport Connection
// 	class from 3.2 code--installs alarm() call to timeout connections
// 	and will retry connections a few times before giving up.
//
// Needs to be used in htdig/* code, but should work w/o problem.
// (just no way of configuring # retries, etc.)
//
// Revision 1.3  1998/10/18 21:22:16  ghutchis
//
// Revised connection timeout methods.
//
// Revision 1.2  1998/10/17 14:29:18  ghutchis
//
// Included fixes sent by Paul J. Meyer <pmeyer@rimeice.msfc.nasa.gov> to fix
// connections on Dec Alpha environments.
//
// Revision 1.1.1.1  1997/02/03 17:11:04  turtle
// Initial CVS
//
//

#if !defined(_Connection_h_)
# define	_Connection_h_

#include "io.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

class String;

class Connection : public io
{
public:
    // Constructors & Destructors
    Connection();
    Connection(int socket);
    ~Connection();

    // (De)initialization
    int				open(int priv = 0);
    int				close();
    int				ndelay();
    int				nondelay();
    int                         timeout(int value);
    int                         retries(int value);

    // Port stuff
    int				assign_port(int port = 0);
    int				assign_port(char *service);
    int				get_port();
    int				is_privileged();

    // Host stuff
    int				assign_server(char *name);
    int				assign_server(unsigned int addr = INADDR_ANY);
    char				*get_server()		{return server_name;}

    // Connection establishment
    int				connect(int allow_EINTR = 0);
    Connection			*accept(int priv = 0);
    Connection			*accept_privileged();

    // Registration things
    int				bind();
    int				listen(int n = 5);

    // IO
    int				read_partial(char *buffer, int maxlength);
    int				write_partial(char *buffer, int maxlength);
    void				stop_io()		{need_io_stop = 1;}

    // Access to socket number
    char				*socket_as_string();
    int				get_socket()		{return sock;}
    int				isopen()		{return sock >= 0;}
    int				isconnected()		{return connected;}

    // Access to info about remote socket
    char				*get_peerip();
    char				*get_peername();

private:
    int				sock;
    struct sockaddr_in		server;
    int				connected;
    char				*peer;
    char				*server_name;
    int				need_io_stop;
    int                         timeout_value;
    int				retry_value;
    int				wait_time;
};


//*************************************************************************
// inline int Connection::is_privileged()
// PURPOSE:
//   Return whether the port is priveleged or not.
//
inline int Connection::is_privileged()
{
    return server.sin_port < 1023;
}


//
// Get arround the lack of gethostip() library call...  There is a gethostname()
// call but we want the IP address, not the name!
// The call will put the ASCII string representing the IP address in the supplied
// buffer and it will also return the 4 byte unsigned long equivalent of it.
// The ip buffer can be null...
//
unsigned int gethostip(char *ip = 0, int length = 0);

#endif
