//
// Transport.cc
//
// Transport: A virtual transport interface class for accessing
//            remote documents. Used to grab URLs based on the 
//            scheme (e.g. http://, ftp://...)
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
// $Id: Transport.cc,v 1.3 1999/09/29 11:17:04 angus Exp $
//
//

#include "Transport.h"
#include <iostream.h>


#define DEFAULT_CONNECTION_TIMEOUT 15

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

Transport_Response::Transport_Response()
{

   // Initialize the pointers to the HtDateTime objs
   _modification_time = NULL;
   _access_time = NULL;

   // Set the content length and the return status code to negative values
   _content_length = -1;
   _status_code = -1;
   
   // Also set the document length, but to zero instead of -1
   _document_length = 0;

   // Zeroes the contents
   _contents = 0;

}


 ///////
    //    Empty destructor
 ///////

Transport_Response::~Transport_Response()
{

   // Free memory correctly
   
   if(_modification_time)
   {
   	 delete _modification_time;
      _modification_time=NULL;
   }

   if(_access_time)
   {
   	 delete _access_time;
      _access_time=NULL;
   }

}



void Transport_Response::Reset()
{
   // Reset all the field of the object

   // Check if an HtDateTime object exists, and delete it
   if(_modification_time)
   {
   	 delete _modification_time;
      _modification_time=NULL;
   }

   if(_access_time)
   {
   	 delete _access_time;
      _access_time=NULL;
   }

   // Set the content length to a negative value
   _content_length=-1;
   
   // Also set the document length, but to zero instead of -1
   _document_length=0;

   // Zeroes the contents and content type strings
   _contents=0;
   _content_type=0;

   // Set the return status code to a negative value
   _status_code=-1;

   // Zeroes the reason phrase of the s.c.
   _reason_phrase=0;
      
}



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
    //    Destructor
 ///////

Transport::~Transport()
{

   // Close the connection that was still up
   if (CloseConnection() && debug > 3)
	    cout << "Closing previous connection with the remote host" << endl;

}


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


   // Assign the server to the connection

int Transport::AssignConnectionServer()
{
   if (debug > 5)
      cout << "\tAssigning the server (" << _host << ") to the TCP connection" << endl;
      
   if (_connection.assign_server(_host) == NOTOK) return 0;
   
   return 1;
}


   // Assign the remote server port to the connection

int Transport::AssignConnectionPort()
{
   if (debug > 5)
      cout << "\tAssigning the port (" << _port << ") to the TCP connection" << endl;
      
   if (_connection.assign_port(_port) == NOTOK) return 0;
   
   return 1;
}




   // Connect
   // Returns
   // 	 -      0 if failed
   // 	 -     -1 if already connected
   // 	 -      1 if ok

int Transport::Connect()
{
   if (debug > 5)
      cout << "\tConnecting via TCP to (" << _host << ":" << _port << ")" << endl;

   if (isConnected()) return -1; // Already connected
   if ( _connection.connect(1) == NOTOK) return 0;  // Connection failed
   
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


void Transport::SetConnection (char *host, int port)
{

   bool ischanged = false;

   // Checking the connection server   
   if( (char *) _host != host )   	 // server is gonna change
     ischanged=true;

   // Checking the connection port
   if( _port != port )  	 // the port is gonna change
     ischanged=true;

   if (ischanged)
   {
     // Let's close any pendant connection with the old
     // server / port pair
	 
     CloseConnection();
   }

   // Copy the host and port information to the object
   _host = host;
   _port = port;

}

// End of Transport.cc (it's a virtual class anyway!)
