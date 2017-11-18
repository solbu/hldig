//
// Connection.cc
//
// Connection: This class forms a easy to use interface to the berkeley
//             tcp socket library. All the calls are basically the same, 
//             but the parameters do not have any stray _addr or _in
//             mixed in...
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: Connection.cc,v 1.10 2004/05/28 13:15:22 lha Exp $
//
#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "Connection.h"
#include "Object.h"
#include "List.h"

#include <errno.h>
#include <stdio.h>
#include <sys/types.h>

#ifdef _MSC_VER /* _WIN32 */
#include <windows.h>
#include <winsock.h>
#define EALREADY     WSAEALREADY
#define EISCONN      WSAEISCONN
#else
#include <sys/socket.h>
#include <arpa/inet.h>  // For inet_ntoa
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#endif

#ifndef _MSC_VER /* _WIN32 */
#include <sys/file.h>
#include <sys/time.h>
#else
#include <io.h>
#endif

#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>

#ifndef _MSC_VER /* _WIN32 */
#include <unistd.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

typedef void (*SIGNAL_HANDLER) (...);

#ifndef _MSC_VER /* _WIN32 */
extern "C" {
    int rresvport(int *);
}
#endif

#undef MIN
#define  MIN(a,b)    ((a)<(b)?(a):(b))

List  all_connections;

//*************************************************************************
// Connection::Connection(int socket)
// - Default constructor
// PURPOSE:
//   Create a connection from just a socket.
// PARAMETERS:
//   int socket:  obvious!!!!
//
//*************************************************************************
Connection::Connection(int socket)
: pos(0), pos_max(0),
   sock(socket), connected(0), peer(""), server_name(""), server_ip_address(""),
   need_io_stop(0), timeout_value(0), retry_value(1),
   wait_time(5) // wait 5 seconds after a failed connection attempt
{
   Win32Socket_Init();

   if (socket > 0)
   {
      GETPEERNAME_LENGTH_T length = sizeof(server);
      if (getpeername(socket, (struct sockaddr *)&server, &length) < 0)
         perror("getpeername");
   }
   
   all_connections.Add(this);
}

// Copy constructor
Connection::Connection(const Connection& rhs)
: pos(rhs.pos), pos_max(rhs.pos_max),
   sock(rhs.sock), connected(rhs.connected),
   peer(rhs.peer), server_name(rhs.server_name), server_ip_address(rhs.server_ip_address),
   need_io_stop(rhs.need_io_stop), timeout_value(rhs.timeout_value),
   retry_value(rhs.retry_value),
   wait_time(rhs.wait_time) // wait 5 seconds after a failed connection attempt
{
    all_connections.Add(this);
}


//*****************************************************************************
// Connection::~Connection()
//
Connection::~Connection()
{
    all_connections.Remove(this);
    this->Close();
}


//*****************************************************************************
// int Connection::Win32Socket_init(void)
//
// This function is only used when Code is compiled as a Native Windows
// application
// 
// The native Windows socket system needs to be initialized.
//
int Connection::Win32Socket_Init(void)
{
#ifdef _MSC_VER /* _WIN32 */
    WORD    wVersionRequested;
    WSADATA wsaData;

    wVersionRequested = MAKEWORD(2, 2);

    if (WSAStartup(wVersionRequested, &wsaData))
        return(-1);

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2 ) {
        WSACleanup();
        return(-1);
    }
#endif

    return(0);
}
//*****************************************************************************
// int Connection::Open(int priv)
//
int Connection::Open(int priv)
{
    if (priv)
    {
  int  aport = IPPORT_RESERVED - 1;

//  Native Windows (MSVC) has no rresvport
#ifndef _MSC_VER /* _WIN32 */
  sock = rresvport(&aport);
#else
  return NOTOK;
#endif
    }
    else
    {
      sock = socket(AF_INET, SOCK_STREAM, 0);
  //cout << "socket()  sock=" << sock << endl;
    }

    if (sock == NOTOK)
  return NOTOK;

    int  on = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on));
    server.sin_family = AF_INET;

    return OK;
}


//*****************************************************************************
// int Connection::Ndelay()
//
int Connection::Ndelay()
{
#ifndef _MSC_VER /* _WIN32 */
    return fcntl(sock, F_SETFL, FNDELAY);
#else
    // Note:  This function is never called
    // TODO: Look into ioctsocket(..) of Win32 Socket API
    return(0);
#endif
}


