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
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: Transport.cc,v 1.12 2004/05/28 13:15:23 lha Exp $
//
//

#ifdef HAVE_CONFIG_H
#include "hlconfig.h"
#endif /* HAVE_CONFIG_H */

#include "Transport.h"

#ifdef HAVE_STD
#include <iomanip>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif
#else
#include <iomanip.h>
#endif /* HAVE_STD */

#include <ctype.h>

#define DEFAULT_CONNECTION_TIMEOUT 15

///////
   //    Static variables initialization
///////

   // Debug level
int
  Transport::debug = 0;

   // Default parser content-type string
String
  Transport::_default_parser_content_type = 0;

   // Statistics
int
  Transport::_tot_open = 0;
int
  Transport::_tot_close = 0;
int
  Transport::_tot_changes = 0;


///////
   //    Transport_Response class definition
///////

 ///////
    //    Class Constructor
 ///////

Transport_Response::Transport_Response ()
{

  // Initialize the pointers to the HtDateTime objs
  _modification_time = 0;
  _access_time = 0;

  // Set the content length and the return status code to negative values
  _content_length = -1;
  _status_code = -1;

  // Also set the document length, but to zero instead of -1
  _document_length = 0;

  // Zeroes the contents and the content-type
  _contents = 0;
  _content_type = 0;

  // Initialize the reason_phrase
  _reason_phrase = 0;

  // Initialize the location
  _location = 0;

}


 ///////
    //    Empty destructor
 ///////

Transport_Response::~Transport_Response ()
{

  // Free memory correctly

  if (_modification_time)
  {
    delete _modification_time;
    _modification_time = 0;
  }

  if (_access_time)
  {
    delete _access_time;
    _access_time = 0;
  }

}



void
Transport_Response::Reset ()
{
  // Reset all the field of the object

  // Check if an HtDateTime object exists, and delete it
  if (_modification_time)
  {
    delete _modification_time;
    _modification_time = 0;
  }

  if (_access_time)
  {
    delete _access_time;
    _access_time = 0;
  }

  // Set the content length to a negative value
  _content_length = -1;

  // Also set the document length, but to zero instead of -1
  _document_length = 0;

  // Zeroes the contents and content type strings
  _contents.trunc ();
  _content_type.trunc ();

  // Set the return status code to a negative value
  _status_code = -1;

  // Zeroes the reason phrase of the s.c.
  _reason_phrase.trunc ();

  // Reset the location
  _location.trunc ();

}



///////
   //    Transport class definition
///////

 ///////
    //    Constructor
 ///////

Transport::Transport (Connection * connection):_connection (connection),
_host (0), _ip_address (0), _port (-1), _timeout (DEFAULT_CONNECTION_TIMEOUT),
_retries (1), _wait_time (5),
_modification_time (0), _max_document_size (0),
_credentials (0), _useproxy (0), _proxy_credentials (0)
{
}


 ///////
    //    Destructor
 ///////

Transport::~Transport ()
{

  // Close the connection that was still up
  if (CloseConnection ())
    if (debug > 4)
      cout << setw (5) << GetTotOpen () << " - "
        << "Closing previous connection with the remote host" << endl;

  if (_connection)
    delete (_connection);

}

///////
   //    Show the statistics
///////

ostream & Transport::ShowStatistics (ostream & out)
{
  out << " Connections opened        : " << GetTotOpen () << endl;
  out << " Connections closed        : " << GetTotClose () << endl;
  out << " Changes of server         : " << GetTotServerChanges () << endl;

  return out;

}


 ///////
    //    Connection Management
 ///////

   // Open the connection
   // Returns
   //    -      0 if failed
   //    -     -1 if already open
   //    -      1 if ok

int
Transport::OpenConnection ()
{
  if (!_connection)
    return 0;

  if (_connection->IsOpen () && _connection->IsConnected ())
    return -1;                  // Already open and connection is up

  // No open connection
  // Let's open a new one

  if (_connection->Open () == NOTOK)
    return 0;                   // failed

  _tot_open++;
  return 1;
}


   // Assign the server to the connection

int
Transport::AssignConnectionServer ()
{
  if (debug > 5)
    cout << "\tAssigning the server (" << _host << ") to the TCP connection"
      << endl;

  if (_connection == 0)
  {
    cout << "Transport::AssignConnectionServer: _connection is NULL\n";
    exit (0);
  }

  if (_connection->Assign_Server (_host) == NOTOK)
    return 0;

  _ip_address = _connection->Get_Server_IPAddress ();

  return 1;
}


   // Assign the remote server port to the connection

int
Transport::AssignConnectionPort ()
{
  if (debug > 5)
    cout << "\tAssigning the port (" << _port << ") to the TCP connection" <<
      endl;

  if (_connection == 0)
  {
    cout << "Transport::AssignConnectionPort: _connection is NULL\n";
    exit (0);
  }

  if (_connection->Assign_Port (_port) == NOTOK)
    return 0;

  return 1;
}




   // Connect
   // Returns
   //    -      0 if failed
   //    -     -1 if already connected
   //    -      1 if ok

