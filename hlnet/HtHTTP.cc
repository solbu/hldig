//
// HtHTTP.cc
//
// HtHTTP: Interface classes for HTTP messaging
//
// Including:
//    -  Generic class
//    -  Response message class
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: HtHTTP.cc,v 1.27 2004/05/28 13:15:23 lha Exp $ 
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "lib.h"
#include "Transport.h"
#include "HtHTTP.h"

#include <signal.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>              // for sscanf

// for setw()
#ifdef HAVE_STD
#include <iomanip>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif
#else
#include <iomanip.h>
#endif /* HAVE_STD */

#if 1
typedef void (*SIGNAL_HANDLER) (...);
#else
typedef SIG_PF SIGNAL_HANDLER;
#endif

   // User Agent
String
  HtHTTP::_user_agent = 0;

   // Stats information
int
  HtHTTP::_tot_seconds = 0;
int
  HtHTTP::_tot_requests = 0;
int
  HtHTTP::_tot_bytes = 0;

   // flag that manage the option of 'HEAD' before 'GET'
bool
  HtHTTP::_head_before_get = true;

   // Handler of the CanParse function

int (*HtHTTP::CanBeParsed) (char *) = 0;

   // Cookies jar
HtCookieJar *
  HtHTTP::_cookie_jar = 0;      // Set to 0 by default

///////
   //    HtHTTP_Response class
   //
   //    Response message sent by the remote HTTP server
///////


// Construction

HtHTTP_Response::HtHTTP_Response ():_version (0),
_transfer_encoding (0),
_server (0), _hdrconnection (0), _content_language (0)
{
}


// Destruction

HtHTTP_Response::~HtHTTP_Response ()
{
}


void
HtHTTP_Response::Reset ()
{

  // Call the base class method in order to reset
  // the base class attributes

  Transport_Response::Reset ();

  // Initialize the version, transfer-encoding, location and server strings
  _version.trunc ();
  _transfer_encoding.trunc ();
  _hdrconnection.trunc ();
  _server.trunc ();
  _content_language.trunc ();

}




///////
   //    HtHTTP generic class
   //
   //    
///////


// Construction

HtHTTP::HtHTTP (Connection & connection):Transport (&connection), _Method (Method_GET),
                                                // Default Method Request
  _bytes_read (0),
_accept_language (0),
_persistent_connection_allowed (true),
_persistent_connection_possible (false), _send_cookies (true)
{
}

// Destruction

HtHTTP::~HtHTTP ()
{
}


///////
   //    Manages the requesting process
///////

Transport::DocStatus HtHTTP::Request ()
{

  DocStatus
    result = Document_ok;

///////
  //    We make a double request (HEAD and, maybe, GET)
  //    Depending on the
///////

  if (HeadBeforeGet () &&       // Option value to true
      _Method == Method_GET)    // Initial request method is GET
  {

    if (debug > 3)
      cout << "  Making a HEAD call before the GET" << endl;

    _Method = Method_HEAD;

    result = HTTPRequest ();

    _Method = Method_GET;
  }

  if (result == Document_ok)
    result = HTTPRequest ();

  if (result == Document_no_header && isPersistentConnectionAllowed ())
  {

    // Sometimes, the parsing phase of the header of the response
    // that the server gives us back, fails and a <no header>
    // error is raised. This happens with HTTP/1.1 persistent
    // connections, usually because the previous response stream
    // has not yet been flushed, so the buffer still contains
    // data regarding the last document retrieved. That sucks alot!
    // The only thing to do is to lose persistent connections benefits
    // for this document, so close the connection and 'GET' it again.

    CloseConnection ();         // Close a previous connection

    if (debug > 0)
      cout << "! Impossible to get the HTTP header line." << endl
        << "  Connection closed. Try to get it again." << endl;

    result = HTTPRequest ();    // Get the document again

  }

  return result;
}


///////
   //    Sends an HTTP 1/1 request
///////

