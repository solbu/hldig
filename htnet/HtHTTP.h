//
// HtHTTP.h
//
// HtHTTP: Class for HTTP messaging (derived from Transport)
//
// Gabriele Bartolini - Prato - Italia
// started: 03.05.1999
//
// ////////////////////////////////////////////////////////////
//
// The HtHTTP class should provide (as I hope) an interface for
// retrieving document on the Web. It derives from Transport class.
//
// It should be HTTP/1.1 compatible.
//
// It also let us take advantage of persitent connections use,
// and optimize request times (specially if directed to the same
// server).
//
// HtHTTP use another class to store the response returned by the
// remote server.
//
///////
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtHTTP.h,v 1.2 1999/09/27 16:18:58 angus Exp $ 
//

#ifndef _HTHTTP_H
#define _HTHTTP_H

#include "Transport.h"
#include "URL.h"
#include "htString.h"


// In advance declarations

class HtHTTP;


class HtHTTP_Response : public Transport_Response
{

   friend class HtHTTP;    // declaring friendship
   
   public:
///////
   //    Construction / Destruction
///////
	 
      HtHTTP_Response();
      ~HtHTTP_Response();


///////
   //    Interface
///////

   	 // Reset
	 void Reset();
	 
	 // Get the HTTP version
   	 char *GetVersion() { return _version; }

	 // Get the Status Code
   	 int GetStatusCode() const { return _status_code; }

	 // Get the Status Code reason phrase
   	 char *GetReasonPhrase() { return _reason_phrase; }


   protected:

   // Status line information
   
   	 String	   _version;	          // HTTP Version
	 int   	   _status_code;  	  // return Status code
	 String	   _reason_phrase;	  // status code reason phrase

   // Other header information
   
	 String      _location;	          // Location (in case of redirect)

};



class HtHTTP : public Transport
{
public:

///////
   //    Construction/Destruction
///////

    HtHTTP();
    ~HtHTTP();

   // Information about the method to be used in the request

   enum Request_Method
   {
   	 Method_GET,
	 Method_HEAD
   };
   


///////
   //    Sends an HTTP request message
///////

   // Sends a Method request message
   virtual DocStatus Request ();
   

///////
   //    Control of member the variables
///////

 ///////
    //    Set the Request Method
 ///////

   void SetRequestMethod (Request_Method rm) { _Method = rm; }
   Request_Method GetRequestMethod() { return _Method; }
   

 ///////
    //    Interface for resource retrieving
 ///////

   // Set and get the document to be retrieved
   void SetRequestURL(URL &u) { _url = u;}
   URL GetRequestURL () { return _url;}


   // Set and get the referring URL
   void SetRefererURL (URL u) { _referer = u;}
   URL GetRefererURL () { return _referer;}


   // Get the date time information about the request
   const HtDateTime *GetStartTime() const { return &_start_time; }
   const HtDateTime *GetEndTime() const { return &_end_time; }


   // Info for multiple requests (static)
   // Get the User agent string
   static void SetRequestUserAgent (char *s) { _user_agent=s; }
   static char *GetRequestUserAgent() { return _user_agent; }


   // Set (Basic) Authentication Credentials
   void SetCredentials (String s);

 ///////
    //    Interface for the HTTP Response
 ///////

   Transport_Response *GetResponse() { return &_response; }

   // Get the document status 
   virtual DocStatus GetDocumentStatus()
      { return GetDocumentStatus (_response); }
   
   // It's a static method
   static DocStatus GetDocumentStatus(HtHTTP_Response &);



///////
   //    Persistent connection choices interface
///////

   // Is allowed
   bool isPersistentConnectionAllowed() {return _persistent_connection_allowed;}
   
   // Is possible
   bool isPersistentConnectionPossible() {return _persistent_connection_possible;}

   // Check if a persistent connection is possible depending on the HTTP version
   void CheckPersistentConnection(const char *);

   // Is Up (is both allowed and permitted by the server too)
   bool isPersistentConnectionUp()
   	    { return isPersistentConnectionAllowed() &&
   	       	   isPersistentConnectionPossible(); }
   
