//
// HtHTTP.cc
//
// HtHTTP: Interface classes for HTTP messaging
//
// Including:
// 	 -  Generic class
// 	 -  Response message class
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtHTTP.cc,v 1.6 1999/10/04 15:46:23 angus Exp $ 
//

#include "lib.h"
#include "Transport.h"
#include "HtHTTP.h"

#include <signal.h>
#include <sys/types.h>
#include <ctype.h>
#include <iostream.h>
#include <stdio.h> // for sscanf


#if 1
typedef void (*SIGNAL_HANDLER) (...);
#else
typedef SIG_PF SIGNAL_HANDLER;
#endif

   // Modification Time is now attribute
         int HtHTTP::modification_time_is_now = 0;
   // User Agent
   	 String HtHTTP::_user_agent = 0;

   // Stats information
   	 int HtHTTP::_tot_seconds = 0;
   	 int HtHTTP::_tot_requests = 0;
   	 int HtHTTP::_tot_bytes = 0;

   // flag that manage the option of 'HEAD' before 'GET'
         bool HtHTTP::_head_before_get = true;

   // Handler of the CanParse function

         int (* HtHTTP::CanBeParsed) (char *) = NULL;

///////
   //    HtHTTP_Response class
   //
   //    Response message sent by the remote HTTP server
///////


// Construction

HtHTTP_Response::HtHTTP_Response()
{
   // Initialize the version, transfer-encoding, location and server strings
   _version = 0;
   _transfer_encoding = 0;
   _location = 0;
   _server = 0;
         
}


// Destruction

HtHTTP_Response::~HtHTTP_Response()
{
   // Delete the HtDateTime Object, if present
   
   if(_modification_time) delete _modification_time;

   if(_access_time) delete _access_time;
      
}


void HtHTTP_Response::Reset()
{
   // Call the base class method in order to reset
   // the base class attributes

   Transport_Response::Reset();

   // Initialize the version, transfer-encoding, location and server strings
   _version = 0;
   _transfer_encoding = 0;
   _location = 0;
   _server = 0;

}




///////
   //    HtHTTP generic class
   //
   //    
///////


// Construction

HtHTTP::HtHTTP()
{
   _modification_time=NULL;
   _persistent_connection_possible = false;
   _persistent_connection_allowed = true; // by default

   _bytes_read=0;

   // Default Method Request
   _Method = Method_GET;
}

// Destruction

HtHTTP::~HtHTTP()
{
   // It's empty
}


///////
   //    Manages the requesting process
///////

Transport::DocStatus HtHTTP::Request()
{

   DocStatus result = Document_ok;

///////
   //    We make a double request (HEAD and, maybe, GET)
   //    Depending on the 
///////
   
   if (HeadBeforeGet() &&                 // Option value to true
      isPersistentConnectionAllowed() &&  // Persistent Connections allowed
      _Method == Method_GET)              // Initial request method is GET
   {
      _Method = Method_HEAD;
         
      result = HTTPRequest();
      
      _Method = Method_GET;
   }
   
   if (result == Document_ok)
      result = HTTPRequest();
      
   return result;
}


///////
   //    Sends an HTTP 1/1 request
///////

