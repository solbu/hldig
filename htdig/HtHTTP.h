///////
   //
   // HtHTTP.h
   //
   // Class for HTTP messaging
   // Gabriele Bartolini - Prato - Italia
   // started: 03.05.1999
   //
   // ////////////////////////////////////////////////////////////
   //
   // The HtHTTP class should provide (as I hope) an interface for
   // retrieving document on the Web.
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
   // To build up an HTTP connection you must set it before, by calling
   // the SetHttpConnection method. Here you only have to specify the
   // server you want to be connect with and its TCP port.
   //
   // Then you'll have to set the host and the document to be retrieved.
   // This can be done by calling the SetRequestHost and SetRequestDocument.
   //
   // Once done, you could invoke the Request method with a HttpRequestMethod
   // enum as parameter (the default, with no argument is GET method, but you
   // could do the HEAD method too).
   //
   // You can also set information regarding only a request or a series of them.
   // The info regarding a single request are:
   // - the host (obviously, but not that much)
   // - the document (obviusly again)
   // - the modification time (to be used for If-Modified-Since directive)
   // Infos regarding a series of them are:
   // - user agent
   // - maximum size of a document
   //
   // For example, if you wanna ask the server www.po-net.prato.it for
   // the /home.htm document, you should do these steps:
   //
   // Instantiate an HtHTTP object, for example:
   //    HtHTTP try;
   //
   // Set the user agent, and the max document size, depending on Config values
   // Also set the timeout for the whole HtHTTP connections (static)
   //
   // Set the connection:
   //    try.SetHttpConnection("www.po-net.prato.it", 80);
   //    try.SetRequestHost("www.po-net.prato.it");
   //    try.SetRequestDocument("/home.htm");
   //
   // And ask the document:
   //    try.Request();    // GET is the default
   // If I had to do a HEAD request, the method is this:
   //    try.Request(HtHTTP::Method_HEAD);
   //
   // Now, all I need is inside the try._response object, only accessible
   // from outside, through GetResponse function.
   //
   // The managing of a Proxy connection should be made by the outside of
   // this class, by specifying the server and the port of the connection
   // as the proxy ones and the host and the document as the request ones.
   //
///////


#ifndef _HTHTTP_H
#define _HTHTTP_H

#include "URL.h"
#include "htString.h"
#include "HtDateTime.h"
#include "Connection.h"

#define DEFAULT_CONNECTION_TIMEOUT 15
#define DEFAULT_MAX_DOCUMENT_SIZE	 100000


// In advance declarations

class HtHTTP;


class HtHTTP_Response
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

	 // Get the Content type
   	 char *GetContentType() { return _content_type; }

	 // Get the Content length
   	 int GetContentLength() const { return _content_length; }

   	 // Get the contents
   	 char *GetContents() { return _contents; }
	 
	 // Get the modification time object pointer
	 HtDateTime *GetModificationTime() const { return _modification_time; }

	 // Get the access time object pointer
	 HtDateTime *GetAccessTime() const { return _access_time; }


   protected:

   // Status line information
   
   	 String	   _version;	   	    // HTTP Version
	 int   	   _status_code;  	    // return Status code
	 String	   _reason_phrase;	    // status code reason phrase

   // Other header information
   
	 String	   _content_type; 	    // Content-type returned by the server
	 int   	   _content_length;     // Content-length returned by the server
	 int   	   _document_length;    // Length really stored
	 HtDateTime *_modification_time;  // Modification time returned by the server
	 HtDateTime *_access_time;  	    // Access time returned by the server
	 String 	   _location;	   	    // Location (in case of redirect)

   // Body of the response message

	 String	   _contents;	   	    // Contents of the document
	 
};



class HtHTTP
{
public:

///////
   //    Construction/Destruction
///////

    HtHTTP();
    ~HtHTTP();

   // Information about the status of a document retrieving
   
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

   // Information about the method to be used in the request

   enum HttpRequestMethod
   {
   	 Method_GET,
	 Method_HEAD
   };
   

///////
   //    Sends an HTTP request message
///////

   // Sends a Method request message (by passing an HttpRequestMethod enum value)
   DocStatus Request (HttpRequestMethod = Method_GET);
   

///////
   //    Control of member the variables
///////

 ///////
    //    Interface for the connection
 ///////

   // Set Connection parameters
   void SetHttpConnection (char *server, int port);