//*****************************************************************************
// int Connection::Nondelay()
//
int Connection::Nondelay()
{
#ifndef _MSC_VER /* _WIN32 */
    return fcntl(sock, F_SETFL, 0);
#else
    // Note:  This function is never called
    // TODO: Look into ioctsocket(..) of Win32 Socket API
    return(0);
#endif
}

//*****************************************************************************
// int Connection::Timeout(int value)
//
int Connection::Timeout(int value)
{
    int oval = timeout_value;
    timeout_value = value;
    return oval;
}

//*****************************************************************************
// int Connection::retries(int value)
//
int Connection::Retries(int value)
{
    int oval = retry_value;
    retry_value = value;
    return oval;
}

//*****************************************************************************
// int Connection::Close()
//
int Connection::Close()
{
    connected = 0;
    if (sock >= 0)
    {
  int ret = close(sock);
  sock = -1;
  return ret;
    }
    return NOTOK;
}


//*****************************************************************************
// int Connection::Assign_Port(int port)
//
int Connection::Assign_Port(int port)
{
    server.sin_port = htons(port);
    return OK;
}


//*****************************************************************************
// int Connection::Assign_Port(char *service)
//
int Connection::Assign_Port(const String &service)
{
    struct servent    *sp;

    sp = getservbyname(service, "tcp");
    if (sp == 0)
    {
  return NOTOK;
    }
    server.sin_port = sp->s_port;
    return OK;
}

//*****************************************************************************
// int Connection::Assign_Server(unsigned int addr)
//
int Connection::Assign_Server(unsigned int addr)
{
    server.sin_addr.s_addr = addr;
    return OK;
}

#ifndef _MSC_VER /* _WIN32 */
//extern "C" unsigned int   inet_addr(char *);
#endif

//*****************************************************************************
//
int Connection::Assign_Server(const String& name)
{
    struct hostent *hp;
    char **alias_list;
    unsigned int addr;

    //
    // inet_addr arg IS const char even though prototype says otherwise
    //
    addr = inet_addr((char*)name.get());
    if (addr == (unsigned int)~0)
    {
        // Gets the host given a string
        hp = gethostbyname(name);

        if (hp == 0)
           return NOTOK;

        alias_list = hp->h_aliases;
        memcpy((char *)&server.sin_addr, (char *)hp->h_addr, hp->h_length);
    }
    else
    {
        memcpy((char *)&server.sin_addr, (char *)&addr, sizeof(addr));
    }

    server_name = name.get();
    server_ip_address = inet_ntoa(server.sin_addr);

    return OK;
}

//
// Do nothing, we are only interested in the EINTR return of the
// running system call.
//
static void handler_timeout(int) {
}

//*****************************************************************************
// int Connection::Connect()
//
int Connection::Connect()
{
    int  status;
    int retries = retry_value;

    while (retries--)
      {
#ifndef _MSC_VER /* _WIN32 */
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
#endif

  status = connect(sock, (struct sockaddr *)&server, sizeof(server));

  //
  // Disable alarm and restore previous policy if any
  //
#ifndef _MSC_VER /* _WIN32 */
  alarm(0);
         sigaction(SIGALRM, &old_action, 0);
#endif

  if (status == 0 || errno == EALREADY || errno == EISCONN)
    {
      connected = 1;
      return OK;
    }

  //
  // Only loop if timed out. Other errors are fatal.
  //
  if (status < 0 && errno != EINTR)
    break;
  
  // cout << " <"  << ::strerror(errno) << "> ";
  close(sock);
        Open();

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
  Close(sock);
  Open();
    }
#else
    close(sock);
    Open(0);
#endif

    connected = 0;
    return NOTOK;
}


//*****************************************************************************
// int Connection::Bind()
//
int Connection::Bind()
{
    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) == NOTOK)
    {
  return NOTOK;
    }
    return OK;
}


//*****************************************************************************
// int Connection::Get_Port()
//
int Connection::Get_Port()
{
    GETPEERNAME_LENGTH_T length = sizeof(server);
    
    if (getsockname(sock, (struct sockaddr *)&server, &length) == NOTOK)
    {
  return NOTOK;
    }
    return ntohs(server.sin_port);
}


//*****************************************************************************
// int Connection::Listen(int n)
//
int Connection::Listen(int n)
{
    return listen(sock, n);
}


