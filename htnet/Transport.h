//
// Transport.h
//
// Transport: A virtual transport interface class for accessing
//            remote documents. Used to grab URLs based on the 
//            scheme (e.g. http://, ftp://...)
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Transport.h,v 1.4 1999/10/04 15:46:23 angus Exp $
//
//

#ifndef _Transport_H
#define _Transport_H

#include "Object.h"
#include "HtDateTime.h"
#include "htString.h"
#include "URL.h"
#include "Connection.h"

// Declare in advance
class Transport;

// But first, something completely different. Here's the response class
class Transport_Response : public Object
{
   friend class Transport;    // declaring friendship
   
   public:

///////
   //    Construction / Destruction
///////
   
   Transport_Response();
   virtual ~Transport_Response();

   // Reset the information stored
   virtual void Reset();  // This function must be defined

   // Get the contents
   virtual String GetContents() { return _contents; }
	 
   // Get the modification time object pointer
   virtual HtDateTime *GetModificationTime() const { return _modification_time; }

   // Get the access time object pointer
   virtual HtDateTime *GetAccessTime() const { return _access_time; }

   // Get the Content type
   virtual String GetContentType() { return _content_type; }

   // Get the Content length
   virtual int GetContentLength() const { return _content_length; }

   // Get the Document length (really stored)
   virtual int GetDocumentLength() const { return _document_length; }

   // Get the Status Code
   virtual int GetStatusCode() const { return _status_code; }

   // Get the Status Code reason phrase
   char *GetReasonPhrase() { return _reason_phrase; }



   
   protected:

   // Body of the response message

	 String	   _contents;	   	    // Contents of the document

	 HtDateTime  *_modification_time; // Modification time returned by the server
	 HtDateTime  *_access_time;       // Access time returned by the server

	 String	     _content_type; 	  // Content-type returned by the server
	 int   	     _content_length;     // Content-length returned by the server
	 int   	     _document_length;    // Length really stored

	 int   	   _status_code;  	  // return Status code
	 String	   _reason_phrase;	  // status code reason phrase
   
};


///////
   //    Transport class declaration
///////

class Transport : public Object
{

 public:

///////
   //    Construction / Destruction
///////

   Transport();
   virtual ~Transport();

///////
   //    Enumeration of possible return status of a resource retrieving
///////

   enum DocStatus
   {
      Document_ok,
      Document_not_changed,
      Document_not_found,
      Document_not_parsable,
      Document_redirect,
      Document_not_authorized,
      Document_connection_down,
      Document_no_header,
      Document_no_host,
      Document_not_local,
      Document_not_recognized_service,    // Transport service not recognized
      Document_other_error                // General error (memory)
   };


///////
   //    Connects to an host and a port
   //    Overloaded methods provided in order to take
   //     the info from a URL obj or ptr
///////

   // Set Connection parameters
   virtual void SetConnection (char *host, int port);

   // from a URL pointer
   virtual void SetConnection (URL *u)
      { SetConnection (u->host(), u->port()); }

   // from a URL object
   virtual void SetConnection (URL &u)
      { SetConnection (&u); }



   // Make the request
   virtual DocStatus Request() = 0; // different in derived classes


   // Set and get the connection time out value
   void SetTimeOut ( int t ) { _timeout=t; }
   int GetTimeOut () { return _timeout; }

   // Get the Connection Host
   char *GetHost() { return _host; }

   // Get the Connection Host
   int GetPort() { return _port; }
   
   // Set and get the credentials
   // Likely to vary based on transport protocol
   virtual void SetCredentials (String s) { _credentials = s;}
   virtual String GetCredentials () { return _credentials;}

   // Set the modification date and time for If-Modified-Since   
   void SetRequestModificationTime (HtDateTime *p) { _modification_time=p; }
   void SetRequestModificationTime (HtDateTime &p)
      { SetRequestModificationTime (&p) ;}

   // Get the modification date time
   HtDateTime *GetRequestModificationTime () { return _modification_time; }

   // Get and set the max document size to be retrieved
   void SetRequestMaxDocumentSize (int s) { _max_document_size=s; }
   int GetRequestMaxDocumentSize() const { return _max_document_size; }

   virtual Transport_Response *GetResponse() = 0;
   
   virtual DocStatus GetDocumentStatus() = 0; 

///////
   //    Querying the status of the connection
///////

   // Are we still connected?
   // This is the only part regarding
   // a connection that's got a public access

   virtual bool isConnected(){ return _connection.isconnected(); }


// Set the debug level   
   static void SetDebugLevel (int d) { debug=d;}   


protected:

   ///////
      //    Services about a Transport layer connection
      //    They're invisible from outside
   ///////

   // Open the connection
   
   int OpenConnection();

   // Assign the host and the port for the connection
   
   int AssignConnectionServer();
   int AssignConnectionPort();   

   // Connect to the specified host and port
   int Connect();
   
   // Write a message
   inline int ConnectionWrite(char *cmd)
      { return _connection.write(cmd); }


   // Assign the timeout to the connection (returns the old value)

   inline int AssignConnectionTimeOut()
      { return _connection.timeout(_timeout); }

   // Flush the connection
   void FlushConnection();
      
   // Close the connection
   
   int CloseConnection();



 protected:

   Connection	_connection;	       // Connection object

   String       _host;                 // TCP Connection host
   int          _port;                 // TCP Connection port
   
   int		_timeout;              // Connection timeout

   HtDateTime	*_modification_time;   // Stored modification time if avail.
   int		_max_document_size;    // Max document size to retrieve

   String	_credentials;	       // Credentials for this connection



   ///////
      //    Debug level
   ///////

   static int debug;

};

#endif

