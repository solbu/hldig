
// HtCookieMemJar.cc
//
// HtCookieMemJar: This class stores/retrieves cookies.
//
// by Robert La Ferla.  Started 12/9/2000.
// Reviewed by G.Bartolini - since 24 Feb 2001
//
////////////////////////////////////////////////////////////
//
// The HtCookieMemJar class stores/retrieves cookies
// directly into memory. It is derived from HtCookieJar class.
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
// $Id: HtCookieMemJar.cc,v 1.2 2002/02/02 18:18:13 ghutchis Exp $ 
//

#include "HtCookieMemJar.h"
#include "HtCookie.h"
#include "List.h"
#include "Dictionary.h"
#include <iostream.h>
#include <stdlib.h>
#include <ctype.h>

// Constructor
HtCookieMemJar::HtCookieMemJar()
: _key(0), _list(0), _idx(0)
{
   cookieDict = new Dictionary();
   cookieDict->Start_Get(); // reset the iterator
}

// Destructor
HtCookieMemJar::~HtCookieMemJar()
{
   if (debug>4)
      printDebug();

   if (cookieDict)
      delete cookieDict;
}

// Add a cookie to the Jar
int HtCookieMemJar::AddCookie(const String &CookieString, const URL &url)
{

   // Builds a new Cookie object
   HtCookie *Cookie = new HtCookie(CookieString, url.get());

   // Interface to the insert method   
   // If the cookie has not been added, we'd better delete it
   if (!AddCookieForHost (Cookie, url.host()))
      delete Cookie;

   return true;

}


// Add a cookie to a host
int HtCookieMemJar::AddCookieForHost(HtCookie *cookie, String HostName)
{

   List *list; // pointer to the Cookie list of an exact host
   HtCookie *theCookie;
   bool inList = false;

/////////////////////////////////////////////////////////////   
// That's an abstract from the Netscape Cookies specification
/////////////////////////////////////////////////////////////   
//
// When searching the cookie list for valid cookies,
// a comparison of the domain attributes of the cookie
// is made with the Internet domain name of the host from which the URL
// will be fetched. If there is a tail match, then the cookie
// will go through path matching to see if it should be sent.
//
// "Tail matching" means that domain attribute is matched against
// the tail of the fully qualified domain name of the host.
// A domain attribute of "acme.com" would match host names "anvil.acme.com"
// as well as "shipping.crate.acme.com". 
//
// Only hosts within the specified domain can set a cookie
// for a domain and domains must have at least two (2)
// or three (3) periods in them to prevent domains of
// the form: ".com", ".edu", and "va.us".
//
// Any domain that fails within one of the seven special top level domains
// listed below only require two periods.
// Any other domain requires at least three.
//
// The seven special top level domains are:
// "COM", "EDU", "NET", "ORG", "GOV", "MIL", and "INT".
// 
// The default value of domain is the host name of the
// server which generated the cookie response.
//
/////////////////////////////////////////////////////////////   


   // Let's get the domain of the cookie
   String Domain(cookie->GetDomain());

   // Lowercase the HostName
   HostName.lowercase();
       
   if (!Domain.length())
      Domain = HostName;
   else
   {
        Domain.lowercase(); // lowercase the domain
                
        // The cookie's domain must have a minimum number of periods
        // inside, as stated by the abstract cited above
        int minimum_periods = GetDomainMinNumberOfPeriods(Domain);

        if (!minimum_periods)
        {
            if (debug > 2)
                cout << "Cookie - Invalid domain "
                    << "(minimum number of periods): " << Domain << endl;

            cookie->SetIsDomainValid(false);
        }
        else
        {
            // Let's see if the domain is now valid
            const char* s = Domain.get();
            const char* r = s + strlen(s) - 1;  // go to the last char
            int num_periods = 1;    // at minimum is one
            
            while (r > s && *r)
            {
                if (*r == '.' && *(r+1) && *(r+1) != '.')
                    ++num_periods;  // when a 'dot' is found increment
                                    // the number of periods
                --r;
            }
                    
            if (num_periods >= minimum_periods) // here is a so-far valid domain
            {
                while (*r && *r == '.')
                    ++r;    // goes beyond the first dot

                if (r>s)
                    Domain.set((char*) r);  // Set the new 'shorter' domain

                if (HostName.indexOf(Domain.get()) != -1)
                {
                    if (debug > 2)
                        cout << "Cookie - valid domain: "
                            << Domain << endl;
                }
                else
                {
                    cookie->SetIsDomainValid(false);
                    if (debug > 2)
                        cout << "Cookie - Invalid domain "
                            << "(host not within the specified domain): " << Domain << endl;
                }
            }
            else
            {
                cookie->SetIsDomainValid(false);
                if (debug > 2)
                    cout << "Cookie - Invalid domain "
                        << "(minimum number of periods): " << Domain << endl;
            }
        }
   }

   if (! cookie->getIsDomainValid())   // Not a valid domain
        Domain = HostName;  // Set the default

   // Is the host in the dictionary?
   if (cookieDict->Exists(Domain) == 0)
   {
      // No, add a list instance
      list = new List();
      cookieDict->Add(Domain, list);
   }
   else list = (List *)cookieDict->Find(Domain);
   
   // Is cookie already in list?
   list->Start_Get();

   // Let's start looking for it
   // The match is made on the name and the path

   if (debug > 5)
      cout << "- Let's go searching for the cookie '"
         << cookie->GetName() << "' in the list" << endl;

   while (!inList && (theCookie = (HtCookie *)list->Get_Next()))
   {
      if ( (theCookie->GetName().compare(cookie->GetName()) == 0 )
      	 && ( theCookie->GetPath().compare(cookie->GetPath()) == 0 ))
      {
         // The cookie has been found
         inList = true;

         // Let's update the expiration datetime
         if (debug > 5)
            cout << " - Found: Update cookie expire time." << endl;

         theCookie->SetExpires(cookie->GetExpires());

      }
   }

   // Well ... the cookie wasn't in the list. Until now! ;-)
   // Let's go add it!
   if (inList == false)
   {
      if (debug > 5)
         cout << " - Not Found: let's go add it." << endl;

      list->Add((Object *)cookie);
   }

   return !inList;
}


