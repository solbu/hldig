//
// HtCookieJar.h
//
// HtCookieJar: Abstract Class for storing/retrieving cookies
//
// by Robert La Ferla.  Started 12/9/2000.
// Reviewed by G.Bartolini - since 24 Feb 2001
//
////////////////////////////////////////////////////////////
//
// The HtCookieJar class stores/retrieves cookies.
// It's an abstract class though, which has to be the interface
// for HtHTTP class.
//
// The class has only 2 access point from the outside:
// - a method for cookies insertion (AddCookie());
// - a method for getting the HTTP request for cookies
//    (SetHTTPRequest_CookiesString).
//
// See "PERSISTENT CLIENT STATE HTTP COOKIES" Specification
// at http://www.netscape.com/newsref/std/cookie_spec.html
// Modified according to RFC2109 (max age and version attributes)
//
///////
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Part of the ht://Check package   <http://htcheck.sourceforge.net/>
// Copyright (c) 2001-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: HtCookieJar.h,v 1.6 2004/05/28 13:15:23 lha Exp $ 
//

#ifndef _HTCOOKIE_JAR_H
#define _HTCOOKIE_JAR_H

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif

#include "Object.h"
#include "htString.h"
#include "HtCookie.h"
#include "URL.h"

// for ShowSummary()
#ifdef HAVE_STD
#include <iostream>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif
#else
#include <iostream.h>
#endif /* HAVE_STD */


class HtCookieJar:public Object
{

public:

  ///////
  //    Construction/Destruction
  ///////

  HtCookieJar ()
  {
  };                            // empty
  virtual ~ HtCookieJar ()
  {
  };                            // empty

  ///////
  //    Interface methods
  ///////

  // This method allow the insertion of a cookie
  // into the jar.   
  virtual int AddCookie (const String & CookieString, const URL & url) = 0;

  // Set the request string to be sent to an HTTP server
  // for cookies. It manages all the process regarding
  // domains and subdomains.
  virtual int SetHTTPRequest_CookiesString (const URL & _url,
                                            String & RequestString) = 0;

  // Get the next cookie
  virtual const HtCookie *NextCookie () = 0;

  // Reset the iterator
  virtual void ResetIterator () = 0;

  // Get the minimum number of periods from a specified domain
  // returns 0 if not valid
  virtual int GetDomainMinNumberOfPeriods (const String & domain) const;

  // Set its debug level and HtCookie class'
  static void SetDebugLevel (int d)
  {
    debug = d;                  // internal one
    HtCookie::SetDebugLevel (d);        // HtCookie's debug level
  }

  // Show summary (abstract)
  virtual ostream & ShowSummary (ostream & out) = 0;

protected:

  ///////
  //    Protected attributes
  ///////

  // Writes the HTTP request line given a cookie
  virtual int WriteCookieHTTPRequest (const HtCookie & Cookie,
                                      String & RequestString,
                                      const int &NumCookies);

  // Print debug info
  virtual void printDebug () = 0;

  ///////
  //    Debug level
  ///////

  static int debug;

};

#endif
