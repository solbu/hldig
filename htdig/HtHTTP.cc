//
// HtHTTP.cc
//
// Interface classes for HTTP messaging
//
// Including:
// 	 -  Generic class
// 	 -  Response message class
//


#include <signal.h>
#include <sys/types.h>
#include <ctype.h>
#include "lib.h"
#include "HtHTTP.h"
#include <iostream.h>
//#include "htcheck.h"

#include <unistd.h>  // for sleep

#if 1
typedef void (*SIGNAL_HANDLER) (...);
#else
typedef SIG_PF SIGNAL_HANDLER;
#endif


///////
   //    Initializing Static variables
///////

   // Timeout of the HTTP connection
   	 int HtHTTP::_timeout=DEFAULT_CONNECTION_TIMEOUT;

   // Maximum document size
   	 int HtHTTP::_max_document_size=DEFAULT_MAX_DOCUMENT_SIZE;

   // Debug level
   	 int HtHTTP::debug = 0;

   // User Agent
   	 String HtHTTP::_user_agent=0;

   // Stats information
   	 int HtHTTP::_tot_seconds = 0;
   	 int HtHTTP::_tot_requests = 0;
   	 int HtHTTP::_tot_bytes = 0;



///////
   //    HtHTTP_Response class
   //
   //    Response message sent by the remote HTTP server
///////


// Construction

HtHTTP_Response::HtHTTP_Response()
{
   // Initialize the pointers to the HtDateTime objs
   _modification_time=NULL;
   _access_time=NULL;

   // Set the content length and the return status code to negative values
   _content_length=-1;
   _status_code=-1;
   
   // Also set the document length, but to zero instead of -1
   _document_length=0;

   // Zeroes the contents
   _contents=0;
         
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

   // Set the content length and the return status code to negative values
   _content_length=-1;
   _status_code=-1;
   
   // Also set the document length, but to zero instead of -1
   _document_length=0;

   // Zeroes the contents, the version, and the reason phrase of the s.c.
   // Also the location string and content type string
   _version=0;
   _reason_phrase=0;
   _contents=0;
   _location=0;
   _content_type=0;
      
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
   _host=NULL;
   _persistent_connection_possible = false;
   _persistent_connection_allowed = true; // by default
   _server = " ";

   _bytes_read=0;

}

// Destruction

HtHTTP::~HtHTTP()
{
   // Close the connection that was still up

   if (CloseConnection() && debug > 3)
	    cout << "Closing previous connection with the remote host" << endl;

}



///////
   //    Sends a GET method request
///////

