//
// SSLConnection.cc
//
// SSLConnection: This class forms a easy to use interface to the 
//		  socket interface using SSL connection encryption.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2001 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: SSLConnection.h,v 1.3 2003/06/20 11:46:58 lha Exp $
//

#ifndef _SSLConnection_h_
#define	_SSLConnection_h_

#include "Connection.h"

#ifdef HAVE_SSL_H
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

class SSLConnection : public Connection
{
 public:
  SSLConnection();
  SSLConnection(int socket);
  ~SSLConnection();

 protected:
  static SSL_CTX *ctx;
  SSL *ssl;

  void InitSSL();
  int  Connect();
  int  Close();

  int Read_Partial( char *buffer, int maxlength );
  int Write_Partial( char *buffer, int maxlength );
};
#endif

#endif