//*****************************************************************************
// Connection *Connection::Accept(int priv)
//
Connection *Connection::Accept(int priv)
{
    int  newsock;

    while (1)
    {
  newsock = accept(sock, (struct sockaddr *)0, (GETPEERNAME_LENGTH_T *)0);
  if (newsock == NOTOK && errno == EINTR)
      continue;
  break;
    }
    if (newsock == NOTOK)
  return (Connection *)0;

    Connection  *newconnect = new Connection;
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
// Connection *Connection::Accept_Privileged()
// PURPOSE:
//   Accept  in  incoming  connection  but  only  if  it  is  from a
//   privileged port
//
Connection * Connection::Accept_Privileged()
{
    return Accept(1);
}

//*****************************************************************************
// int Connection::read_char()
//
int Connection::Read_Char()
{
    if (pos >= pos_max)
    {
  pos_max = Read_Partial(buffer, sizeof(buffer));
  pos = 0;
  if (pos_max <= 0)
  {
      return -1;
  }
    }
    return buffer[pos++] & 0xff;
}


//*****************************************************************************
// String *Connection::Read_Line(String &s, char *terminator)
//
String *Connection::Read_Line(String &s, const char *terminator)
{
    int    termseq = 0;
    s = 0;

    for (;;)
    {
  int  ch = Read_Char();
  if (ch < 0)
  {
      //
      // End of file reached.  If we still have stuff in the input buffer
      // we need to return it first.  When we get called again we will
      // return 0 to let the caller know about the EOF condition.
      //
      if (s.length())
    break;
      else
    return (String *) 0;
  }
  else if (terminator[termseq] && ch == terminator[termseq])
  {
      //
      // Got one of the terminator characters.  We will not put
      // it in the string but keep track of the fact that we
      // have seen it.
      //
      termseq++;
      if (!terminator[termseq])
    break;
  }
  else
  {
      s << (char) ch;
  }
    }

    return &s;
}


//*****************************************************************************
// String *Connection::read_line(char *terminator)
//
String *Connection::Read_Line(char *terminator)
{
    String  *s;

    s = new String;
    return Read_Line(*s, terminator);
}


//*****************************************************************************
// char *Connection::read_line(char *buffer, int maxlength, char *terminator)
//
char *Connection::Read_Line(char *buffer, int maxlength, char *terminator)
{
    char  *start = buffer;
    int    termseq = 0;

    while (maxlength > 0)
    {
  int    ch = Read_Char();
  if (ch < 0)
  {
      //
      // End of file reached.  If we still have stuff in the input buffer
      // we need to return it first.  When we get called again, we will
      // return 0 to let the caller know about the EOF condition.
      //
      if (buffer > start)
    break;
      else
    return (char *) 0;
  }
  else if (terminator[termseq] && ch == terminator[termseq])
  {
      //
      // Got one of the terminator characters.  We will not put
      // it in the string but keep track of the fact that we
      // have seen it.
      //
      termseq++;
      if (!terminator[termseq])
    break;
  }
  else
  {
      *buffer++ = ch;
      maxlength--;
  }
    }
    *buffer = '\0';

    return start;
}


//*****************************************************************************
// int Connection::write_line(char *str, char *eol)
//
int Connection::Write_Line(char *str, char *eol)
{
    int    n, nn;

    if ((n = Write(str)) < 0)
  return -1;

    if ((nn = Write(eol)) < 0)
  return -1;

    return n + nn;
}


//*****************************************************************************
// int Connection::Write(char *buffer, int length)
//
int Connection::Write(char *buffer, int length)
{
    int    nleft, nwritten;

    if (length == -1)
  length = strlen(buffer);

    nleft = length;
    while (nleft > 0)
    {
  nwritten = Write_Partial(buffer, nleft);
  if (nwritten < 0 && errno == EINTR)
      continue;
  if (nwritten <= 0)
      return nwritten;
  nleft -= nwritten;
  buffer += nwritten;
    }
    return length - nleft;
}


//*****************************************************************************
// int Connection::Read(char *buffer, int length)
//
int Connection::Read(char *buffer, int length)
{
    int    nleft, nread;

    nleft = length;

    //
    // If there is data in our internal input buffer, use that first.
    //
    if (pos < pos_max)
    {
  int n = MIN(length, pos_max - pos);

  memcpy(buffer, &this->buffer[pos], n);
  pos += n;
  buffer += n;
  nleft -= n;
    }

    while (nleft > 0)
    {
  nread = Read_Partial(buffer, nleft);
  if (nread < 0 && errno == EINTR)
      continue;
  if (nread < 0)
      return -1;
  else if (nread == 0)
      break;

  nleft -= nread;
  buffer += nread;
    }
    return length - nleft;
}


void Connection::Flush()
{
   pos = pos_max = 0;
}

//*************************************************************************
// int Connection::Read_Partial(char *buffer, int maxlength)
// PURPOSE:
//   Read  at  most  <maxlength>  from  the  current TCP connection.
//   This  is  equivalent  to  the  workings  of the standard read()
//   system call
// PARAMETERS:
//   char *buffer:  Buffer to read the data into
//   int maxlength:  Maximum number of bytes to read into the buffer
// RETURN VALUE:
//   The actual number of bytes read in.
// ASSUMPTIONS:
//   The connection has been previously established.
// FUNCTIONS USED:
//   read()
//
int Connection::Read_Partial(char *buffer, int maxlength)
{
    int    count;

    need_io_stop = 0;
    do
    {
      errno = 0;

      if (timeout_value > 0) {
          FD_SET_T fds;
          FD_ZERO(&fds);
          FD_SET(sock, &fds);

          timeval tv;
          tv.tv_sec = timeout_value;
          tv.tv_usec = 0;

          int selected = select(sock+1, &fds, 0, 0, &tv);
          if (selected <= 0)
              need_io_stop++;
      }

      if (!need_io_stop)
          count = recv(sock, buffer, maxlength, 0);
      else
          count = -1;         // Input timed out
    }
    while (count <= 0 && errno == EINTR && !need_io_stop);
    need_io_stop = 0;

    return count;
}


//*************************************************************************
// int Connection::Write_Partial(char *buffer, int maxlength)
//
int Connection::Write_Partial(char *buffer, int maxlength)
{
    int    count;

    do
    {

       count = send(sock, buffer, maxlength, 0);
  
    }
    while (count < 0 && errno == EINTR && !need_io_stop);
    need_io_stop = 0;

    return count;
}


//*************************************************************************
// char * Connection::Socket_as_String()
// PURPOSE:
//   Return  the  numeric  ASCII  equivalent  of  the socket number.
//   This is needed to pass the socket to another program
//
char * Connection::Socket_as_String()
{
    char  *buffer = new char[20];

    sprintf(buffer, "%d", sock);
    return buffer;
}

#ifndef _MSC_VER /* _WIN32 */
extern "C" char *inet_ntoa(struct in_addr);
#endif

//*************************************************************************
// char *Connection::Get_Peername()
//
const char* Connection::Get_Peername()
{
    if (peer.empty())
    {
  struct sockaddr_in  p;
  GETPEERNAME_LENGTH_T  length = sizeof(p);
  struct hostent    *hp;
  
  if (getpeername(sock, (struct sockaddr *) &p, &length) < 0)
  {
      return 0;
  }
  
  length = sizeof(p.sin_addr);
  hp = gethostbyaddr((const char *) &p.sin_addr, length, AF_INET);
  if (hp)
      peer = (char *) hp->h_name;
  else
      peer = (char *) inet_ntoa(p.sin_addr);
    }
    return (const char*) peer.get();
}


//*************************************************************************
// char *Connection::Get_PeerIP()
//
const char* Connection::Get_PeerIP() const
{
    struct sockaddr_in  p;
    GETPEERNAME_LENGTH_T  length = sizeof(p);
    
    if (getpeername(sock, (struct sockaddr *) &p, &length) < 0)
    {
  return 0;
    }
    return (const char*) inet_ntoa(p.sin_addr);
}

#ifdef NEED_PROTO_GETHOSTNAME
extern "C" int gethostname(char *name, int namelen);
#endif

//*************************************************************************
// unsigned int GetHostIP(char *ip, int length)
//
unsigned int GetHostIP(char *ip, int length)
{
    char  hostname[100];
    if (gethostname(hostname, sizeof(hostname)) == NOTOK)
  return 0;

    struct hostent  *ent = gethostbyname(hostname);
    if (!ent)
  return 0;

    struct in_addr  addr;
    memcpy((char *) &addr.s_addr, ent->h_addr, sizeof(addr));
    if (ip)
  strncpy(ip, inet_ntoa(addr), length);
    return addr.s_addr;
}



//*************************************************************************
// int Connection::WaitTime(unsigned int _wt)
//
int Connection::WaitTime(unsigned int _wt)
{
   wait_time = _wt;
   return OK;
}