int
Transport::Connect ()
{
  if (debug > 5)
    cout << "\tConnecting via TCP to (" << _host << ":" << _port << ")" <<
      endl;

  if (isConnected ())
    return -1;                  // Already connected

  if (_connection == 0)
  {
    cout << "Transport::Connection: _connection is NULL\n";
    exit (0);
  }

  if (_connection->Connect () == NOTOK)
    return 0;                   // Connection failed

  return 1;                     // Connected
}


   // Flush the connection

void
Transport::FlushConnection ()
{

  if (_connection)
  {
    _connection->Flush ();
  }

}


   // Close the connection
   // Returns
   //    -      0 if not open
   //    -      1 if closed ok


int
Transport::CloseConnection ()
{
  if (_connection == 0)
  {
    // We can't treat this as a fatal error, because CloseConnection()
    // may be called from our destructor after _connection already deleted.
    // cout << "Transport::CloseConnection: _connection is NULL\n";
    // exit(0);
    return 0;
  }

  if (_connection->IsOpen ())
    _connection->Close ();      // Close the connection
  else
    return 0;

  _tot_close++;

  return 1;
}


void
Transport::SetConnection (const String & host, int port)
{

  if (_port != -1)
  {
    // Already initialized
    // Let's check if the server or the port are changed

    bool ischanged = false;

    // Checking the connection server   
    if (_host != host)          // server is gonna change
      ischanged = true;

    // Checking the connection port
    if (_port != port)          // the port is gonna change
      ischanged = true;

    if (ischanged)
    {
      // Let's close any pendant connection with the old
      // server / port pair

      _tot_changes++;

      if (debug > 4)
        cout << setw (5) << GetTotOpen () << " - "
          << "Change of server. Previous connection closed." << endl;

      CloseConnection ();
    }
  }

  // Copy the host and port information to the object
  _host = host;
  _port = port;

}


// Create a new date time object containing the date specified in a string
HtDateTime *
Transport::NewDate (const char *datestring)
{

  while (isspace (*datestring))
    datestring++;               // skip initial spaces

  DateFormat df = RecognizeDateFormat (datestring);

  if (df == DateFormat_NotRecognized)
  {
    // Not recognized
    if (debug > 0)
      cout << "Date Format not recognized: " << datestring << endl;

    return 0;
  }

  HtDateTime *dt = new HtDateTime;

  dt->ToGMTime ();              // Set to GM time

  switch (df)
  {
    // Asc Time format
  case DateFormat_AscTime:
    dt->SetAscTime ((char *) datestring);
    break;
    // RFC 1123
  case DateFormat_RFC1123:
    dt->SetRFC1123 ((char *) datestring);
    break;
    // RFC 850
  case DateFormat_RFC850:
    dt->SetRFC850 ((char *) datestring);
    break;
  default:
    cout << "Date Format not handled: " << (int) df << endl;
    break;
  }

  return dt;

}


// Recognize the possible date format sent by the server
Transport::DateFormat Transport::RecognizeDateFormat (const char *datestring)
{
  const char * s;

  if ((s = strchr (datestring, ',')))
  {
    // A comma is present.
    // Two chances: RFC1123 or RFC850

    if (strchr (s, '-'))
      return DateFormat_RFC850; // RFC 850 recognized   
    else
      return DateFormat_RFC1123;        // RFC 1123 recognized
  }
  else
  {
    // No comma present

    // Let's try C Asctime:    Sun Nov  6 08:49:37 1994
    if (strlen (datestring) == 24)
      return DateFormat_AscTime;
  }

  return DateFormat_NotRecognized;

}


// This method is used to write into 'dest' the credentials contained in 's'
// according to the HTTP Basic access authorization [RFC2617]
// It is written in this abstract class because it is used also
// when dealing with HTTP proxies, no matter what protocol we are
// using (HTTP now, but FTP in the future).

void
Transport::SetHTTPBasicAccessAuthorizationString (String & dest,
                                                  const String & s)
{
  static char tbl[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/'
  };
  dest.trunc ();
  const char *p;
  int n = s.length ();
  int ch;

  for (p = s.get (); n > 2; n -= 3, p += 3)
  {
    ch = *p >> 2;
    dest << tbl[ch & 077];
    ch = ((*p << 4) & 060) | ((p[1] >> 4) & 017);
    dest << tbl[ch & 077];
    ch = ((p[1] << 2) & 074) | ((p[2] >> 6) & 03);
    dest << tbl[ch & 077];
    ch = p[2] & 077;
    dest << tbl[ch & 077];
  }

  if (n != 0)
  {
    char c1 = *p;
    char c2 = n == 1 ? 0 : p[1];

    ch = c1 >> 2;
    dest << tbl[ch & 077];

    ch = ((c1 << 4) & 060) | ((c2 >> 4) & 017);
    dest << tbl[ch & 077];

    if (n == 1)
      dest << '=';
    else
    {
      ch = (c2 << 2) & 074;
      dest << tbl[ch & 077];
    }
    dest << '=';
  }
}

// End of Transport.cc (it's a virtual class anyway!)
