//
// HtFTP.h
//
// HtFTP: Class for FTP protocol access (derived from Transport)
//
// Søren Vejrup Carlsen, based on from HtFTP.h by Alexis Mikhailov
// started: 26.08.2002
//
// ////////////////////////////////////////////////////////////
//
// The HtFTP class should provide an interface for retrieving documents 
// from FTP-servers. It derives from Transport class.
//
///////
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: HtFTP.h,v 1.1.2.1 2006/09/25 23:50:59 aarnone Exp $ 
//

#ifndef _HTFTP_H
#define _HTFTP_H

#include "Transport.h"
#include "URL.h"
#include "htString.h"


// In advance declarations

class HtFTP;

class HtFTP_Response : public Transport_Response
{

   friend class HtFTP;    // declaring friendship
   
   public:
///////
   //    Construction / Destruction
///////
	 
      HtFTP_Response();
      ~HtFTP_Response();
};

class HtFTP : public Transport
{
public:

///////
   //    Construction/Destruction
///////

    HtFTP();
    ~HtFTP();

   // Information about the method to be used in the request

   // manages a Transport request (method inherited from Transport class)
   virtual DocStatus Request ();
   
 ///////
    //    Interface for resource retrieving
 ///////

   // Set and get the document to be retrieved
   void SetRequestURL(URL &u) { _url = u;}
   URL GetRequestURL () { return _url;}


   // Set and get the referring URL
   void SetRefererURL (URL u) { _referer = u;}
   URL GetRefererURL () { return _referer;}


 ///////
    //    Interface for the HTTP Response
 ///////

   // We have a valid response only if the status code is not equal to
   // initialization value
   
   Transport_Response *GetResponse()
   {
      if (_response._status_code != -1)
         return &_response;
      else return NULL;}


   // Get the document status 
   virtual DocStatus GetDocumentStatus();
   
protected:

///////
   //    Member attributes
///////

   ///////
      //    Http single Request information (Member attributes)
   ///////

   URL		_url;               // URL to retrieve
   URL		_referer;	    // Referring URL
   
   ///////
      //    Http Response information
   ///////

   HtFTP_Response	 _response; 	 // Object where response
   	       	   	       	   	 // information will be stored into
};

#endif

