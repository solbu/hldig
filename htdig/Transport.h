//
// Transport.h
//
// A virtual transport interface class for accessing remote documents.
// Used to grab URLs based on the scheme (e.g. http://, ftp://...)
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Transport.h,v 1.1 1999/06/23 02:51:23 ghutchis Exp $
//
//

#ifndef _Transport_H
#define _Transport_H

#include "Object.h"
#include "HtDateTime.h"
#include "URL.h"
#include "htString.h"
#include "Connection.h"

#define DEFAULT_CONNECTION_TIMEOUT 15

// Declare in advance
class Transport;

// But first, something completely different. Here's the response class
class Transport_Response : public Object
{
  friend class Transport;    // declaring friendship
   
 public:
  Transport_Response();
  virtual ~Transport_Response();

  // Get the Content type
  virtual String GetContentType();

  // Get the Content length
  virtual int GetContentLength() const = 0;

  // Get the contents
  virtual String GetContents();

  // Get the modification time object pointer
  virtual HtDateTime *GetModificationTime() const = 0;

  // Get the access time object pointer
  virtual HtDateTime *GetAccessTime() const = 0;
   
 protected:
};


class Transport : public Object
{
 public:

  Transport();
  virtual ~Transport();

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

  // Make the request
  virtual DocStatus Request () { return Document_not_found;}
   
  // Set and get the connection time out value
  void SetTimeOut ( int t ) { _timeout=t; }
  int GetTimeOut () { return _timeout; }
  
  // Set and get the document to be retrieved
  void SetRequestURL(URL u) { _url = u;}
  URL GetRequestURL () { return _url;}

  // Set the modification date and time for If-Modified-Since   
  void SetRequestModificationTime (HtDateTime *p) { _modification_time=p; }
  void SetRequestModificationTime (HtDateTime &p)
    { SetRequestModificationTime (&p) ;}
  // Get the modification date time
  HtDateTime *GetRequestModificationTime () { return _modification_time; }

   // Get and set the max document size to be retrieved
  void SetRequestMaxDocumentSize (int s) { _max_document_size=s; }
  GetRequestMaxDocumentSize() const { return _max_document_size; }

  virtual const Transport_Response *GetResponse() { return 0;}
   
  virtual DocStatus GetDocumentStatus(Transport_Response &) 
    { return Document_not_found; }

   // Are we still connected?
  virtual bool isConnected() = FALSE;

 protected:

  int		_timeout;		// Connection timeout
  URL		_url;			// URL to retrieve
  HtDateTime	*_modification_time;	// Stored modification time if avail.
  int		_max_document_size;	// Max document size to retrieve

};

#endif

