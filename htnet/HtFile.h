//
// HtFile.h
//
// HtFile: Class for local files (derived from Transport)
//
// Alexis Mikhailov, from HtHTTP.h by Gabriele Bartolini - Prato - Italia
// started: 03.05.1999
//
// ////////////////////////////////////////////////////////////
//
// The HtFile class should provide  an interface for retrieving local
// documents. It derives from Transport class.
//
///////
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: HtFile.h,v 1.6 2004/05/28 13:15:23 lha Exp $ 
//

#ifndef _HTFILE_H
#define _HTFILE_H

#include "Transport.h"
#include "URL.h"
#include "htString.h"


// In advance declarations

class HtFile;

class HtFile_Response : public Transport_Response
{

   friend class HtFile;    // declaring friendship
   
   public:
///////
   //    Construction / Destruction
///////
	 
      HtFile_Response();
      ~HtFile_Response();
};

class HtFile : public Transport
{
public:

///////
   //    Construction/Destruction
///////

    HtFile();
    ~HtFile();

   // Information about the method to be used in the request

   // manages a Transport request (method inherited from Transport class)
   virtual DocStatus Request ();

   // Determine Mime type of file from its extension
   static const String *Ext2Mime (const char *);

   // Determine Mime type of file from its contents
   static String File2Mime (const char *);

 ///////
    //    Interface for resource retrieving
 ///////
   
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

   HtFile_Response	 _response; 	 // Object where response
   	       	   	       	   	 // information will be stored into
};

#endif

