//
// HtCookieMemJar.h
//
// HtCookieMemJar: Class for storing/retrieving cookies
//
// by Robert La Ferla.  Started 12/9/2000.
// Reviewed by G.Bartolini - since 24 Feb 2001
//
////////////////////////////////////////////////////////////
//
// The HtCookieMemJar class stores/retrieves cookies
// directly into memory.
//
// See "PERSISTENT CLIENT STATE HTTP COOKIES" Specification
// at http://www.netscape.com/newsref/std/cookie_spec.html
// Modified according to RFC2109 (max age and version attributes)
//
///////
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Part of the ht://Check package   <http://htcheck.sourceforge.net/>
// Copyright (c) 2001 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtCookieMemJar.h,v 1.3 2002/04/09 14:43:58 angusgb Exp $ 
//

#ifndef _HTCOOKIE_MEM_JAR_H
#define _HTCOOKIE_MEM_JAR_H

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif

#include "Object.h"
#include "htString.h"
#include "Dictionary.h"
#include "List.h"
#include "HtCookieJar.h"
#include <iostream.h>   // for ShowSummary()

class HtCookieMemJar : public HtCookieJar
{

   public:
   
   ///////
      //    Construction/Destruction
   ///////

      HtCookieMemJar();
      virtual ~HtCookieMemJar();

   ///////
      //    Interface methods
   ///////
   
      // Set the request string to be sent to an HTTP server
      // for cookies. It manages all the process regarding
      // domains and subdomains.
      virtual int SetHTTPRequest_CookiesString(const URL &_url,
      	 String &RequestString);
	 
      virtual int AddCookie(const String &CookieString,
      	 const URL &url);

      // Get the next cookie
      virtual const HtCookie* NextCookie();

      // Reset the iterator
      virtual void ResetIterator();

      // Show stats
      virtual ostream &ShowSummary (ostream &out);

      void printDebug();

   protected:

   ///////
      //    Protected methods
   ///////

      // Passed a domain, this method writes all the cookies
      // directly in the request string for HTTP.
      int WriteDomainCookiesString(const URL &_url,
      	 const String &Domain, String &RequestString);

      // Get a list of the cookies for a domain
      List *cookiesForDomain(const String &DomainName);

      // Add a cookie in memory
      int AddCookieForHost(HtCookie *cookie, String HostName);
      
   ///////
      //    Protected attributes
   ///////

      ///////
         //    Internal dictionary of cookies
      ///////

      Dictionary * cookieDict;
      char* _key;    // For iteration purposes
      List* _list;   // ditto
      int _idx;      // ditto

};

#endif