Transport::DocStatus HtHTTP::HTTPRequest()
{

   static Transport::DocStatus DocumentStatus;
   bool ShouldTheBodyBeRead = true;

   SetBodyReadingController(&ReadBody);

   // Reset the response
   _response.Reset();

   // Flush the connection
   FlushConnection();

   _bytes_read=0;

   if( debug > 4)
   	 cout << "Try to get through to host "
	      << _url.host() << " (port " << _url.port() << ") via HTTP" << endl;

   ConnectionStatus result;

   // Start the timer
   _start_time.SettoNow();
   
   result = EstablishConnection();
   
   if(result != Connection_ok && result != Connection_already_up)
   {

   	 if(debug>0)

   	    switch (result)
   	    {
   	       // Open failed
		  
	       case Connection_open_failed:
		  	cout << "Unable to open the connection with host: "
			   << _url.host() << " (port " << _url.port() << ")" << endl;
			break;

   	       // Server not reached
	       case Connection_no_server:
		  	cout << "Unable to find the host: "
			   << _url.host() << " (port " << _url.port() << ")" << endl;
			break;
	    
   	       // Port not reached
	       case Connection_no_port:
		  	cout << "Unable to connect with the port " << _url.port()
   	       	   << " of the host: " << _url.host() << endl;
			break;
	    
   	       // Connection failed
	       case Connection_failed:
		  	cout << "Unable to establish the connection with host: "
			   << _url.host() << " (port " << _url.port() << ")" << endl;
			break;
	       default:
		  	cout << "connection failed with unexpected result: result = "
			     << (int)result << ", "
			   << _url.host() << " (port " << _url.port() << ")" << endl;
			break;
	    }
	 
   	 return FinishRequest(Document_not_found);

   }

   // Visual comments about the result of the connection
   if (debug > 5)
   	 switch(result)
      {
	    case Connection_already_up:
   	       cout << "Taking advantage of persistent connections" << endl;
   	       break;
	    case Connection_ok:
   	       cout << "New connection open successfully" << endl;
	       break;
	    default:
	       cout << "Unexptected value: " << (int)result << endl;
	       break;
   }

   String command;

   switch(_Method)
   {
      case Method_GET:
   	    command = "GET ";
            break;
      case Method_HEAD:
            command = "HEAD ";
            ShouldTheBodyBeRead = false;
      break;
   }

   // Set the request command
   
   SetRequestCommand(command);

   if (debug > 6)
      cout << "Request\n" << command;

   // Writes the command
   ConnectionWrite(command);

   // Assign the timeout
   AssignConnectionTimeOut();

   // Parse the header
   if (ParseHeader() == -1) // Connection down
   {
	    // The connection probably fell down !?!
   	 	 if ( debug > 4 )
	       cout << "Connection fell down ... let's close it" << endl;
	    
	    CloseConnection();	// Let's close the connection which is down now
	    
	    // Return that the connection has fallen down during the request
	    return Document_connection_down;
   }   


   if (_response._status_code == -1)
   {
   	 // Unable to retrieve the status line

   	 if ( debug > 4 )
	    cout << "Unable to retrieve or parse the status line" << endl;
	 
   	 return Document_no_header;
   }   
      

   if (debug > 3)
   {

      cout << "Retrieving document " << _url.path() << " on host: "
         << _url.host() << ":" << _url.port() << endl;
		  
      cout << "Http version      : " << _response._version << endl;
      cout << "Server            : " << _response._version << endl;
      cout << "Status Code       : " << _response._status_code << endl;
      cout << "Reason            : " << _response._reason_phrase << endl;

      if (_response.GetAccessTime())
      cout << "Access Time       : " << _response.GetAccessTime()->GetRFC1123() << endl;

      if (_response.GetModificationTime())
      cout << "Modification Time : " << _response.GetModificationTime()->GetRFC1123() << endl;

      cout << "Content-type      : " << _response.GetContentType() << endl;
      
      if (_response._transfer_encoding.length())
      cout << "Transfer-encoding : " << _response._transfer_encoding << endl;
      
   }   

   // Check if persistent connection are possible
   CheckPersistentConnection(_response.GetVersion());

   if (debug > 4)
   	 cout << "Persistent connection: "
	    << (_persistent_connection_possible ? "would be accepted" : "not accepted")
	    << endl;

   DocumentStatus = GetDocumentStatus(_response);

   // We read the body only if the document has been found
   if (DocumentStatus != Document_ok)
   {
      ShouldTheBodyBeRead=false;
   }

   // For now a chunked response MUST BE retrieved   
   if (strcmp (_response._transfer_encoding, "chunked") == 0)
   {
      // Change the controller of the body reading
      SetBodyReadingController(&ReadChunkedBody);
   }

   // If "ShouldTheBodyBeRead" is set to true and	    
   // If the document is parsable, we can read the body
   // otherwise it is not worthwhile      

   if (ShouldTheBodyBeRead)
   {
      if ( debug > 4 )
         cout << "Reading the body of the response" << endl;

      // We use a int (HtHTTP::*)() function pointer
      if ( (this->*_readbody)() == -1 )
      {
         // The connection probably fell down !?!
         if ( debug > 4 )
            cout << "Connection fell down ... let's close it" << endl;
	    
         CloseConnection();	// Let's close the connection which is down now
	    
         // Return that the connection has fallen down during the request
         return Document_connection_down;
      }

      if ( debug > 6 )
         cout << "Contents:" << endl << _response.GetContents();

   }
   else if ( debug > 4 )
         cout << "Body not retrieved" << endl;


   // Close the connection (if there's no persistent connection)

   if( ! isPersistentConnectionUp() )
   {
      if ( debug > 4 )
         cout << "Connection closed (No persistent connection)" << endl;
         
      CloseConnection();
   }
   else
	 if ( debug > 4 )
	    cout << "Connection stays up ... (Persistent connection)" << endl;


   // Check the doc_status and return a value

   return FinishRequest(DocumentStatus);
   
}



