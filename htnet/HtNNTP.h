//
// HtNNTP.h
//
// HtNNTP: Class for NNTP messaging (derived from Transport)
//
// Gabriele Bartolini - Prato - Italia
// started: 01.08.2000
//
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the General GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: HtNNTP.h,v 1.5 2004/05/28 13:15:23 lha Exp $
//

#ifndef _HTNNTP_H
#define _HTNNTP_H

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif

#include "Transport.h"
#include "URL.h"
#include "htString.h"

// for HtNNTP::ShowStatistics
#ifdef HAVE_STD
#include <iostream>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif
#else
#include <iostream.h>
#endif /* HAVE_STD */


// In advance declarations

class HtNNTP;


class HtNNTP_Response : public Transport_Response
{

   friend class HtNNTP;    // declaring friendship

   public:
///////
   //    Construction / Destruction
///////
  
      HtNNTP_Response();
      ~HtNNTP_Response();


///////
   //    Interface
///////

      // Reset
   void Reset();
  
   protected:

   // Other header information

};



class HtNNTP : public Transport
{
public:

///////
   //    Construction/Destruction
///////

    HtNNTP();
    ~HtNNTP();

///////
   //    Sends an NNTP request message
///////

   // manages a Transport request (method inherited from Transport class)
   virtual DocStatus Request ();

///////
   //    Control of member the variables
///////

 ///////
    //    Interface for resource retrieving
 ///////

   // Set and get the document to be retrieved
   void SetRequestURL(URL &u) { _url = u;}
   URL GetRequestURL () { return _url;}


   Transport_Response *GetResponse()
   {
      if (_response._status_code != -1)
         return &_response;
      else return NULL;
   }

   // Get the document status 
   virtual DocStatus GetDocumentStatus()
      { return GetDocumentStatus (_response); }
   
   // It's a static method
   static DocStatus GetDocumentStatus(HtNNTP_Response &);


// Manage statistics

   static int GetTotSeconds () { return _tot_seconds; }

   static int GetTotRequests () { return _tot_requests; }

   static int GetTotBytes () { return _tot_bytes; }

   static double GetAverageRequestTime ()
      { return _tot_seconds?( ((double) _tot_seconds) / _tot_requests) : 0; }

   static float GetAverageSpeed ()
      { return _tot_bytes?( ((double) _tot_bytes) / _tot_seconds) : 0; }

   static void ResetStatistics ()
      { _tot_seconds=0; _tot_requests=0; _tot_bytes=0;}

   // Show stats
   static ostream &ShowStatistics (ostream &out);

   // Proxy settings
  void SetProxy(int aUse) { _useproxy=aUse; }

protected:

///////
   //    Member attributes
///////

   ///////
      //    NNTP single Request information (Member attributes)
   ///////

   int      _bytes_read;        // Bytes read
   URL    _url;               // URL to retrieve
   int    _useproxy;          // Shall we use a proxy?


   ///////
      //    NNTP Response information
   ///////

   HtNNTP_Response   _response;    // Object where response
                                  // information will be stored into

   ///////
      //    Set the string of the command containing the request
   ///////

   void SetRequestCommand(String &);

   ///////
      //    Parse the header returned by the server
   ///////

   int ParseHeader();

   ///////
      //    Read the body returned by the server
   ///////

   int ReadBody();

///////
   //    Static attributes and methods
///////
   
   static int  _tot_seconds;     // Requests last (in seconds)
   static int  _tot_requests;    // Number of requests
   static int  _tot_bytes;       // Number of bytes read

};

#endif

