//
// Transport.h
//
// A virtual pure transport interface class for accessing remote documents.
// Used to grab URLs based on the scheme (e.g. http://, ftp://...)
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Transport.h,v 1.5 1999/07/03 20:55:58 ghutchis Exp $
//
//

#ifndef _Transport_H
#define _Transport_H

#include "Object.h"
#include "HtDateTime.h"
#include "URL.h"
#include "htString.h"
#include "Connection.h"

#define DEFAULT_CONNECTION_TIMEOUT 15

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

   // Get the Content type
   virtual String GetContentType() = 0;

   // Get the Content length
   virtual int GetContentLength() const = 0;

   // Get the contents
   virtual String GetContents() = 0;

   // Get the modification time object pointer
   virtual HtDateTime *GetModificationTime() const = 0;

   // Get the access time object pointer
   virtual HtDateTime *GetAccessTime() const = 0;
   
   protected:

      // Empty so far
   
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
      Document_no_header
   };


   // Make the request
   virtual DocStatus Request () { return Document_not_found;}
   
   // Set and get the connection time out value
   void SetTimeOut ( int t ) { _timeout=t; }
   int GetTimeOut () { return _timeout; }
  
   // Set and get the document to be retrieved
   void SetRequestURL(URL u) { _url = u;}
   URL GetRequestURL () { return _url;}

   // Set and get the referring URL
   void SetRefererURL (URL u) { _referer = u;}
   URL GetRefererURL () { return _referer;}

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
   GetRequestMaxDocumentSize() const { return _max_document_size; }

   virtual const Transport_Response *GetResponse() { return 0;}
   
   virtual DocStatus GetDocumentStatus(Transport_Response &) 
      { return Document_not_found; }

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

   Connection	_connection;	       // Connection object
   int		_timeout;              // Connection timeout

   URL		_url;                  // URL to retrieve
   URL		_referer;	       // Referring URL
   
   HtDateTime	*_modification_time;   // Stored modification time if avail.
   int		_max_document_size;    // Max document size to retrieve

   String	_credentials;	       // Credentials for this connection

   ///////
      //    Services about a Transport layer connection
      //    They're invisible from outside
   ///////

   // Open the connection
   
   inline int OpenConnection();

   // Assign the host and the port for the connection
   
   inline int AssignConnectionServer();
   inline int AssignConnectionPort();   

   // Connect to the specified host and port
   inline int Connect();
   
   // Write a message
   virtual int ConnectionWrite(char *cmd)
      { return _connection.write(cmd); }


   // Assign the timeout to the connection (returns the old value)

   int AssignConnectionTimeOut()
      { return _connection.timeout(_timeout); }

      
   // Close the connection
   
   inline int CloseConnection();


   ///////
      //    Debug level
   ///////

   static int debug;

};

#endif