HtHTTP::ConnectionStatus HtHTTP::EstablishConnection()
{

   int result;
   
   // Open the connection
   
   result=OpenConnection();
   
   if (!result)
   	 return Connection_open_failed; // Connection failed
   else if(debug > 4)
   	    if (result == -1)
	       cout << "Connection already open. No need to re-open." << endl;
	    else
	       cout << "Open of the connection ok" << endl;


   if(result==1) // New connection open
   {

      // Assign the remote host to the connection
      if ( !AssignConnectionServer() )
      	 return Connection_no_server;   	 
	 else if (debug > 4)
	       cout << "\tAssigned the remote host " << _url.host() << endl;
   
      // Assign the port of the remote host
      if ( !AssignConnectionPort() )
      	 return Connection_no_port;   	 
	 else if (debug > 4)
	       cout << "\tAssigned the port " << _url.port() << endl;
   }

   // Connect
   if (! (result = Connect()))
   	 return Connection_failed;
   else if (result == -1) return Connection_already_up; // Persistent
	    else return Connection_ok; // New connection
   
}



// Set the string of the HTTP message request

void HtHTTP::SetRequestCommand(String &cmd)
{

   // Initialize it

   cmd << _url.path() << " HTTP/1.1\r\n";

   // Insert the "virtual" host to which ask the document
   
   cmd << "Host: " << _url.host() << "\r\n";

	 
   // Insert the User Agent
   
   if (_user_agent.length()) cmd << "User-Agent: " << _user_agent << "\r\n";
   

   // Referer
   if (_referer.get() && strlen(_referer.get()))
     cmd << "Referer: " << _referer.get() << "\r\n";


   // Authentication
   if (_credentials.length())
     cmd << "Authorization: Basic " << _credentials << "\r\n";

   
   // A date has been passed to check if the server one is newer than
   // the one we already own.

   if(_modification_time)
   	 cmd << "If-Modified-Since: " << _modification_time->GetRFC1123() << "\r\n";

   // Let's close the command
   
   cmd << "\r\n";
   
}




