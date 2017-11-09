//
// SSLConnection.cc
//
// SSLConnection: This class forms a easy to use interface to the 
//		  socket interface using SSL connection encryption.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: SSLConnection.cc,v 1.6 2004/05/28 13:15:23 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_SSL_H
#include "SSLConnection.h"

#ifdef HAVE_STD
#include <iostream>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif
#else
#include <iostream.h>
#endif /* HAVE_STD */

// Global needed only once in HtDig
//
SSL_CTX *SSLConnection::ctx = NULL;

SSLConnection::SSLConnection()
{
  InitSSL();
}

SSLConnection::SSLConnection(int socket)
{
  InitSSL();
}
  

void SSLConnection::InitSSL()
{
  if( ctx == NULL )
    {
      // Add in the OpenSSL algorithms ??
      //
      OpenSSL_add_ssl_algorithms();

      // Load the error strings ... openssl says so
      //
      SSL_load_error_strings();
 
      // New CTX, either v3 but can default to v2
      //
      ctx = SSL_CTX_new (SSLv23_client_method());        

      if( ctx == NULL )
	{
	  cout << "ctx NULL" << endl;
	  exit(1);
	}
    }
}

SSLConnection::~SSLConnection()
{
  if( ctx != NULL )
    SSL_CTX_free (ctx);
  ctx = NULL;
}

int SSLConnection::Connect()
{
  // Run the standard connect
  //
  int ret = Connection::Connect();

  if( ret != OK )
    return ret;

  // Now start SSL negotiation
  //
  ssl = SSL_new (ctx);

  // Attach ssl to socket
  //
  SSL_set_fd (ssl, sock);
  int err = SSL_connect (ssl);
  if( err == -1) 
    {
      // cout << "SSL_connect failed!" << endl;
      // Close the socket
      //
      Connection::Close();

      return NOTOK;
    }
  return OK;
}

int SSLConnection::Close()
{
  // First kill the SSL stuff
  //
  SSL_shutdown (ssl);  /* send SSL/TLS close_notify */
 
  /* Clean up. */

  // Now call the Connection Close
  //
  int ret = Connection::Close();
  if( ret == OK )
    {
      SSL_free (ssl);
    }
  return ret;
}

int SSLConnection::Read_Partial(char *buffer, int maxlength)
{
    int		count;

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
          count = SSL_read(ssl, buffer, maxlength);
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
int SSLConnection::Write_Partial(char *buffer, int maxlength)
{
    int		count;

    do
    {
	count = SSL_write(ssl, buffer, maxlength);
    }
    while (count < 0 && errno == EINTR && !need_io_stop);
    need_io_stop = 0;

    return count;
}

#endif
