//
// HtCookie.cc
//
// HtCookie: This class represents a HTTP cookie.
//
// HtCookie.cc
//
// by Robert La Ferla.  Started 12/5/2000.
// Reviewed by G.Bartolini - since 24 Feb 2001
//
////////////////////////////////////////////////////////////
//
// HtCookie: This class allows you to store details about a single
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
// $Id: HtCookie.cc,v 1.5 2002/04/09 10:53:38 angusgb Exp $ 
//

#include "HtCookie.h"
#include <iostream.h>
#include <stdlib.h>
#include <ctype.h>

///////
   //    Static variables initialization
///////

   // Debug level
   int HtCookie::debug = 0;


// Default constructor
HtCookie::HtCookie()
:   name(0),
   value(0),
   path(0),
   domain(0),
   expires(0),
   isSecure(false),
   isDomainValid(true),
   srcURL(0)
{
}


// Constructor that accepts a name and a value
// and the calling URL
HtCookie::HtCookie(const String &aName, const String &aValue,
    const String& aURL)
:   name(aName),
   value(aValue),
   path(0),
   domain(0),
   expires(0),
   isSecure(false),
   isDomainValid(true),
   srcURL(aURL)
{
}


// Constructor from a server response header
HtCookie::HtCookie(const String &setCookieLine, const String& aURL)
:   name(0),
   value(0),
   path(0),
   domain(0),
   expires(0),
   isSecure(false),
   isDomainValid(true),
   srcURL(aURL)
{

   String cookieLineStr(setCookieLine);
   char * token;
   const char * str;

   if (debug > 5)
      cout << "Creating cookie from response header: " << cookieLineStr << endl;

   // Parse the cookie line
   token = strtok(cookieLineStr, "=");
   if (token != NULL)
   {
      SetName(token);
      token = strtok(NULL, ";");
      SetValue(token);
   }

   // Get all the fields returned by the server
   while ((str = strtok(NULL, "=")))
   {
      const char * ctoken;

      token = stripAllWhitespace(str);
    
      if (mystrcasecmp(token, "path") == 0)
      {
      	 // Let's grab the path
      	 ctoken = strtok(NULL, ";");
      	 SetPath(ctoken);
      }
      else
      {
      	 if (mystrcasecmp(token, "expires") == 0)
	     {
      	    // Let's grab the expiration date
      	    HtDateTime dt;
	
      	    ctoken = strtok(NULL, ";");

      	    if (ctoken && SetDate(ctoken, dt))
      	       SetExpires(&dt);
            else
               SetExpires(0);
            
      	 }
      	 else
      	 {
      	    if (mystrcasecmp(token, "secure") == 0)
      	       SetIsSecure(true);
      	    else
            {
      	       if (mystrcasecmp(token, "domain") == 0)
      	       {
      	          ctoken = strtok(NULL, ";");
      	          SetDomain(ctoken);
      	       }
      	    }
      	 }
      }

      if (token) delete[](token);

   }

   if (debug>3)   
      printDebug();

}


// Copy constructor
HtCookie::HtCookie(const HtCookie& rhs)
:   name(rhs.name),
   value(rhs.value),
   path(rhs.path),
   domain(rhs.domain),
   expires(rhs.expires),
   isSecure(rhs.isSecure),
   isDomainValid(rhs.isDomainValid),
   srcURL(rhs.srcURL)
{
}

// Destructor
HtCookie::~HtCookie()
{
   // Delete the DateTime info
   if (expires)
      delete expires;
}


// Set the expires datetime
void HtCookie::SetExpires(const HtDateTime *aDateTime)
{
   //
   // If expires has not yet been set,
   // we just copy the reference
   // otherwise, we just change the contents
   // of our internal attribute
   //
   
   // We don't have a valid datetime, it's null
   if (!aDateTime)
   {
      if (expires)
         delete expires;
         
      expires=0;

   }
   else
   {
      // We do have a valid datetime

      // Let's check whether expires has already been created
      if (!expires)
         expires = new HtDateTime(*aDateTime); // No ... let's create it and copy it

   }

}