// Retrieve all cookies that are valid for a domain
List * HtCookieMemJar::cookiesForDomain(const String &DomainName)
{
  List * list;

  list = (List *)cookieDict->Find(DomainName);
     return list;
}



int HtCookieMemJar::SetHTTPRequest_CookiesString(const URL &_url,
   String &RequestString)
{

    // Let's split the URL domain and get all of the subdomains.
    // For instance:
    // 	 - bar.com
    // 	 - foo.bar.com
    // 	 - www.foo.bar.com                                                                                               

    String Domain(_url.host());
    Domain.lowercase();
    
    int minimum_periods = GetDomainMinNumberOfPeriods(Domain);

    if (debug > 3)
        cout << "Looking for cookies - Domain: "
            << Domain 
            << " (Minimum periods: " << minimum_periods << ")" << endl;

    // Let's get the subdomains, starting from the end
    const char* s = Domain.get();
    const char* r = s + strlen(s) - 1;  // go to the last char
    int num_periods = 1;    // at minimum is one
            
    while (r > s && *r)
    {
        if (*r == '.' && *(r+1) && *(r+1) != '.')
        {
            ++num_periods;  // when a 'dot' is found increment
                            // the number of periods
            
            if (num_periods > minimum_periods) // here is a so-far valid domain
            {
                const String SubDomain(r+1);
                if (debug > 3)
                    cout << "Trying to find cookies for subdomain: "
                        << SubDomain << endl;

                if (cookieDict->Exists(SubDomain))
                    WriteDomainCookiesString(_url, SubDomain, RequestString);
            }
        }
        
        --r;
    }
                    
    if (num_periods >= minimum_periods
        && cookieDict->Exists(Domain))
            // Let's send cookies for this domain to the Web server ...
            WriteDomainCookiesString(_url, Domain, RequestString);

    return true;
}



/////////////////////////////////////////////////////////////   
// That's an abstract from the Netscape Cookies specification
/////////////////////////////////////////////////////////////   
//
//
// When requesting a URL from an HTTP server, the browser will match
// the URL against all cookies and if any of them match,
// a line containing the name/value pairs of all matching cookies
// will be included in the HTTP request.
//
// Here is the format of that line: 
// Cookie: NAME1=OPAQUE_STRING1; NAME2=OPAQUE_STRING2 ...
//
// This method writes on a string (RequestString) the headers
// for cookies settings as defined by Netscape standard
//
/////////////////////////////////////////////////////////////   