Transport::DocStatus HtHTTP::HTTPRequest ()
{

  static
    Transport::DocStatus
    DocumentStatus;
  bool
    ShouldTheBodyBeRead = true;

  SetBodyReadingController (&HtHTTP::ReadBody);

  // Reset the response
  _response.Reset ();

  // Flush the connection
  FlushConnection ();

  _bytes_read = 0;

  if (debug > 4)
    cout << "Try to get through to host "
      << _url.host () << " (port " << _url.port () << ")" << endl;

  ConnectionStatus
    result;

  // Assign the timeout
  AssignConnectionTimeOut ();

  // Assign number of retries
  AssignConnectionRetries ();

  // Assign connection wait time
  AssignConnectionWaitTime ();

  // Start the timer
  _start_time.SettoNow ();

  result = EstablishConnection ();

  if (result != Connection_ok && result != Connection_already_up)
  {

    switch (result)
    {
      // Open failed

    case Connection_open_failed:
      if (debug > 1)
        cout << "Unable to open the connection with host: "
          << _url.host () << " (port " << _url.port () << ")" << endl;
      CloseConnection ();
      return FinishRequest (Document_no_connection);
      break;

      // Server not reached
    case Connection_no_server:
      if (debug > 1)
        cout << "Unable to find the host: "
          << _url.host () << " (port " << _url.port () << ")" << endl;
      CloseConnection ();
      return FinishRequest (Document_no_host);
      break;

      // Port not reached
    case Connection_no_port:
      if (debug > 1)
        cout << "Unable to connect with the port " << _url.port ()
          << " of the host: " << _url.host () << endl;
      CloseConnection ();
      return FinishRequest (Document_no_port);
      break;

      // Connection failed
    case Connection_failed:
      if (debug > 1)
        cout << "Unable to establish the connection with host: "
          << _url.host () << " (port " << _url.port () << ")" << endl;
      CloseConnection ();
      return FinishRequest (Document_no_connection);
      break;

      // Other reason
    default:
      if (debug > 1)
        cout << "connection failed with unexpected result: result = "
          << (int) result << ", "
          << _url.host () << " (port " << _url.port () << ")" << endl;
      CloseConnection ();
      return FinishRequest (Document_other_error);
      break;
    }

    return FinishRequest (Document_other_error);

  }

  // Visual comments about the result of the connection
  if (debug > 5)
    switch (result)
    {
    case Connection_already_up:
      cout << "Taking advantage of persistent connections" << endl;
      break;
    case Connection_ok:
      cout << "New connection open successfully" << endl;
      break;
    default:
      cout << "Unexptected value: " << (int) result << endl;
      break;
    }

  String
    command;

  switch (_Method)
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

  SetRequestCommand (command);

  if (debug > 6)
    cout << "Request\n" << command;

  // Writes the command
  ConnectionWrite (command);

  // Parse the header
  if (ParseHeader () == -1)     // Connection down
  {
    // The connection probably fell down !?!
    if (debug > 4)
      cout << setw (5) << Transport::GetTotOpen () << " - "
        << "Connection fell down ... let's close it" << endl;

    CloseConnection ();         // Let's close the connection which is down now

    // Return that the connection has fallen down during the request
    return FinishRequest (Document_connection_down);
  }


  if (_response._status_code == -1)
  {
    // Unable to retrieve the status line

    if (debug > 4)
      cout << "Unable to retrieve or parse the status line" << endl;

    return FinishRequest (Document_no_header);
  }


  if (debug > 3)
  {

    cout << "Retrieving document " << _url.path () << " on host: "
      << _url.host () << ":" << _url.port () << endl;

    cout << "Http version      : " << _response._version << endl;
    cout << "Server            : " << _response._version << endl;
    cout << "Status Code       : " << _response._status_code << endl;
    cout << "Reason            : " << _response._reason_phrase << endl;

    if (_response.GetAccessTime ())
      cout << "Access Time       : " << _response.GetAccessTime ()->
        GetRFC1123 () << endl;

    if (_response.GetModificationTime ())
      cout << "Modification Time : " << _response.GetModificationTime ()->
        GetRFC1123 () << endl;

    cout << "Content-type      : " << _response.GetContentType () << endl;

    if (_response._transfer_encoding.length ())
      cout << "Transfer-encoding : " << _response._transfer_encoding << endl;

    if (_response._content_language.length ())
      cout << "Content-Language : " << _response._content_language << endl;

    if (_response._hdrconnection.length ())
      cout << "Connection        : " << _response._hdrconnection << endl;

  }

  // Check if persistent connection are possible
  CheckPersistentConnection (_response);

  if (debug > 4)
    cout << "Persistent connection: "
      << (_persistent_connection_possible ? "would be accepted" :
          "not accepted") << endl;

  DocumentStatus = GetDocumentStatus (_response);

  // We read the body only if the document has been found
  if (DocumentStatus != Document_ok)
  {
    ShouldTheBodyBeRead = false;
  }

  // For now a chunked response MUST BE retrieved
  if (mystrncasecmp ((char *) _response._transfer_encoding, "chunked", 7) ==
      0)
  {
    // Change the controller of the body reading
    SetBodyReadingController (&HtHTTP::ReadChunkedBody);
  }

  // If "ShouldTheBodyBeRead" is set to true and  
  // If the document is parsable, we can read the body
  // otherwise it is not worthwhile

  if (ShouldTheBodyBeRead)
  {
    if (debug > 4)
      cout << "Reading the body of the response" << endl;

    // We use a int (HtHTTP::*)() function pointer
    if ((this->*_readbody) () == -1)
    {
      // The connection probably fell down !?!
      if (debug > 4)
        cout << setw (5) << Transport::GetTotOpen () << " - "
          << "Connection fell down ... let's close it" << endl;

      CloseConnection ();       // Let's close the connection which is down now

      // Return that the connection has fallen down during the request
      return FinishRequest (Document_connection_down);
    }

    if (debug > 6)
      cout << "Contents:" << endl << _response.GetContents ();

    // Check if the stream returned by the server has not been completely read

    if (_response._document_length != _response._content_length &&
        _response._document_length == _max_document_size)
    {
      // Max document size reached

      if (debug > 4)
        cout << "Max document size (" << GetRequestMaxDocumentSize ()
          << ") reached ";

      if (isPersistentConnectionUp ())
      {
        // Only have to close persistent connection when we didn't read
        // all the input. For now, we always read all chunked input...
        if (mystrncasecmp
            ((char *) _response._transfer_encoding, "chunked", 7) != 0)
        {
          if (debug > 4)
            cout << "- connection closed. ";

          CloseConnection ();
        }
      }

      if (debug > 4)
        cout << endl;
    }

    // Make sure our content-length makes sense, if none given...
    if (_response._content_length < _response._document_length)
      _response._content_length = _response._document_length;

  }
  else if (debug > 4)
    cout << "Body not retrieved" << endl;


  // Close the connection (if there's no persistent connection)

  if (!isPersistentConnectionUp ())
  {
    if (debug > 4)
      cout << setw (5) << Transport::GetTotOpen () << " - "
        << "Connection closed (No persistent connection)" << endl;

    CloseConnection ();
  }
  else
  {
    // Persistent connection is active

    // If the document is not parsable and we asked for it with a 'GET'
    // method, the stream's not been completely read.

    if (DocumentStatus == Document_not_parsable && _Method == Method_GET)
    {
      // We have to close the connection.
      if (debug > 4)
        cout << "Connection must be closed (stream not completely read)"
          << endl;

      CloseConnection ();

    }
    else if (debug > 4)
      cout << "Connection stays up ... (Persistent connection)" << endl;
  }


  // Check the doc_status and return a value

  return FinishRequest (DocumentStatus);

}



