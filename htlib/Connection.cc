//
// Connection.cc
//
// (c) Copyright 1993, San Diego State University -- College of Sciences
//       (See the COPYRIGHT file for more Copyright information)
//
// Implementation of the Connection class
//
// $Log: Connection.cc,v $
// Revision 1.6  1998/05/26 03:58:06  turtle
// Got rid of compiler warnings.
//
// Revision 1.5  1998/01/05 05:18:18  turtle
// Fix by Pontus Borg for AIX.  Changed 'size_t' to 'unsigned long' for
// the length parameter for getpeername()
//
// Revision 1.4  1997/10/23 18:01:10  turtle
// Fix by Pontus Borg for AIX.  Changed 'size_t' to 'unsigned long' for
// the length parameter for getpeername()
//
// Revision 1.3  1997/03/24 04:33:19  turtle
// Renamed the String.h file to htString.h to help compiling under win32
//
// Revision 1.2  1997/02/10 17:32:47  turtle
// Applied AIX specific patches supplied by Lars-Owe Ivarsson
// <lars-owe.ivarsson@its.uu.se>
//
// Revision 1.1.1.1  1997/02/03 17:11:04  turtle
// Initial CVS
//
//

#include "Connection.h"
#include "Object.h"
#include "htString.h"
#include "List.h"

#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdlib.h>

extern "C" {
    int rresvport(int *);
}

List	all_connections;

Connection::Connection()
{
    sock = -1;
    connected = 0;
    peer = 0;
    server_name = 0;
    all_connections.Add(this);
}


//*************************************************************************
// Connection::Connection(int socket)
// PURPOSE:
//   Create a connection from just a socket.
// PARAMETERS:
//   int socket:  obvious!!!!
//
Connection::Connection(int socket)
{
    sock = socket;
    connected = 0;
    size_t length = sizeof(server);
    if (getpeername(socket, (struct sockaddr *)&server, &length) < 0)
    {
	perror("getpeername");
    }
    peer = 0;
    server_name = 0;
    all_connections.Add(this);
}


//*****************************************************************************
// Connection::~Connection()
//
Connection::~Connection()
{
    all_connections.Remove(this);
    this->close();
    delete peer;
    delete server_name;
}


//*****************************************************************************
// int Connection::open(int priv)
//
int Connection::open(int priv)
{
    if (priv)
    {
	int	aport = IPPORT_RESERVED - 1;

	sock = rresvport(&aport);
    }
    else
	sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == NOTOK)
	return NOTOK;

    int	on = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on));
    server.sin_family = AF_INET;

    return OK;
}


//*****************************************************************************
// int Connection::ndelay()
//
int Connection::ndelay()
{
    return fcntl(sock, F_SETFL, FNDELAY);
}


//*****************************************************************************
// int Connection::nondelay()
//
int Connection::nondelay()
{
    return fcntl(sock, F_SETFL, 0);
}


//*****************************************************************************
// int Connection::close()
//
int Connection::close()
{
    connected = 0;
    if (sock >= 0)
    {
	int ret = ::close(sock);
	sock = -1;
	return ret;
    }
    return NOTOK;
}


//*****************************************************************************
// int Connection::assign_port(int port)
//
int Connection::assign_port(int port)
{
    server.sin_port = htons(port);
    return OK;
}


//*****************************************************************************
// int Connection::assign_port(char *service)
//
int Connection::assign_port(char *service)
{
    struct servent		*sp;

    sp = getservbyname(service, "tcp");
    if (sp == NULL)
    {
	return NOTOK;
    }
    server.sin_port = sp->s_port;
    return OK;
}

//*****************************************************************************
// int Connection::assign_server(unsigned long addr)
//
int Connection::assign_server(unsigned long addr)
{
    server.sin_addr.s_addr = addr;
    return OK;
}

extern "C" unsigned long   inet_addr(char *);

//*****************************************************************************
// int Connection::assign_server(char *name)
//
int Connection::assign_server(char *name)
{
    struct hostent		*hp;
    unsigned long		addr;

    addr = inet_addr(name);
    if (addr == ~0L)
    {
	hp = gethostbyname(name);
	if (hp == NULL)
	{
	    return NOTOK;
	}
	memcpy((char *)&server.sin_addr, (char *)hp->h_addr, hp->h_length);
    }
    else
    {
	memcpy((char *)&server.sin_addr, (char *)&addr, sizeof(addr));
    }

    delete server_name;
    server_name = strdup(name);

    return OK;
}


//*****************************************************************************
// int Connection::connect(int allow_EINTR)
//
int Connection::connect(int allow_EINTR)
{
    int	status;

    for (;;)
    {
	status = ::connect(sock, (struct sockaddr *)&server, sizeof(server));
	if (status < 0 && errno == EINTR && !allow_EINTR)
	{
	    ::close(sock);
	    open();
	    continue;
	}
	break;
    }
	
    if (status == 0 || errno == EALREADY || errno == EISCONN)
    {
	connected = 1;
	return OK;
    }
#if 0
    if (status == ECONNREFUSED)
    {
	//
	// For the case where the connection attempt is refused, we
	// need to close the socket and create a new one in order to
	// do any more with it.
	//
	::close(sock);
	open();
    }
#else
    ::close(sock);
    open();
#endif

    connected = 0;
    return NOTOK;
}