   // Allow Persistent Connection
   void AllowPersistentConnection() { _persistent_connection_allowed=true; }
   
   // Disable Persistent Connection
   void DisablePersistentConnection() { _persistent_connection_allowed=false; }
   

// Manage statistics
   
   static double GetAverageRequestTime ()
   	 { return _tot_seconds?( ((double) _tot_seconds) / _tot_requests) : 0; }   

   static float GetAverageSpeed ()
   	 { return _tot_bytes?( ((double) _tot_bytes) / _tot_seconds) : 0; }   

   static void ResetStatistics ()
   	 { _tot_seconds=0; _tot_requests=0; _tot_bytes=0;}   

// Set the modification_time_is_now static attribute
   static void SetModificationTimeIsNow (int d) { modification_time_is_now=d;}   

// Set the controller for the parsing check. That is to say
// that External function that checks if a document is parsable or not.
// CanBeParsed static attribute should point to a function
// that returns an int value, given a char * containing the content-type.

   static void SetParsingController (int (*f)(char*)) { CanBeParsed = f; }

	
protected:

///////
   //    Member attributes
///////

   Request_Method    _Method;
   
   ///////
      //    Modification Time is Now config attribute
   ///////

   static int modification_time_is_now;

   ///////
      //    Http single Request information (Member attributes)
   ///////

   HtDateTime  _start_time;	   	 // Start time of the request
   HtDateTime  _end_time;	   	 // end time of the request

   int      	_bytes_read;	   	 // Bytes read

   URL		_url;                  // URL to retrieve

   URL		_referer;	       // Referring URL
   
   
   ///////
      //    Http multiple Request information
   ///////

   static String   	_user_agent;  	   // User agent
   


   ///////
      //    Http Response information
   ///////

   HtHTTP_Response	 _response; 	 // Object where response
   	       	   	       	   	 // information will be stored into


   ///////
      //    Allow or not a persistent connection (user choice)
   ///////

   bool _persistent_connection_allowed;


   ///////
      //    Is a persistent connection possible (with this http server)?
   ///////

   bool _persistent_connection_possible;


///////
   //    Enum
///////

   // Information about the status of a connection
   
   enum ConnectionStatus
   {
   	 Connection_ok,
	 Connection_already_up,
   	 Connection_open_failed,
   	 Connection_no_server,
   	 Connection_no_port,
   	 Connection_failed
   };


///////
   //    Protected Services or method (Hidden by outside)
///////


   ///////
      //    Establish the connection
   ///////

   ConnectionStatus EstablishConnection ();
   

   ///////
      //    Set the string of the command containing the request
   ///////

   void SetRequestCommand(String &);


   ///////
      //    Parse the header returned by the server
   ///////

   int ParseHeader();


   enum DateFormat
   {
   	 DateFormat_RFC1123,
	 DateFormat_RFC850,
	 DateFormat_AscTime,
	 DateFormat_NotRecognized
   };


   ///////
      //    Create a new HtDateTime object
   ///////

   HtDateTime *NewDate(const char *);

   ///////
      //    Recognize Date Format
   ///////

   DateFormat RecognizeDateFormat (const char *);


   ///////
      //    Check if a document is parsable looking the content-type info
   ///////

   static bool isParsable(const char *);

   ///////
      //    Read the body returned by the server
   ///////

   int ReadBody();


   // Finish the request and return a DocStatus value;

   DocStatus FinishRequest (DocStatus);


///////
   //    Static attributes and methods
///////
   
   // Statistics about requests
   static int  _tot_seconds;  	 // Requests last (in seconds)
   static int  _tot_requests; 	 // Number of requests
   static int  _tot_bytes;    	 // Number of bytes read

   // Retrieve statistics
   
   static int GetTotSeconds () { return _tot_seconds; }   
   static int GetTotRequests () { return _tot_requests; }   
   static int GetTotBytes () { return _tot_bytes; }   

   // This is a pointer to function that check if a ContentType
   // is parsable or less.
   
   static int (*CanBeParsed) (char *);
   
};

#endif

