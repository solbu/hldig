//
// URL.h
//
// URL: A URL parsing class, implementing as closely as possible the standard
//      laid out in RFC2396 (e.g. http://www.faqs.org/rfcs/rfc2396.html)
//      including support for multiple schemes.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: URL.h,v 1.8 2004/05/28 13:15:12 lha Exp $
//

#ifndef _URL_h_
#define _URL_h_

#include "htString.h"

class URL
{
public:
  URL ();
  URL (const String & url);
    URL (const URL & rhs);
    URL (const String & ref, const URL & parent);

  void parse (const String & url);

  const String & host () const
  {
    return _host;
  }
  void host (const String & h)
  {
    _host = h;
  }

  int port () const
  {
    return _port;
  }
  void port (const int p)
  {
    _port = p;
  }
  int DefaultPort ();

  const String & service () const
  {
    return _service;
  }
  void service (const String & s)
  {
    _service = s;
  }

  const String & path () const
  {
    return _path;
  }
  void path (const String & p);

  int hopcount () const
  {
    return _hopcount;
  }
  void hopcount (int h)
  {
    _hopcount = h;
  }

  const String & user () const
  {
    return _user;
  }
  void user (const String & u)
  {
    _user = u;
  }

  const String & get () const
  {
    return _url;
  }
  void dump ();
  void normalize ();
  void rewrite ();
  const String & signature ();

  const URL & operator = (const URL & rhs);

private:
  String _url;
  String _path;
  String _service;
  String _host;
  int _port;
  int _normal;
  int _hopcount;
  String _signature;
  String _user;

  void removeIndex (String &, String &);
  void normalizePath ();
  void ServerAlias ();
  void constructURL ();
  // Number of slashes following service specifier.  eg service("http")=2
  static int slashes (const String &);
};


// Unreserved punctuation allowed unencoded in URLs.  We use a more restricted
// list of unreserved characters than allowed by RFC 2396 (which revises and
// replaces RFC 1738), because it can't hurt to encode any of these
// characters, and they can pose problems in some contexts.  RFC 2396 says
// that only alphanumerics, the unreserved characters "-_.!~*'(),", and
// reserved characters used for their reserved purposes may be used
// unencoded within a URL.  We encode reserved characters because we now
// encode URL parameter values individually before piecing together the whole
// query string using reserved characters.

#define UNRESERVED  "-_.!~*"

//String &encodeURL(String &, char *valid = "?_@.=&/:");
//String &encodeURL(String &, char *reserved = ";/?:@&=+$,");
//                         char *unreserved = "-_.!~*'()");
String & encodeURL (String &, char *valid = (char *) UNRESERVED);

String & decodeURL (String &);

#endif