//*****************************************************************************
// int Connection::bind()
//
int Connection::bind()
{
    if (::bind(sock, (struct sockaddr *)&server, sizeof(server)) == NOTOK)
    {
	return NOTOK;
    }
    return OK;
}


//*****************************************************************************
// int Connection::get_port()
//
int Connection::get_port()
{
    size_t length = sizeof(server);
    
    if (getsockname(sock, (struct sockaddr *)&server, &length) == NOTOK)
    {
	return NOTOK;
    }
    return ntohs(server.sin_port);
}


//*****************************************************************************
// int Connection::listen(int n)
//
int Connection::listen(int n)
{
    return ::listen(sock, n);
}


//*****************************************************************************
// Connection *Connection::accept(int priv)
//
Connection *Connection::accept(int priv)
{
    int	newsock;

    while (1)
    {
	newsock = ::accept(sock, (struct sockaddr *)0, (size_t *)0);
	if (newsock == NOTOK && errno == EINTR)
	    continue;
	break;
    }
    if (newsock == NOTOK)
	return (Connection *)0;

    Connection	*newconnect = new Connection;
    newconnect->sock = newsock;

    size_t length = sizeof(newconnect->server);
    getpeername(newsock, (struct sockaddr *)&newconnect->server, &length);

    if (priv && newconnect->server.sin_port >= IPPORT_RESERVED)
    {
	delete newconnect;
	return (Connection *)0;
    }

    return newconnect;
}


//*************************************************************************
// Connection *Connection::accept_privileged()
// PURPOSE:
//   Accept  in  incoming  connection  but  only  if  it  is  from a
//   privileged port
//
Connection * Connection::accept_privileged()
{
    return accept(1);
}


//*************************************************************************
// int Connection::read_partial(char *buffer, int maxlength)
// PURPOSE:
//   Read  at  most  <maxlength>  from  the  current TCP connection.
//   This  is  equivalent  to  the  workings  of the standard read()
//   system call
// PARAMETERS:
//   char *buffer:	Buffer to read the data into
//   int maxlength:	Maximum number of bytes to read into the buffer
// RETURN VALUE:
//   The actual number of bytes read in.
// ASSUMPTIONS:
//   The connection has been previously established.
// FUNCTIONS USED:
//   read()
//
int Connection::read_partial(char *buffer, int maxlength)
{
    int		count;

    do
    {
	count = ::read(sock, buffer, maxlength);
    }
    while (count < 0 && errno == EINTR && !need_io_stop);
    need_io_stop = 0;

    return count;
}


//*************************************************************************
// int Connection::write_partial(char *buffer, int maxlength)
//
int Connection::write_partial(char *buffer, int maxlength)
{
    int		count;

    do
    {
	count = ::write(sock, buffer, maxlength);
    }
    while (count < 0 && errno == EINTR && !need_io_stop);
    need_io_stop = 0;

    return count;
}


//*************************************************************************
// char * Connection::socket_as_string()
// PURPOSE:
//   Return  the  numeric  ASCII  equivalent  of  the socket number.
//   This is needed to pass the socket to another program
//
char * Connection::socket_as_string()
{
    char	*buffer = new char[20];

    sprintf(buffer, "%d", sock);
    return buffer;
}


extern "C" char *inet_ntoa(struct in_addr);

//*************************************************************************
// char *Connection::get_peername()
//
char *Connection::get_peername()
{
    if (!peer)
    {
	struct sockaddr_in	p;
	size_t			length = sizeof(p);
	struct hostent		*hp;
	
	if (getpeername(sock, (struct sockaddr *) &p, &length) < 0)
	{
	    return 0;
	}
	
	length = sizeof(p.sin_addr);
	hp = gethostbyaddr((const char *) &p.sin_addr, length, AF_INET);
	if (hp)
	    peer = strdup((char *) hp->h_name);
	else
	    peer = strdup((char *) inet_ntoa(p.sin_addr));
    }
    return peer;
}


//*************************************************************************
// char *Connection::get_peerip()
//
char *Connection::get_peerip()
{
    struct sockaddr_in	peer;
    size_t		length = sizeof(peer);
    
    if (getpeername(sock, (struct sockaddr *) &peer, &length) < 0)
    {
	return 0;
    }
    return inet_ntoa(peer.sin_addr);
}

#if defined(__sun__)
extern "C" int gethostname(char *name, int namelen);
#endif

//*************************************************************************
// unsigned long gethostip(char *ip, int length)
//
unsigned long gethostip(char *ip, int length)
{
    char	hostname[100];
    if (gethostname(hostname, sizeof(hostname)) == NOTOK)
	return 0;

    struct hostent	*ent = gethostbyname(hostname);
    if (!ent)
	return 0;

    struct in_addr	addr;
    memcpy((char *) &addr.s_addr, ent->h_addr, sizeof(addr));
    if (ip)
	strncpy(ip, inet_ntoa(addr), length);
    return addr.s_addr;
}