HtHTTP::ConnectionStatus HtHTTP::EstablishConnection ()
{

  int
    result;

  // Open the connection
  result = OpenConnection ();

  if (!result)
    return Connection_open_failed;      // Connection failed
  else if (debug > 4)
  {
    cout << setw (5) << Transport::GetTotOpen () << " - ";

    if (result == -1)
      cout << "Connection already open. No need to re-open." << endl;
    else
      cout << "Open of the connection ok" << endl;
  }


  if (result == 1)              // New connection open
  {

    // Assign the remote host to the connection
    if (!AssignConnectionServer ())
      return Connection_no_server;
    else if (debug > 4)
      cout << "\tAssigned the remote host " << _url.host () << endl;

    // Assign the port of the remote host
    if (!AssignConnectionPort ())
      return Connection_no_port;
    else if (debug > 4)
      cout << "\tAssigned the port " << _url.port () << endl;
  }

  // Connect
  if (!(result = Connect ()))
    return Connection_failed;
  else if (result == -1)
    return Connection_already_up;       // Persistent
  else
    return Connection_ok;       // New connection

}



// Set the string of the HTTP message request

void
HtHTTP::SetRequestCommand (String & cmd)
{

  // Initialize it

  if (_useproxy)
  {
    cmd << _url.get () << " HTTP/1.1\r\n";
  }
  else
    cmd << _url.path () << " HTTP/1.1\r\n";

  // Insert the "virtual" host to which ask the document

  cmd << "Host: " << _url.host ();
  if (_url.port () != 0 && _url.port () != _url.DefaultPort ())
    cmd << ":" << _url.port ();
  cmd << "\r\n";


  // Insert the User Agent

  if (_user_agent.length ())
    cmd << "User-Agent: " << _user_agent << "\r\n";


  // Referer
  if (_referer.get ().length ())
    cmd << "Referer: " << _referer.get () << "\r\n";

  // Accept-Language
  if (_accept_language.length ())
    cmd << "Accept-language: " << _accept_language << "\r\n";

  // Authentication
  if (_credentials.length ())
    cmd << "Authorization: Basic " << _credentials << "\r\n";

  // Proxy Authentication
  if (_useproxy && _proxy_credentials.length ())
    cmd << "Proxy-Authorization: Basic " << _proxy_credentials << "\r\n";

  // Accept-Encoding: waiting to handle the gzip and compress formats, we
  // just send an empty header which, according to the HTTP 1/1 standard,
  // should let the server know that we only accept the 'identity' case
  // (no encoding of the document)
  cmd << "Accept-Encoding: \r\n";

  // A date has been passed to check if the server one is newer than
  // the one we already own.

  if (_modification_time && *_modification_time > 0)
  {
    _modification_time->ToGMTime ();
    cmd << "If-Modified-Since: " << _modification_time->
      GetRFC1123 () << "\r\n";
  }

///////
  //    Cookies! Let's go eat them! ;-)
///////

  // The method returns all the valid cookies and writes them
  // directly into the request string, as a list of headers
  if (_send_cookies && _cookie_jar)
    _cookie_jar->SetHTTPRequest_CookiesString (_url, cmd);


  // Let's close the command
  cmd << "\r\n";

}