//*****************************************************************************
// int HtHTTP::ParseHeader()
//   Parse the header of the document
//
int HtHTTP::ParseHeader()
{
    String	line;
    int		inHeader = 1;

    if (_response._modification_time)
    {
	delete _response._modification_time;
	_response._modification_time=NULL;
    }
    while (inHeader)
    {
   
      if(! _connection.read_line(line, "\n"))
         return -1;  // Connection down
	 
      _bytes_read+=line.length();
      line.chop('\r');
      
      if (line.length() == 0)
         inHeader = 0;
      else
      {
         // Found a not-empty line
	    
         if (debug > 3)
            cout << "Header line: " << line << endl;
	    
         // Status - Line check
         char	*token = line.get();

         while (*token && !isspace(*token))
            token++;
            
         while (*token && isspace(*token))
            token++;
	    
         if(!strncmp(line, "HTTP/", 5))
         {
            // Here is the status-line

            // store the HTTP version returned by the server
            _response._version = strtok(line, " ");

            // Store the status code
            _response._status_code = atoi(strtok(0, " "));
			
            // Store the reason phrase
            _response._reason_phrase = strtok(0, "\n");

         }
         else if( ! mystrncasecmp(line, "last-modified:", 14))
         {
            // Modification date sent by the server
		  
            // Set the response modification time
            token = strtok(token, "\n\t");

            if (token && *token)
               _response._modification_time = NewDate(token);

         }
         else if( ! mystrncasecmp(line, "date:", 5))
         {
            // Access date time sent by the server
		  
            // Set the response access time
            token = strtok(token, "\n\t");

            if (token && *token)
               _response._access_time = NewDate(token);

         }
         else if( ! mystrncasecmp(line, "content-type:", 13))
         {
            // Content - type
		  
            token = strtok(token, "\n\t");

            if (token && *token)
               _response._content_type = token;

         }
         else if( ! mystrncasecmp(line, "content-length:", 15))
         {
            // Content - length
		  
            token = strtok(token, "\n\t");

            if (token && *token)
               _response._content_length = atoi(token);

         }
         else if( ! mystrncasecmp(line, "transfer-encoding:", 18))
         {
            // Transfer-encoding
		  
            token = strtok(token, "\n\t");

            if (token && *token)
               _response._transfer_encoding = token;

         }
         else if( ! mystrncasecmp(line, "location:", 9))
         {
            // Found a location directive - redirect in act
		  
            token = strtok(token, "\n\t");

            if (token && *token)
               _response._location = token;

         }
         else
         {
            // Discarded
			
            if (debug > 3)
               cout << "Discarded header line: " << line << endl;
         }
      }
    }

    if (_response._modification_time == NULL && modification_time_is_now)
    {
         //Set the modification time
	_response._modification_time = new HtDateTime;
    }
    
    return 1;
   
}



// Create a new date time object containing the date specified in a string

HtDateTime *HtHTTP::NewDate(const char *datestring)
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

HtHTTP::DateFormat HtHTTP::RecognizeDateFormat (const char *datestring)
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


// Check for a document to be parsable
// It all depends on the content-type directive returned by the server

bool HtHTTP::isParsable(const char *content_type)
{

   // Here I can decide what kind of document I can parse
   // text/html -> HTML, text/* -> plaintext
   // and the rest are determined by the external_parser settings

   if( ! mystrncasecmp ("text/", content_type, 5)
       || ! mystrncasecmp ("application/pdf", content_type, 15) )
       return true;
       
   // External function that checks if a document is parsable or not.
   // CanBeParsed should point to a function that returns an int value,
   // given a char * containing the content-type.
   
   if (CanBeParsed && (*CanBeParsed)( (char *) content_type) )
      return true;

   return false;
   	 
}


// Check for a possibile persistent connection
// on the return message's HTTP version basis

void HtHTTP::CheckPersistentConnection(const char *version)
{
   
   if( ! mystrncasecmp ("HTTP/1.1", version, 8))
   	 _persistent_connection_possible=true;
   else
      _persistent_connection_possible=false;
   	 
}


HtHTTP::DocStatus HtHTTP::FinishRequest (HtHTTP::DocStatus ds)
{

   int seconds;
   
   // Set the finish time
   _end_time.SettoNow();

   // Let's add the number of seconds needed by the request
   seconds=HtDateTime::GetDiff(_end_time, _start_time);
   
   _tot_seconds += seconds;
   _tot_requests ++;
   _tot_bytes += _bytes_read;
   
   if (debug > 2)
      cout << "Request time: " << seconds << " secs" << endl; 

   return ds;
   
}


