//
// HtCookie.h
//
// HtCookie: Class for cookies
//
// by Robert La Ferla.  Started 12/5/2000.
// Reviewed by G.Bartolini - since 24 Feb 2001
//
////////////////////////////////////////////////////////////
//
// The HtCookie class represents a single HTTP cookie.
//
// See "PERSISTENT CLIENT STATE HTTP COOKIES" Specification
// at http://www.netscape.com/newsref/std/cookie_spec.html
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
// $Id: HtCookie.h,v 1.2 2002/02/02 18:18:13 ghutchis Exp $ 
//

#ifndef _HTCOOKIE_H
#define _HTCOOKIE_H

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif

#include "Object.h"
#include "htString.h"
#include "HtDateTime.h"

class HtCookie : public Object
{
   public:

   ///////
      //    Construction/Destruction
   ///////

      HtCookie(); // default constructor
      HtCookie(const String &setCookieLine, const String& aURL);
      HtCookie(const String &aName, const String &aValue, const String& aURL);
      HtCookie(const HtCookie& rhs); // default constructor
      
      ~HtCookie();   // Destructor

   ///////
      //    Public Interface
   ///////

      void SetName(const String &aName) { name = aName; }
      void SetValue(const String &aValue) { value = aValue; }
      void SetPath(const String &aPath) { path = aPath; }
      void SetDomain(const String &aDomain) { domain = aDomain; }
      void SetExpires(const HtDateTime *aDateTime);
      void SetIsSecure(const bool flag) { isSecure = flag; }
      void SetIsDomainValid(const bool flag) { isDomainValid = flag; }
      void SetSrcURL(const String &aURL) { srcURL = aURL; }

      const String &GetName() const { return name; }
      const String &GetValue()const { return value; }
      const String &GetPath()const { return path; }
      const String &GetDomain()const { return domain; }
      const HtDateTime *GetExpires() const { return expires; }
      const bool getIsSecure() const { return isSecure; }
      const bool getIsDomainValid() const { return isDomainValid; }
      const String &GetSrcURL()const { return srcURL; }

      // Print debug info
      void printDebug();

      // Set the debug level   
      static void SetDebugLevel (int d) { debug=d;}   

      // Copy operator overload
      const HtCookie &operator = (const HtCookie &rhs);
      
   protected:

   ///////
      //    Date formats enumeration
   ///////

      enum DateFormat
      {
      	 DateFormat_RFC1123,
      	 DateFormat_RFC850,
      	 DateFormat_AscTime,
      	 DateFormat_NotRecognized
      };

   ///////
      //    Protected methods
   ///////

      char * stripAllWhitespace(const char * str);
      int SetDate(const char * datestring, HtDateTime &dt);
      DateFormat RecognizeDateFormat(const char * datestring);

      String name;
      String value;  
      String path;
      String domain;
      HtDateTime * expires;
      bool isSecure;
      bool isDomainValid;
      String srcURL;

   ///////
      //    Debug level
   ///////

      static int debug;

};

#endif

