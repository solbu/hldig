///////////////////////////////////////////////////////////////
//
// File: HtCookieInFileJar.cc - Definition of class 'HtCookieInFileJar'
// 
// Author: Gabriele Bartolini <angusgb@users.sf.net>
// Started: Mon Jan 27 14:38:42 CET 2003
//
// Class which allows a cookie file to be imported in memory
// for ht://Check and ht://Dig applications.
//
// The cookie file format is a text file, as proposed by Netscape,
// and each line contains a name-value pair for a cookie.
// Fields within a single line are separated by the 'tab' character;
// Here is the format for a line, as taken from http://www.cookiecentral.com/faq/#3.5:
// 
// domain - The domain that created AND that can read the variable.
// flag - A TRUE/FALSE value indicating if all machines within a given domain
//  can access the variable. This value is set automatically by the browser,
//  depending on the value you set for domain.
// path - The path within the domain that the variable is valid for.
// secure - A TRUE/FALSE value indicating if a secure connection with the
//  domain is needed to access the variable.
// expiration - The UNIX time that the variable will expire on. UNIX time is
//  defined as the number of seconds since Jan 1, 1970 00:00:00 GMT.
// name - The name of the variable.
// value - The value of the variable.
//
//
///////////////////////////////////////////////////////////////
//
// Part of the ht://Check <http://htcheck.sourceforge.net/>
// Copyright (c) 1999-2004 Comune di Prato, Italia
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
///////////////////////////////////////////////////////////////
// $Id: HtCookieInFileJar.cc,v 1.5 2004/05/28 13:15:23 lha Exp $
///////////////////////////////////////////////////////////////

#ifndef __HtCookieInFileJar_H
#include "HtCookieInFileJar.h"
#endif

#include <stdio.h>

#define MAX_COOKIE_LINE 16384

// Costruttore (default constructor)
HtCookieInFileJar::HtCookieInFileJar(const String& fn, int& result)
: _filename(fn)
{
  result = Load();
}

// Costruttore di copia (copy constructor)
HtCookieInFileJar::HtCookieInFileJar(const HtCookieInFileJar& rhs)
{
}

// Distruttore
HtCookieInFileJar::~HtCookieInFileJar()
{
}

// Operatore di assegnamento (assignment operator)
HtCookieInFileJar& HtCookieInFileJar::operator=(const HtCookieInFileJar& rhs)
{
  if (this == &rhs)
    return *this;

  // Code for attributes copy

  return *this; // ritorna se stesso
}


// Loads the contents of a cookies file into memory
int HtCookieInFileJar::Load()
{
  FILE *f = fopen((const char *)_filename, "r");

  if (f == NULL)
    return -1;
  
  char buf[MAX_COOKIE_LINE];
  while(fgets(buf, MAX_COOKIE_LINE, f))
  {
    if (*buf && *buf != '#' && (strlen(buf) > 10))  // 10 is an indicative value
    {
      HtCookie *Cookie = new HtCookie(buf);

      // Interface to the insert method  
      // If the cookie is not valid or has not been added, we'd better delete it
      if (!Cookie->GetName().length()
        || !AddCookieForHost (Cookie, Cookie->GetSrcURL()))
      {
        if (debug > 2)
          cout << "Discarded cookie line: " << buf;
        delete Cookie;
      }
      
    }
  }
  
  return 0;
  
}


// Outputs a summary of the cookies that have been imported
ostream &HtCookieInFileJar::ShowSummary(ostream &out)
{

  char * key;
  int num_cookies = 0; // Global number of cookies
  
  cookieDict->Start_Get();
  
  out << endl << "Cookies that have been correctly imported from: " << _filename << endl;
  
  while ((key = cookieDict->Get_Next()))
  {
    List * list;
    HtCookie * cookie;

    list = (List *)cookieDict->Find(key);
    list->Start_Get();
    
    while ((cookie = (HtCookie *)list->Get_Next()))
    {
      ++num_cookies;
      out << "  " << num_cookies << ". " << cookie->GetName()
        << ": " << cookie->GetValue() << " (Domain: " << cookie->GetDomain();
      if (debug > 1)
      {
        out << " - Path: " << cookie->GetPath();
        if (cookie->GetExpires())
          out << " - Expires: " << cookie->GetExpires()->GetRFC850();
      }
      out << ")" << endl;
    }
    

    // Global number of cookies
  }

  return out;

}

///////////////////////////////////////////////////////////////
