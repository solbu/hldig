//
// Connection.cc
//
// (c) Copyright 1993, San Diego State University -- College of Sciences
//       (See the COPYRIGHT file for more Copyright information)
//
// Implementation of the Connection class
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
#include <sys/time.h>
#include <signal.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#include <htconfig.h>

typedef void (*SIGNAL_HANDLER) (...);

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
    timeout_value = 0;
    retry_value = 1;
    wait_time = 5; // wait 5 seconds after a failed connection
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
    GETPEERNAME_LENGTH_T length = sizeof(server);
    if (getpeername(socket, (struct sockaddr *)&server, &length) < 0)
    {
	perror("getpeername");
    }
    peer = 0;
    server_name = 0;
    all_connections.Add(this);
    timeout_value = 0;
    retry_value = 1;
    wait_time = 5;
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
// int Connection::timeout(int value)
//
int Connection::timeout(int value)
{
    int oval = timeout_value;
    timeout_value = value;
    return oval;
}

//*****************************************************************************
// int Connection::retries(int value)
//
int Connection::retries(int value)
{
    int oval = retry_value;
    retry_value = value;
    return oval;
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
// int Connection::assign_server(unsigned int addr)
//
int Connection::assign_server(unsigned int addr)
{
    server.sin_addr.s_addr = addr;
    return OK;
}

extern "C" unsigned int   inet_addr(char *);

//*****************************************************************************
// int Connection::assign_server(char *name)
//
int Connection::assign_server(char *name)
{
    struct hostent		*hp;
    unsigned int		addr;

    addr = inet_addr(name);
    if (addr == ~0 || addr == ~0/*L*/)
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

//
// Do nothing, we are only interested in the EINTR return of the
// running system call.
//
static void handler_timeout(int) {
}

//*****************************************************************************
// int Connection::connect(int allow_EINTR)
//
int Connection::connect(int allow_EINTR)
{
    int	status;
    int retries = retry_value;

     while (retries--)
      {
        //
        // Set an alarm to make sure the connect() call times out
        // appropriately This ensures the call won't hang on a
        // dead server or bad DNS call.
        // Save the previous alarm signal handling policy, if any.
        //
        struct sigaction action;
        struct sigaction old_action;
        memset((char*)&action, '\0', sizeof(struct sigaction));
        memset((char*)&old_action, '\0', sizeof(struct sigaction));
        action.sa_handler = handler_timeout;
        sigaction(SIGALRM, &action, &old_action);
        alarm(timeout_value);

	status = ::connect(sock, (struct sockaddr *)&server, sizeof(server));
	//
        // Disable alarm and restore previous policy if any
        //
        alarm(0);
        sigaction(SIGALRM, &old_action, 0);

	if (status == 0 || errno == EALREADY || errno == EISCONN)
	  {
	    connected = 1;
	    return OK;
	  }

        //
        // Only loop if timed out. Other errors are fatal.
        //
	#ifndef EAGAIN
        if (status < 0 && (errno != EINTR || !allow_EINTR))
          break;
	#else
	if (status < 0 && (errno != EINTR || !allow_EINTR) && errno != EAGAIN)
          break;
	#endif

	::close(sock);
	open();
	sleep(wait_time);
      }
	
#if 0
    if (status == ECONNREFUSED)
    {
	//
	// For the case where the connection attempt is refused, we need
	// to close the socket and create a new one in order to do any
	// more with it.
	//
	::close(sock);
	open();
    }
#else
    ::close(sock);
    open(0);
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
    GETPEERNAME_LENGTH_T length = sizeof(server);
    
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
	newsock = ::accept(sock, (struct sockaddr *)0, (GETPEERNAME_LENGTH_T *)0);
	if (newsock == NOTOK && errno == EINTR)
	    continue;
	break;
    }
    if (newsock == NOTOK)
	return (Connection *)0;

    Connection	*newconnect = new Connection;
    newconnect->sock = newsock;

    GETPEERNAME_LENGTH_T length = sizeof(newconnect->server);
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

    need_io_stop = 0;
    do
    {
      errno = 0;

      if (timeout_value > 0) {
          fd_set fds;
          FD_ZERO(&fds);
          FD_SET(sock, &fds);

          timeval tv;
          tv.tv_sec = timeout_value;
          tv.tv_usec = 0;

          int selected = ::select(sock+1, &fds, 0, 0, &tv);
          if (selected <= 0)
              need_io_stop++;
      }

      if (!need_io_stop)
          count = ::read(sock, buffer, maxlength);
      else
          count = -1;         // Input timed out
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
	GETPEERNAME_LENGTH_T	length = sizeof(p);
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
    GETPEERNAME_LENGTH_T	length = sizeof(peer);
    
    if (getpeername(sock, (struct sockaddr *) &peer, &length) < 0)
    {
	return 0;
    }
    return inet_ntoa(peer.sin_addr);
}

#ifdef NEED_PROTO_GETHOSTNAME
extern "C" int gethostname(char *name, int namelen);
#endif

//*************************************************************************
// unsigned int gethostip(char *ip, int length)
//
unsigned int gethostip(char *ip, int length)
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