//*****************************************************************************
// int HtHTTP::ParseHeader()
//   Parse the header of the document
//
int
HtHTTP::ParseHeader ()
{
  String line = 0;
  int inHeader = 1;

  if (_response._modification_time)
  {
    delete _response._modification_time;
    _response._modification_time = 0;
  }
  while (inHeader)
  {

    line.trunc ();

    if (!_connection->Read_Line (line, "\n"))
      return -1;                // Connection down

    _bytes_read += line.length ();
    line.chop ('\r');

    if (line.length () == 0)
      inHeader = 0;
    else
    {
      // Found a not-empty line

      if (debug > 2)
        cout << "Header line: " << line << endl;

      // Status - Line check
      char *token = line.get ();

      while (*token && !isspace (*token) && *token != ':')
        ++token;

      while (*token && (isspace (*token) || *token == ':'))
        ++token;

      if (!strncmp ((char *) line, "HTTP/", 5))
      {
        // Here is the status-line

        // store the HTTP version returned by the server
        _response._version = strtok (line, " ");

        // Store the status code
        _response._status_code = atoi (strtok (0, " "));

        // Store the reason phrase
        _response._reason_phrase = strtok (0, "\n");

      }
      else if (!mystrncasecmp ((char *) line, "server:", 7))
      {
        // Server info

        // Set the server info
        token = strtok (token, "\n\t");

        if (token && *token)
          _response._server = token;

      }
      else if (!mystrncasecmp ((char *) line, "last-modified:", 14))
      {
        // Modification date sent by the server

        // Set the response modification time
        token = strtok (token, "\n\t");

        if (token && *token)
          _response._modification_time = NewDate (token);

      }
      else if (!mystrncasecmp ((char *) line, "date:", 5))
      {
        // Access date time sent by the server

        // Set the response access time
        token = strtok (token, "\n\t");

        if (token && *token)
          _response._access_time = NewDate (token);

      }
      else if (!mystrncasecmp ((char *) line, "content-type:", 13))
      {
        // Content - type

        token = strtok (token, "\n\t");

        if (token && *token)
          _response._content_type = token;

      }
      else if (!mystrncasecmp ((char *) line, "content-length:", 15))
      {
        // Content - length

        token = strtok (token, "\n\t");

        if (token && *token)
          _response._content_length = atoi (token);

      }
      else if (!mystrncasecmp ((char *) line, "transfer-encoding:", 18))
      {
        // Transfer-encoding

        token = strtok (token, "\n\t");

        if (token && *token)
          _response._transfer_encoding = token;

      }
      else if (!mystrncasecmp ((char *) line, "location:", 9))
      {
        // Found a location directive - redirect in act

        token = strtok (token, "\n\t");

        if (token && *token)
          _response._location = token;

      }
      else if (!mystrncasecmp ((char *) line, "connection:", 11))
      {
        // Ooops ... found a Connection clause

        token = strtok (token, "\n\t");

        if (token && *token)
          _response._hdrconnection = token;

      }
      else if (!mystrncasecmp ((char *) line, "content-language:", 17))
      {
        // Found a content-language directive

        token = strtok (token, "\n\t");

        if (token && *token)
          _response._content_language = token;

      }
      else if (!mystrncasecmp ((char *) line, "set-cookie:", 11))
      {
        // Found a cookie

        // Are cookies enabled?
        if (_send_cookies && _cookie_jar)
        {
          token = strtok (token, "\n\t");

          if (token && *token)
          {
            // Insert the cookie into the jar
            _cookie_jar->AddCookie (token, _url);
          }
        }

      }
      else
      {
        // Discarded

        if (debug > 3)
          cout << "Discarded header line: " << line << endl;
      }
    }
  }

  if (_response._modification_time == 0)
  {
    if (debug > 3)
      cout << "No modification time returned: assuming now" << endl;

    //Set the modification time
    _response._modification_time = new HtDateTime;
    _response._modification_time->ToGMTime ();  // Set to GM time

  }

  return 1;

}


