//
// Transport.cc
//
// A virtual transport interface class for accessing remote documents.
// Used to grab URLs based on the scheme (e.g. http://, ftp://...)
//
// Keep constructor and destructor in a file of its own.
// Also takes care of the lower-level connection code.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Transport.cc,v 1.4 1999/07/03 03:50:33 ghutchis Exp $
//
//

#include "Transport.h"

///////
   //    Static variables initialization
///////

   // Debug level
   	 int Transport::debug = 0;


///////
   //    Transport_Response class definition
///////

 ///////
    //    Empty constructor
 ///////

Transport_Response::Transport_Response() { }


 ///////
    //    Empty destructor
 ///////

Transport_Response::~Transport_Response() { }


///////
   //    Transport class definition
///////

 ///////
    //    Constructor
 ///////

Transport::Transport()
{
  _timeout = DEFAULT_CONNECTION_TIMEOUT;
}


 ///////
    //    Empty destructor
 ///////

Transport::~Transport() { }


 ///////
    //    Connection Management
 ///////

   // Open the connection
   // Returns
   // 	 -      0 if failed
   // 	 -     -1 if already open
   // 	 -      1 if ok

int Transport::OpenConnection()
{
   if(_connection.isopen()) return -1; // Already open

   // No open connection
   // Let's open a new one
   
   if(_connection.open() == NOTOK) return 0; // failed

   return 1;
}


   // Connect
   // Returns
   // 	 -      0 if failed
   // 	 -     -1 if already connected
   // 	 -      1 if ok

int Transport::Connect()
{
   if (isConnected()) return -1; // Already connected
   if (_connection.connect(1) == NOTOK) return 0;  // Connection failed
   
   return 1;	// Connected
}



   // Close the connection
   // Returns
   // 	 -      0 if not open
   // 	 -      1 if closed ok


int Transport::CloseConnection()
{
   if(_connection.isopen())
   	 _connection.close(); 	// Close the connection
   else return 0;
   
   return 1;
}


// End of Transport.cc (it's a virtual class anyway!)
