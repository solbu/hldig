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
// $Id: Transport.cc,v 1.5.2.4 1999/11/28 02:46:08 ghutchis Exp $
//
//

#include "Transport.h"

#include <iomanip.h>
#include <ctype.h>

#define DEFAULT_CONNECTION_TIMEOUT 15

///////
   //    Static variables initialization
///////

   // Debug level
   	 int Transport::debug = 0;

   // Statistics
   	 int Transport::_tot_open = 0;
   	 int Transport::_tot_close = 0;
   	 int Transport::_tot_changes = 0;


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
  _port = -1;     // Initialize port
  _host = 0;      // Initialize the host
  _max_document_size = 0;
  _timeout = DEFAULT_CONNECTION_TIMEOUT;
}


 ///////
    //    Destructor
 ///////

Transport::~Transport()
{

   // Close the connection that was still up
   if (CloseConnection())
      if ( debug > 4 )
         cout  << setw(5) << GetTotOpen() << " - "
            << "Closing previous connection with the remote host" << endl;

}

///////
   //    Show the statistics
///////

ostream &Transport::ShowStatistics (ostream &out)
{
   out << " Connections opened        : " << GetTotOpen() << endl;
   out << " Connections closed        : " << GetTotClose() << endl;
   out << " Changes of server         : " << GetTotServerChanges() << endl;
         
   return out;

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
   if(_connection.isopen() && _connection.isconnected())
      return -1; // Already open and connection is up

   // No open connection
   // Let's open a new one
   
   if(_connection.open() == NOTOK) return 0; // failed

   _tot_open ++;
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


   // Flush the connection
   
void Transport::FlushConnection()
{
   _connection.flush();
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

   _tot_close ++;
   
   return 1;
}


void Transport::SetConnection (const String &host, int port)
{

   if (_port != -1)
   {
      // Already initialized
      // Let's check if the server or the port are changed
      
      bool ischanged = false;

      // Checking the connection server   
      if(_host != host)   	 // server is gonna change
        ischanged=true;

      // Checking the connection port
      if( _port != port )  	 // the port is gonna change
        ischanged=true;

      if (ischanged)
      {
        // Let's close any pendant connection with the old
        // server / port pair

         _tot_changes ++;
      
         if ( debug > 4 )
            cout  << setw(5) << GetTotOpen() << " - "
               << "Change of server. Previous connection closed." << endl;
      
        CloseConnection();
      }
   }

   // Copy the host and port information to the object
   _host = host;
   _port = port;

}


// Create a new date time object containing the date specified in a string
HtDateTime *Transport::NewDate(const char *datestring)
{

   while(isspace(*datestring)) datestring++; // skip initial spaces

   DateFormat df = RecognizeDateFormat (datestring);

   if(df == DateFormat_NotRecognized)
   {
   	 // Not recognized
	 if(debug > 0)
   	 	 cout << "Date Format not recognized: " << datestring << endl;
	 
	 return NULL;
   }

   HtDateTime *dt = new HtDateTime;
   
   dt->ToGMTime(); // Set to GM time
   
   switch(df)
   {
	       // Asc Time format
   	 case DateFormat_AscTime:
   	       	dt->SetAscTime((char *)datestring);
   	 	       break;
	       // RFC 1123
   	 case DateFormat_RFC1123:
   	       	dt->SetRFC1123((char *)datestring);
   	 	       break;
		  // RFC 850
   	 case DateFormat_RFC850:
   	       	dt->SetRFC850((char *)datestring);
   	 	       break;
         default:
	        cout << "Date Format not handled: " << (int)df << endl;
	        break;
   }

   return dt;
   
}


// Recognize the possible date format sent by the server
Transport::DateFormat Transport::RecognizeDateFormat (const char *datestring)
{
   register char *s;
   
   if((s=strchr(datestring, ',')))
   {
   	 // A comma is present.
	 // Two chances: RFC1123 or RFC850

   	 if(strchr(s, '-'))
	    return DateFormat_RFC850;  // RFC 850 recognized   
	 else
	    return DateFormat_RFC1123; // RFC 1123 recognized
   }
   else
   {
      // No comma present
	 
	 // Let's try C Asctime:    Sun Nov  6 08:49:37 1994
	 if(strlen(datestring) == 24) return DateFormat_AscTime;
   }
   
   return DateFormat_NotRecognized;
   
}

// End of Transport.cc (it's a virtual class anyway!)