HtHTTP::DocStatus HtHTTP::Request(HtHTTP::HttpRequestMethod _Method)
{

   bool ShouldTheBodyBeRead = true;
   
   // Reset the response
   _response.Reset();

   _bytes_read=0;

   if( debug > 3)
   	 cout << "Try to get through to host "
			   << _server << " (port " << _port << ")" << endl;

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
			   << _server << " (port " << _port << ")" << endl;
			break;

   	       // Server not reached
	       case Connection_no_server:
		  	cout << "Unable to find the host: "
			   << _server << " (port " << _port << ")" << endl;
			break;
	    
   	       // Port not reached
	       case Connection_no_port:
		  	cout << "Unable to connect with the port " << _port
   	       	   << " of the host: " << _server << endl;
			break;
	    
   	       // Connection failed
	       case Connection_failed:
		  	cout << "Unable to establish the connection with host: "
			   << _server << " (port " << _port << ")" << endl;
			break;
	    }
	 
   	 return FinishRequest(Document_not_found);

   }

   // Visual comments about the result of the connection
   if (debug > 4)
   	 switch(result)
      {
	    case Connection_already_up:
   	       cout << "Taking advantage of persistent connections" << endl;
   	       break;
	    case Connection_ok:
   	       cout << "New connection open successfully" << endl;
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


   // Let's check if the host has been set before
   // If not, we assume that the host oh the HTTP request
   // is the same of the TCP connection (that is to say _server)
   
   if (!_host) _host=_server;

   SetRequestCommand(command);

   if (debug > 5)
      cout << "Request\n" << command;

   // Writes the command
   ConnectionWrite(command);

   // Assign the timeout
   AssignConnectionTimeOut();

   // Parse the header
   if (ParseHeader() == -1) // Connection down
   {
	    // The connection probably fell down !?!
   	 	 if ( debug > 3 )
	       cout << "Connection fell down ... let's close it" << endl;
	    
	    CloseConnection();	// Let's close the connection which is down now
	    
	    // Return that the connection has fallen down during the request
	    return Document_connection_down;
   }   

   if (_response._status_code == -1)
   {
   	 // Unable to retrieve the status line

   	 if ( debug > 3 )
	    cout << "Unable to retrieve or parse the status line" << endl;
	 
   	 return Document_no_header;
	 
   }   
      

   if (debug > 2)
   {

   	 cout << "Retrieving document " << _document << " on host: "
	       << _host << ":" << _port << endl;
		  
      cout << "Http version: " << _response._version << endl;
      cout << "Status Code: " << _response._status_code << endl;
      cout << "Reason: " << _response._reason_phrase << endl;

   	 if (_response.GetAccessTime())
         cout << "Access Time: " << _response.GetAccessTime()->GetRFC1123() << endl;

   	 if (_response.GetModificationTime())
         cout << "Modification Time: " << _response.GetModificationTime()->GetRFC1123() << endl;

      cout << "Content-type: " << _response.GetContentType() << endl;
   }   

   // Check if persistent connection are possible
   CheckPersistentConnection(_response.GetVersion());

   if (debug > 3)
   	 cout << "Persistent connection: "
	    << (_persistent_connection_possible ? "would be accepted" : "not accepted")
	    << endl;

   // If "ShouldTheBodyBeRead" is set to true and	    
   // If the document is parsable, we can read the body
   // otherwise it is not worthwhile      

   if (ShouldTheBodyBeRead && isParsable (_response.GetContentType()))
   	 if ( ReadBody() == -1 )
   	 {
	    // The connection probably fell down !?!
   	 	 if ( debug > 3 )
	       cout << "Connection fell down ... let's close it" << endl;
	    
	    CloseConnection();	// Let's close the connection which is down now
	    
	    // Return that the connection has fallen down during the request
	    return Document_connection_down;
	    
   	 }

   // Close the connection (if there's no persistent connection)

   if( ! isPersistentConnectionUp() )
      CloseConnection();
   else
	 if ( debug > 3 )
	    cout << "Connection stays up ... (Persistent connection)" << endl;


   // Check the doc_status and return a value

   return FinishRequest(GetDocumentStatus(_response));
   
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

      // Assign the remote host
      if (!AssignConnectionServer())
      	 return Connection_no_server;   	 
	 else if (debug > 4)
	       cout << "\tAssigned the remote host " << _server << endl;
   
      // Assign the port of the remote host
      if (!AssignConnectionPort())
      	 return Connection_no_port;   	 
	 else if (debug > 4)
	       cout << "\tAssigned the port " << _port << endl;
   }

   // Connect
   if (! (result = Connect()))
   	 return Connection_failed;
   else if (result == -1) return Connection_already_up; // Persistent
	    else return Connection_ok; // New connection
   
}


void HtHTTP::SetHttpConnection (char *server, int port)
{
   bool ischanged = false;

   // Checking the connection server   
   if(strcmp(server, _server))   	 // server is gonna change
   {
   	 SetConnectionServer(server);  // let's change the server
   	 ischanged=true;
   }

   // Checking the connection port
   if(port != _port)   	  	   	 // the port is gonna change
   {
   	 SetConnectionPort(port);   	 // let's change the port
   	 ischanged=true;
   }

   if (ischanged)
   {
   	 // Let's close any pendant connection with the old
	 // server / port pair
	 
	 CloseConnection();
   }

}



// Set the string of the HTTP message request

void HtHTTP::SetRequestCommand(String &cmd)
{

   // Initialize it

   cmd << _document << " HTTP/1.1\r\n";

   // Insert the "virtual" host to which ask the document
   
   if(_host)
   	 cmd << "Host: " << _host << "\r\n";

	 
   // Insert the User Agent
   
   if (_user_agent.length()) cmd << "User-Agent: " << _user_agent << "\r\n";
   

   // Referer ???
   
   // A date has been passed to check if the server one is newer than
   // the one we already own.

   if(_modification_time)
   	 cmd << "If-Modified-Since: " << _modification_time->GetRFC1123() << "\r\n";

   // Let's close the command
   
   cmd << "\r\n";
   
}


// Open the connection
// Returns
// 	 -      0 if failed
// 	 -     -1 if already open
// 	 -      1 if ok

