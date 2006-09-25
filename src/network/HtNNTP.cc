//
// HtNNTP.cc
//
// HtNNTP: Interface classes for NNTP messaging
//
// Gabriele Bartolini - Prato - Italia
// started: 01.08.2000
//
// Including:
// 	 -  Generic class
// 	 -  Response message class
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 2000-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: HtNNTP.cc,v 1.1.2.1 2006/09/25 23:51:00 aarnone Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "lib.h"
#include "Transport.h"
#include "HtNNTP.h"
#include "HtDebug.h"

extern HtDebug * debug;

#include <signal.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>      // for sscanf

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

   // Stats information
   	 int HtNNTP::_tot_seconds = 0;
   	 int HtNNTP::_tot_requests = 0;
   	 int HtNNTP::_tot_bytes = 0;

///////
   //    HtNNTP_Response class
   //
   //    Response message sent by the remote NNTP server
///////


// Construction

HtNNTP_Response::HtNNTP_Response()
{

}


// Destruction

HtNNTP_Response::~HtNNTP_Response()
{
}


void HtNNTP_Response::Reset()
{

   // Call the base class method in order to reset
   // the base class attributes

   Transport_Response::Reset();

}




///////
   //    HtNNTP generic class
   //
   //
///////


// Construction

HtNNTP::HtNNTP()
: Transport(new Connection()),
   _bytes_read(0),
   _useproxy(0)
{
}

// Destruction

HtNNTP::~HtNNTP()
{
  // Free the connection
  //
  CloseConnection();
  if (_connection)
    delete _connection;
  _connection = 0;
}


///////
   //    Manages the requesting process
///////

Transport::DocStatus HtNNTP::Request()
{

   DocStatus result = Document_ok;
   _response.Reset();   // Reset the response

   return result;

}


void HtNNTP::SetRequestCommand(String &cmd)
{

   cmd << "\r\n";

}




//*****************************************************************************
// int HtNNTP::ParseHeader()
//   Parse the header of the document
//
int HtNNTP::ParseHeader()
{
    String	line = 0;
    int		inHeader = 1;

    if (_response._modification_time)
    {
	delete _response._modification_time;
	_response._modification_time=NULL;
    }
    while (inHeader)
    {

      line.trunc();

      if(! _connection->Read_Line(line, "\n"))
         return -1;  // Connection down
	
      _bytes_read+=line.length();
      line.chop('\r');

      if (line.length() == 0)
         inHeader = 0;
      else
      {
         // Found a not-empty line
	
         debug->outlog(3, "Header line: %s\n", line.get());
	
         // Status - Line check
         char	*token = line.get();

         while (*token && !isspace(*token))
            token++;

         while (*token && isspace(*token))
            token++;
      }
    }

    if (_response._modification_time == NULL)
    {
      debug->outlog(3, "No modification time returned: assuming now\n");

         //Set the modification time
      _response._modification_time = new HtDateTime;
      _response._modification_time->ToGMTime(); // Set to GM time

    }

    return 1;

}


HtNNTP::DocStatus HtNNTP::GetDocumentStatus(HtNNTP_Response &r)
{

   // Let's give a look at the return status code

   HtNNTP::DocStatus returnStatus=Document_not_found;
   int statuscode;

   statuscode=r.GetStatusCode();

   if(statuscode==200)
   {
	    returnStatus = Document_ok;   // OK
   }

   // Exit the function
   return returnStatus;
	
}


int HtNNTP::ReadBody()
{

    _response._contents = 0;	// Initialize the string

    char	docBuffer[8192];
    int		bytesRead = 0;
    int		bytesToGo = _response._content_length;

    if (bytesToGo < 0 || bytesToGo > _max_document_size)
        bytesToGo = _max_document_size;

    if( _connection == NULL )
      {
	cout << "HtNNTP::ReadBody: _connection is NULL\n";
	exit(0);
      }


    while (bytesToGo > 0)
    {
        int len = bytesToGo< (int)sizeof(docBuffer) ? bytesToGo : (int)sizeof(docBuffer);
        bytesRead = _connection->Read(docBuffer, len);
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


///////
   //    Show the statistics
///////

ostream &HtNNTP::ShowStatistics (ostream &out)
{
   Transport::ShowStatistics(out);  // call the base class method

   out << " NNTP Requests             : " << GetTotRequests() << endl;
   out << " NNTP KBytes requested     : " << (double)GetTotBytes()/1024 << endl;
   out << " NNTP Average request time : " << GetAverageRequestTime()
      << " secs" << endl;

   out << " NNTP Average speed        : " << GetAverageSpeed()/1024
      << " KBytes/secs" << endl;

   return out;
}