// Strip all the whitespaces
char * HtCookie::stripAllWhitespace(const char * str)
{
  int len;
  int i;
  int j;
  char * newstr;

  len = strlen(str);
  newstr = new char[len + 1];
  j = 0;
  for (i = 0; i < len; i++) {
    char c;
    
    c = str[i];
    if (isspace(c) == 0)
      newstr[j++] = c;
  }
  newstr[j++] = (char)0;
  return newstr;
}


// Copy operator overload
const HtCookie &HtCookie::operator = (const HtCookie &rhs)
{

   // Prevent from copying itself
   if (this == &rhs)
       return *this;
   
   // Copy all the values

   name = rhs.name;
   value = rhs.value;
   path = rhs.path;
   domain = rhs.domain;
   srcURL = rhs.srcURL;

   // Set the expiration time   
   SetExpires(rhs.expires);

   isSecure = rhs.isSecure;
   isDomainValid = rhs.isDomainValid;
   
   return *this;
}
      

// Print a debug message
void HtCookie::printDebug()
{

   cout << "   - ";

   cout << "NAME=" << name << " VALUE=" << value << " PATH=" << path;

   if (expires)
      cout << " EXPIRES=" << expires->GetRFC850();

   if (domain.length())
      cout << " DOMAIN=" << domain << " ("
      << (isDomainValid?"VALID":"INVALID")
      << ")";

   if (isSecure)
      cout << " SECURE";

   cout << " - Issued by: " << srcURL;
   
   cout << endl;

}


//
// Set the date time value of a cookie's expires
// Given an HtDateTime object and a datestring
// It returns true if everything goes ok
// and false otherwise.
//
int HtCookie::SetDate(const char *datestring, HtDateTime &dt)
{
   if (!datestring) // for any reason we don't have a string for the date
      return 0;     // and we exit

   DateFormat df;

   while (*datestring && isspace(*datestring))
      datestring++; // skip initial spaces

   df = RecognizeDateFormat(datestring);
   if (df == DateFormat_NotRecognized)
   {
      // Not recognized
      if (debug > 0)
      cout << "Cookie '" << name
      	 << "' date format not recognized: " << datestring << endl;
    
      return false;
   }

   dt.ToGMTime(); // Set to GM time
   
   switch(df)
   {
      // Asc Time format
      case DateFormat_AscTime:
      	 dt.SetAscTime((char *)datestring);
         break;

      // RFC 1123
      case DateFormat_RFC1123:
      	 dt.SetRFC1123((char *)datestring);
      	 break;

      // RFC 850
      case DateFormat_RFC850:
      	 dt.SetRFC850((char *)datestring);
      	 break;

      default:
         if (debug > 0)
            cout << "Cookie '" << name
      	       << "' date format not handled: " << (int)df << endl;
      	 break;
   }

   return !(df==DateFormat_NotRecognized);
  
}


// Recognize the date sent by the server
//
// The expires attribute specifies a date string that defines the valid life time
// of that cookie. Once the expiration date has been reached, the cookie will no
// longer be stored or given out.
//
// The date string is formatted as:
// Wdy, DD-Mon-YYYY HH:MM:SS GMT
// This is based on RFC 822, RFC 850, RFC 1036, and RFC 1123, with the variations
// that the only legal time zone is GMT and the separators between the elements
// of the date must be dashes. 
//  

HtCookie::DateFormat HtCookie::RecognizeDateFormat(const char *datestring)
{

   register char *s;

   if (datestring)
   {

      if ((s=strchr(datestring, ',')))
      {
         // A comma is present.
         // Two chances: RFC1123 or RFC850
    
         if(strchr(s, '-'))
      	    return DateFormat_RFC850;  // RFC 850 recognized   
         else return DateFormat_RFC1123; // RFC 1123 recognized
      }
      else
      {
         // No comma present
    
         // Let's try C Asctime:    Sun Nov  6 08:49:37 1994
         if (strlen(datestring) == 24)
         {
      	    return DateFormat_AscTime;
         }
      }
   }

   return DateFormat_NotRecognized;

}