int HtHTTP::OpenConnection()
{
   if(_connection.isopen()) return -1; // Already open

   // No open connection
   // Let's open a new one
   
   if(_connection.open() == NOTOK) return 0; // failed

   return 1;
}


// Assign the server to the connection

int HtHTTP::AssignConnectionServer()
{
   if (_connection.assign_server(_server) == NOTOK) return 0;
   
   return 1;
}

// Assign the remote server port to the connection

int HtHTTP::AssignConnectionPort()
{
   if (_connection.assign_port(_port) == NOTOK) return 0;
   
   return 1;
}


// Connect
// Returns
// 	 -      0 if failed
// 	 -     -1 if already connected
// 	 -      1 if ok

int HtHTTP::Connect()
{
   if (isConnected()) return -1; // Already connected
   if (_connection.connect(1) == NOTOK) return 0;  // Connection failed
   
   return 1;	// Connected
}


// Write a message

int HtHTTP::ConnectionWrite(char *cmd)
{
   _connection.write(cmd);
   
   return 1;
}


// Assign the timeout to the connection

int HtHTTP::AssignConnectionTimeOut()
{
   return _connection.timeout(_timeout);
}



// Close the connection
// Returns
// 	 -      0 if not open
// 	 -      1 if closed ok


int HtHTTP::CloseConnection()
{
   if(_connection.isopen())
   	 _connection.close(); 	// Close the connection
   else return 0;
   
   return 1;
}

// Is the connection up
bool HtHTTP::isConnected()
{
   return _connection.isconnected();
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
		  
                  strtok(line, " \t");
		  char    *token = strtok(0, "\n\t");
		  while (*token == ' ' || *token == '\t')
		    token++; // Strip off any whitespace...
		  
		  // Set the response modification time
		  if (token)
		    _response._modification_time = NewDate(token);

   	    }
	    else if( ! mystrncasecmp(line, "date:", 5))
   	    {
	       // Access date time sent by the server
		  
	          strtok(line, " \t"); // Let's position after the field name
		  char    *token = strtok(0, "\n\t");
		  while (*token == ' ' || *token == '\t')
		    token++; // Strip off any whitespace...
		  
		  // Set the response access time
		  if (token)
		    _response._access_time = NewDate(token);

   	    }
	    else if( ! mystrncasecmp(line, "content-type:", 13))
   	    {
	       // Content - type
		  
		  strtok(line, " \t"); // Let's position after the field name
		  char    *token = strtok(0, "\n\t");
		  while (*token == ' ' || *token == '\t')
		    token++; // Strip off any whitespace...

		  if (token)
		    _response._content_type = token;

   	    }
	    else if( ! mystrncasecmp(line, "content-length:", 15))
   	    {
	       // Content - length
		  
	          strtok(line, " \t"); // Let's position after the field name
		  char    *token = strtok(0, "\n\t");
		  while (*token == ' ' || *token == '\t')
		    token++; // Strip off any whitespace...

		  if (token)
		    _response._content_length = atoi(token);

   	    }
	    else if( ! mystrncasecmp(line, "location:", 9))
   	    {
	       // Found a location directive - redirect in act
		  
		  strtok(line, " \t"); // Let's position after the field name
		  char    *token = strtok(0, "\n\t");
		  while (*token == ' ' || *token == '\t')
		    token++; // Strip off any whitespace...

		  if (token)
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
    static int	modification_time_is_now =
			config.Boolean("modification_time_is_now");
    if (_response._modification_time == NULL && modification_time_is_now)
	_response._modification_time = new HtDateTime;
    
    return 1;
   
}



int HtHTTP::ReadBody()
{

    _response._contents = 0;	// Initialize the string
    
    char	docBuffer[8192];
    int		bytesRead;
    int		bytesToGo = _response._content_length;

    if (bytesToGo < 0 || bytesToGo > _max_document_size)
        bytesToGo = _max_document_size;

    while (bytesToGo > 0)
    {
        int len = bytesToGo<sizeof(docBuffer) ? bytesToGo : sizeof(docBuffer);
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
   }

   return dt;
   
}



// Recognize the possible date format sent by the server

HtHTTP::DateFormat HtHTTP::RecognizeDateFormat (const char *datestring)
{
   register char *s;
   
   if(s=strchr(datestring, ','))
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
   // I only need to parse the ones which return a "text/html"
   // content-type ... But if in future I wanna check and parse
   // other document types, I only have to add some code here
   // and add the ExternalParser handler ...
   
   if( ! mystrncasecmp ("text/html", content_type, 9))
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