int HtCookieMemJar::WriteDomainCookiesString(const URL &_url,
   const String &Domain, String &RequestString)
{

   // Cookie support. We need a list of cookies and a cookie object
   List *cookieList;
   HtCookie *cookie;
   const HtDateTime now;   // Instant time, used for checking
                           // cookies expiration time

   // Let's find all the valid cookies depending on the specified domain
   cookieList = cookiesForDomain(Domain);

   if (cookieList)
   {
      // Let's store the number of cookies eventually sent
      int NumCookies = 0;

      if (debug > 5)
      	 cout << "Found a cookie list for: '" << Domain << "'" << endl;

      // Let's crawl the list for getting the 'path' matching ones
      cookieList->Start_Get();

      while ((cookie = (HtCookie *)cookieList->Get_Next()))
      {
      	 const String cookiePath = cookie->GetPath();
      	 const String urlPath = _url.path();

         //
         // Let's see if the cookie has expired
         // by checking the Expires value of it
         // If it's not empty and the datetime
         // is before now.
         //
         
         const bool expired = cookie->GetExpires() &&
            (*(cookie->GetExpires()) < now);

         if (debug > 5)
      	    cout << "Trying to match paths and expiration time: "
	       << urlPath << " in " << cookiePath;

      	 // Is the path matching
      	 if (!expired && !strncmp(cookiePath, urlPath, cookiePath.length()))
	 {

            if (debug > 5)
	       cout << " (passed)" << endl;

      	    ++NumCookies;
	    
      	    // Write the string by passing the cookie to the superclass' method
	    WriteCookieHTTPRequest(*cookie, RequestString, NumCookies);

      	 }
	 else if (debug > 5) cout << " (discarded)" << endl;

     }
     
     // Have we sent one cookie at least?
     if (NumCookies > 0)
       RequestString <<"\r\n";

   }

   // That's the end of function
   return true;
}


// Debug info
void HtCookieMemJar::printDebug()
{
   char * key;
  
   cookieDict->Start_Get();
   
   cout << "Summary of the cookies stored so far" << endl;
   
   while ((key = cookieDict->Get_Next()))
   {
      List * list;
      HtCookie * cookie;

      cout << " - View cookies for: '" << key << "'" << endl;
      list = (List *)cookieDict->Find(key);
      list->Start_Get();
      
      while ((cookie = (HtCookie *)list->Get_Next()))
      	 cookie->printDebug();
   }
}


///////
   //    Show the summary of the stored cookies
///////

ostream &HtCookieMemJar::ShowSummary(ostream &out)
{

   char * key;
   int num_cookies = 0; // Global number of cookies
   int num_server = 0;	// Number of servers with cookies
  
   cookieDict->Start_Get();
   
   out << endl << "Summary of the cookies" << endl;
   out << "======================" << endl;
   
   while ((key = cookieDict->Get_Next()))
   {
      List * list;
      HtCookie * cookie;
      int num_cookies_server = 0;

      ++num_server;	// Number of servers with cookies

      out << " Host: '" << key << "'" << endl;
      list = (List *)cookieDict->Find(key);
      list->Start_Get();
      
      while ((cookie = (HtCookie *)list->Get_Next()))
      {
      	 ++num_cookies_server;
      	 cookie->printDebug();
      }
      
      out << "   Number of cookies: " << num_cookies_server << endl << endl;

      // Global number of cookies
      num_cookies += num_cookies_server;
   }

   out << "Total number of cookies: " << num_cookies << endl;
   out << "Servers with cookies: " << num_server << endl << endl;

   return out;

}


// Get the next cookie. It is a bit tricky, but for now it is good
const HtCookie* HtCookieMemJar::NextCookie()
{
   if (!cookieDict)
      return 0;

   if (!_idx && (_key = cookieDict->Get_Next())
      && (_list = (List *)cookieDict->Find(_key)))
         _list->Start_Get();   // the first time we position at the beginning

   ++_idx;
   	 
   if (!_key)
      return 0;   // ends

   if (!_list)
      return 0;   // ends

   const HtCookie* cookie((const HtCookie*)_list->Get_Next()); // Cookie object
      
   if (cookie)
      return cookie;
   else
   {
      // Non ci sono cookie per l'host. Si passa a quello seguente
      if ((_key = cookieDict->Get_Next()) &&
      	 (_list = (List *)cookieDict->Find(_key)))
      {
         _list->Start_Get();
	 if ((cookie = (const HtCookie*)_list->Get_Next()))
	    return cookie;
      }
   }
   
   return 0;         
}

// Reset the iterator
void HtCookieMemJar::ResetIterator()
{
   cookieDict->Start_Get();
   _idx = 0;
}