// Check for a document to be parsable
// It all depends on the content-type directive returned by the server

bool
HtHTTP::isParsable (const char *content_type)
{

  // Here I can decide what kind of document I can parse
  // depending on the value of Transport:_default_parser_content_type
  // and the rest are determined by the external_parser settings

  if (!mystrncasecmp (_default_parser_content_type.get (), content_type,
                      _default_parser_content_type.length ()))
    return true;

  // External function that checks if a document is parsable or not.
  // CanBeParsed should point to a function that returns an int value,
  // given a char * containing the content-type.

  if (CanBeParsed && (*CanBeParsed) ((char *) content_type))
    return true;

  return false;

}


// Check for a possibile persistent connection
// on the return message's HTTP version basis

void
HtHTTP::CheckPersistentConnection (HtHTTP_Response & response)
{

  const char *version = response.GetVersion ();

  if (!mystrncasecmp ("HTTP/1.1", version, 8))
  {
    const char *connection = response.GetConnectionInfo ();

    if (!mystrncasecmp ("close", connection, 5))
      _persistent_connection_possible = false;  // Server wants to close
    else
      _persistent_connection_possible = true;

  }
  else
    _persistent_connection_possible = false;

}


HtHTTP::DocStatus HtHTTP::FinishRequest (HtHTTP::DocStatus ds)
{

  int
    seconds;

  // Set the finish time
  _end_time.SettoNow ();

  // Let's add the number of seconds needed by the request
  seconds = HtDateTime::GetDiff (_end_time, _start_time);

  _tot_seconds += seconds;
  _tot_requests++;
  _tot_bytes += _bytes_read;

  if (debug > 2)
    cout << "Request time: " << seconds << " secs" << endl;

  return ds;

}


HtHTTP::DocStatus HtHTTP::GetDocumentStatus (HtHTTP_Response & r)
{

  // Let's give a look at the return status code

  HtHTTP::DocStatus returnStatus = Document_not_found;
  int
    statuscode;

  statuscode = r.GetStatusCode ();

  if (statuscode == 200)
  {
    returnStatus = Document_ok; // OK

    // Is it parsable?

    if (!isParsable ((const char *) r.GetContentType ()))
      returnStatus = Document_not_parsable;
  }
  else if (statuscode > 200 && statuscode < 300)
    returnStatus = Document_ok; // Successful 2xx
  else if (statuscode == 304)
    returnStatus = Document_not_changed;        // Not modified
  else if (statuscode > 300 && statuscode < 400)
    returnStatus = Document_redirect;   // Redirection 3xx
  else if (statuscode == 401)
    returnStatus = Document_not_authorized;     // Unauthorized

  // Exit the function
  return returnStatus;

}