   // Set and get the connection time out value
   static void SetTimeOut ( int t ) { _timeout=t; }
   static int GetTimeOut () { return _timeout; }


 ///////
    //    Interface for the HTTP Request
 ///////

   // Set and get the host
   void SetRequestHost(char *h) { _host = h; }
   char *GetRequestHost() const { return _host; }

   // Set and get the document to be retrieved
   void SetRequestDocument(char *d) { _document = d; }
   char *GetRequestDocument () const { return _document; }

   // Set the modification date and time for If-Modified-Since   
   void SetRequestModificationTime (HtDateTime *p) { _modification_time=p; }
   void SetRequestModificationTime (HtDateTime &p)
   	       	   	       	   	    { SetRequestModificationTime (&p) ; }
   // Get the modification date time
   HtDateTime *GetRequestModificationTime () { return _modification_time; }

   // Get the date time information about the request
   const HtDateTime *GetStartTime() const { return &_start_time; }
   const HtDateTime *GetEndTime() const { return &_end_time; }


   // Info for multiple requests (static)
   // Get the User agent string
   static void SetRequestUserAgent (char *s) { _user_agent=s; }
   static char *GetRequestUserAgent() { return _user_agent; }

   // Get and set the max document size to be retrieved
   static void SetRequestMaxDocumentSize (int t) { _max_document_size=t; }
   static int GetRequestMaxDocumentSize() { return _max_document_size; }


 ///////
    //    Interface for the HTTP Response
 ///////

   const HtHTTP_Response *GetResponse() const { return &_response; }
   
   // Get the document status 
   static DocStatus GetDocumentStatus(HtHTTP_Response &);


///////
   //    Querying the status of the connection
///////


   // Are we still connected?
   bool isConnected();

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


// Set the debug level   
   static void SetDebugLevel (int d) { debug=d;}   
	
protected:

///////
   //    Member attributes
///////

   ///////
   	 //    Http Connection information
   ///////

   Connection  _connection;	   // Connection object

   static int	_timeout;     	   // Connection Time out (static)

   char *   	_server;    	   // Server or proxy to connect with
   int      	_port;       	   // Port of the remote host or proxy


   ///////
      //    Http single Request information (Member attributes)
   ///////

   char *   	_document;  	   // Document to be retrieved
   char *   	_host;     	   // HTTP host
   
   HtDateTime *_modification_time;  // Set the modification time for
   	       	   	       	   	 // If-Modified-Since

   HtDateTime  _start_time;	   	 // Start time of the request
   HtDateTime  _end_time;	   	 // end time of the request

   int      	_bytes_read;	   	 // Bytes read

   
   ///////
      //    Http multiple Request information
   ///////

   static int      	_max_document_size;  // Max document size to retrieve

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
      //    Debug level
   ///////

   static int debug;
   

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
    //    Hidden Interface for the connection
 ///////

   // Set and get the remote server to be connected with
   
   void SetConnectionServer(char *server) { _server = server; }
   char *GetConnectionServer () const { return _server; }

   // Set and get the remote host port to be connected with
   
   void SetConnectionPort(int port) { _port = port; }     // Set the remote host port
   int GetConnectionPort () const { return _port; } 	   // and get it


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


   ///////
      //    Services about HTTP connection
   ///////

   // Open the connection
   
   inline int OpenConnection();

   // Assign the host and the port for the connection
   
   inline int AssignConnectionServer();
   inline int AssignConnectionPort();   

   // Connect to the specified host and port
   inline int Connect();
   
   // Write a message
   inline int ConnectionWrite(char *);

   // Assign the timeout to the connection (returns the old value)

   inline int AssignConnectionTimeOut();
   
   // Close the connection
   
   inline int CloseConnection();


   // Finish the request and return a DocStatus value;

   DocStatus FinishRequest (DocStatus);


///////
   //    Static attributes and methods
///////
   
   // Statistics about requests
   static int	   _tot_seconds;  	 // Requests last (in seconds)
   static int	   _tot_requests; 	 // Number of requests
   static int	   _tot_bytes;    	 // Number of bytes read

   // Retrieve statistics
   
   static int GetTotSeconds () { return _tot_seconds; }   
   static int GetTotRequests () { return _tot_requests; }   
   static int GetTotBytes () { return _tot_bytes; }   
   
   
};

#endif

