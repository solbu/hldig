//
// HtCookieJar.cc
//
// HtCookieJar: This class stores/retrieves cookies.
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
// $Id: HtCookieJar.cc,v 1.3 2002/04/09 14:43:58 angusgb Exp $ 
//

#include "HtCookieJar.h"

///////
   //    Static variables initialization
///////

   // Debug level
   int HtCookieJar::debug = 0;

///////
   // 	 Writes the HTTP request line given a cookie
///////
   //
   // RFC2109: The syntax for the header is:
   // cookie          =       "Cookie:" cookie-version
   //                         1*((";" | ",") cookie-value)
   // cookie-value    =       NAME "=" VALUE [";" path] [";" domain]
   // cookie-version  =       "$Version" "=" value
   // NAME            =       attr
   // VALUE           =       value
   // path            =       "$Path" "=" value
   // domain          =       "$Domain" "=" value
   //


int HtCookieJar::WriteCookieHTTPRequest(const HtCookie &Cookie,
   String &RequestString, const int &NumCookies)
{
   // Writes the string to be sent to the web server
   if (NumCookies == 1)
      RequestString << "Cookie: $Version=\"1\"; ";
   else
      RequestString << "; " ;

   // Print complete debug info
   if (debug > 6)
   {
      cout << "Cookie info: NAME=" << Cookie.GetName() << " VALUE=" << Cookie.GetValue()
      	 << " PATH=" << Cookie.GetPath();

      if (Cookie.GetExpires())
      	 cout << " EXPIRES=" << Cookie.GetExpires()->GetRFC850();

      cout << endl;
   }
	 
   // Prepare cookie line for HTTP protocol
   RequestString << Cookie.GetName() << "=" << Cookie.GetValue();

   if (Cookie.GetPath().length() > 0)
      RequestString << " ;$Path=" << Cookie.GetPath();

   if (Cookie.GetDomain().length() > 0)
      RequestString << " ;$Domain=" << Cookie.GetDomain();

   return true;
      
}


int HtCookieJar::GetDomainMinNumberOfPeriods(const String& domain) const
{
    // Well ... if a domain has been specified, we need some check-ups
    // as the standard says.
    static char* TopLevelDomains[] = { "com", "edu", "net", "org",
        "gov", "mil", "int", 0};

    const char* s = strrchr(domain.get(), '.');

    if (!s) // no 'dot' has been found. Not valid
        return 0;

    if (! *(++s))   // nothing after the dot. Not Valid
        return 0;
    
    for (char** p = TopLevelDomains; *p; ++p)
    {
        if (!strncmp(*p, s, strlen(*p)))
            return 2;
    }
    
    return 3;   // By default the minimum value
}