HtHTTP::DocStatus HtHTTP::GetDocumentStatus(HtHTTP_Response &r)
{ 

   // Let's give a look at the return status code

   HtHTTP::DocStatus returnStatus=Document_not_found;
   int statuscode;

   statuscode=r.GetStatusCode();

   if(statuscode==200)
   { 
	    returnStatus = Document_ok;   // OK

   	    // Is it parsable?
	    
   	    if (! isParsable (r.GetContentType()) )
   	 	    returnStatus=Document_not_parsable;
   }
   else if(statuscode > 200 && statuscode < 300)
	    returnStatus = Document_ok;      	   	 // Successful 2xx
   else if(statuscode==304)
	    returnStatus = Document_not_changed;   	 // Not modified
   else if(statuscode > 300 && statuscode < 400)
	    returnStatus = Document_redirect;      	 // Redirection 3xx
   else if(statuscode==401)
	    returnStatus = Document_not_authorized;   // Unauthorized
   
   // Exit the function
   return returnStatus;
	    
}


void HtHTTP::SetCredentials (String s)
{
  // Overload default Transport method to take care of Basic HTTP auth.
    static char	tbl[64] =
    {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/'
    };
    _credentials = 0;
    char	*p;
    int		n = s.length();
    int		ch;

    for (p = s.get(); n > 2; n -= 3, p += 3)
    {
	ch = *p >> 2;
	_credentials << tbl[ch & 077];
	ch = ((*p << 4) & 060) | ((p[1] >> 4) & 017);
	_credentials << tbl[ch & 077];
	ch = ((p[1] << 2) & 074) | ((p[2] >> 6) & 03);
	_credentials << tbl[ch & 077];
	ch = p[2] & 077;
	_credentials << tbl[ch & 077];
    }

    if (n != 0)
    {
	char c1 = *p;
	char c2 = n == 1 ? 0 : p[1];

	ch = c1 >> 2;
	_credentials << tbl[ch & 077];

	ch = ((c1 << 4) & 060) | ((c2 >> 4) & 017);
	_credentials << tbl[ch & 077];

	if (n == 1)
	    _credentials << '=';
	else
        {
	    ch = (c2 << 2) & 074;
	    _credentials << tbl[ch & 077];
        }
	_credentials << '=';
    }
}


int HtHTTP::ReadBody()
{

    _response._contents = 0;	// Initialize the string
    
    char	docBuffer[8192];
    int		bytesRead = 0;
    int		bytesToGo = _response._content_length;

    if (bytesToGo < 0 || bytesToGo > _max_document_size)
        bytesToGo = _max_document_size;


    while (bytesToGo > 0)
    {
        int len = bytesToGo< (int)sizeof(docBuffer) ? bytesToGo : (int)sizeof(docBuffer);
        bytesRead = _connection.read(docBuffer, len);
        if (bytesRead <= 0)
            break;

	_response._contents.append(docBuffer, bytesRead);

	bytesToGo -= bytesRead;
	
	_bytes_read+=bytesRead;

    }

    // Set document length
    _response._document_length = _response._contents.length();

   return bytesRead;
   
}


int HtHTTP::ReadChunkedBody()
{
   // Chunked Transfer decoding
   // as shown in the RFC2616 (HTTP/1.1) - 19.4.6
    
   int            length = 0;  // initialize the length
   unsigned int   chunk_size;
   String         ChunkHeader = 0;
   char           buffer[8192];
   
   _response._contents = 0;	// Initialize the string

   // Read chunk-size and CRLF
   _connection.read_line(ChunkHeader);
   sscanf ((char *)ChunkHeader, "%x", &chunk_size);

      cout << "Initial chunk-size: " << chunk_size << endl;

   while (chunk_size > 0)
   {
      // Read Chunk data
      if (_connection.read(buffer, chunk_size) == -1)
         return -1;

      // Read CRLF - to be ignored
      _connection.read_line(ChunkHeader);

      // Append the chunk-data to the contents of the response
      _response._contents << buffer;
                  
      length+=chunk_size;

      // Read chunk-size and CRLF
      _connection.read_line(ChunkHeader);
      sscanf ((char *)ChunkHeader, "%x", &chunk_size);

      cout << "Chunk-size: " << chunk_size << endl;
   }
   
   ChunkHeader = 0;
   
   // Ignoring next part of the body - the TRAILER
   // (it contains further headers - not implemented)

   // I think we must add some code because it doesn't recognize the end
   // of stream or something similar. It waits for the timeout and the connection
   // falls down ...

   _response._content_length = length;

   return length;

}


