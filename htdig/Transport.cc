//
// Transport.cc
//
// A virtual transport interface class for accessing remote documents.
// Used to grab URLs based on the scheme (e.g. http://, ftp://...)
//
// Keep constructor and destructor in a file of its own.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Transport.cc,v 1.2 1999/06/25 21:17:35 ghutchis Exp $
//
//

#include "Transport.h"

Transport_Response::Transport_Response()
{ }

Transport_Response::~Transport_Response()
{ }

Transport::Transport()
{
  _timeout = DEFAULT_CONNECTION_TIMEOUT;
}

Transport::~Transport()
{ }

// End of Transport.cc (it's a virtual class anyway!)