void
HtHTTP::SetCredentials (const String & s)
{
  Transport::SetHTTPBasicAccessAuthorizationString (_credentials, s);
}


void
HtHTTP::SetProxyCredentials (const String & s)
{
  Transport::SetHTTPBasicAccessAuthorizationString (_proxy_credentials, s);
}

int
HtHTTP::ReadBody ()
{

  _response._contents = 0;      // Initialize the string

  char docBuffer[8192];
  int bytesRead = 0;
  int bytesToGo = _response._content_length;

  if (bytesToGo < 0 || bytesToGo > _max_document_size)
    bytesToGo = _max_document_size;

  while (bytesToGo > 0)
  {
    int len =
      bytesToGo <
      (int) sizeof (docBuffer) ? bytesToGo : (int) sizeof (docBuffer);
    bytesRead = _connection->Read (docBuffer, len);
    if (bytesRead <= 0)
      break;

    _response._contents.append (docBuffer, bytesRead);

    bytesToGo -= bytesRead;

    _bytes_read += bytesRead;

  }

  // Set document length
  _response._document_length = _response._contents.length ();

  return bytesRead;

}


int
HtHTTP::ReadChunkedBody ()
{
  // Chunked Transfer decoding
  // as shown in the RFC2616 (HTTP/1.1) - 19.4.6

#define  BSIZE  8192

  int length = 0;               // initialize the length
  unsigned int chunk_size;
  String ChunkHeader = 0;
  char buffer[BSIZE + 1];
  int chunk, rsize;

  _response._contents.trunc (); // Initialize the string

  // Read chunk-size and CRLF
  if (!_connection->Read_Line (ChunkHeader, "\r\n"))
    return -1;

  sscanf ((char *) ChunkHeader, "%x", &chunk_size);

  if (debug > 4)
    cout << "Initial chunk-size: " << chunk_size << endl;

  while (chunk_size > 0)
  {
    chunk = chunk_size;

    do
    {
      if (chunk > BSIZE)
      {
        rsize = BSIZE;
        if (debug > 4)
          cout << "Read chunk partial: left=" << chunk << endl;
      }
      else
      {
        rsize = chunk;
      }
      chunk -= rsize;

      // Read Chunk data
      if (_connection->Read (buffer, rsize) == -1)
        return -1;

      length += rsize;

      // Append the chunk-data to the contents of the response
      // ... but not more than _max_document_size...
      if (rsize > _max_document_size - _response._contents.length ())
        rsize = _max_document_size - _response._contents.length ();
      buffer[rsize] = 0;
      _response._contents.append (buffer, rsize);

    }
    while (chunk);

    //     if (_connection->Read(buffer, chunk_size) == -1)
    //       return -1;

    // Read CRLF - to be ignored
    if (!_connection->Read_Line (ChunkHeader, "\r\n"))
      return -1;

    // Read chunk-size and CRLF
    if (!_connection->Read_Line (ChunkHeader, "\r\n"))
      return -1;

    sscanf ((char *) ChunkHeader, "%x", &chunk_size);

    if (debug > 4)
      cout << "Chunk-size: " << chunk_size << endl;
  }

  ChunkHeader = 0;

  // Ignoring next part of the body - the TRAILER
  // (it contains further headers - not implemented)

  // Set content length
  _response._content_length = length;

  // Set document length
  _response._document_length = _response._contents.length ();

  return length;

}


///////
   //    Show the statistics
///////

ostream & HtHTTP::ShowStatistics (ostream & out)
{
  Transport::ShowStatistics (out);      // call the base class method

  out << " HTTP Requests             : " << GetTotRequests () << endl;
  out << " HTTP KBytes requested     : " << (double) GetTotBytes () /
    1024 << endl;
  out << " HTTP Average request time : " << GetAverageRequestTime () <<
    " secs" << endl;

  out << " HTTP Average speed        : " << GetAverageSpeed () / 1024
    << " KBytes/secs" << endl;

  return out;
}
