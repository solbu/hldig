//
// cgi.cc
//
// cgi: Parse cgi arguments and put them in a dictionary.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: cgi.cc,v 1.9 2004/05/28 13:15:12 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#ifdef _MSC_VER /* _WIN32 */
#include <io.h>
#endif

#include "cgi.h"
#include "htString.h"
#include "Dictionary.h"
#include "good_strtok.h"
#include "StringList.h"
#include "URL.h"

#include <stdlib.h>

#ifndef _MSC_VER /* _WIN32 */
#include <unistd.h>
#endif

#ifdef HAVE_STD
#include <fstream>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif
#else
#include <fstream.h>
#endif /* HAVE_STD */

//*****************************************************************************
// cgi::cgi()
//
cgi::cgi()
{
  init("");
}


//*****************************************************************************
// cgi::cgi(char *s)
//
cgi::cgi(char *s)
{
  init(s);
}


//*****************************************************************************
// void cgi::init(char *s)
//
void
cgi::init(const char *s)
{
  pairs = new Dictionary;

  int i;
  String  method(getenv("REQUEST_METHOD"));

  if ((!s || !*s) && method.length() == 0)
  {
    //
    // Interactive mode
    //
    query = 1;
    return;
  }
  query = 0;
  String  results;

  if (s && *s && method.length() == 0)
  {
    results = s;
  }
  else if (strcmp((char*)method, "GET") == 0)
  {
    results = getenv("QUERY_STRING");
  }
  else
  {
    int    n;
    char  *buf;
    
    buf = getenv("CONTENT_LENGTH");
    if (!buf || !*buf || (n = atoi(buf)) <= 0)
      return;    // null query
    buf = new char[n + 1];
    int  r, i = 0;
    while (i < n && (r = read(0, buf+i, n-i)) > 0)
      i += r;
    buf[i] = '\0';
    results = buf;
    delete [] buf;
  }

  //
  // Now we need to split the line up into name/value pairs
  //
  StringList  list(results, "&;");
  
  //
  // Each name/value pair now needs to be added to the dictionary
  //
  for (i = 0; i < list.Count(); i++)
  {
    char  *name = good_strtok(list[i], '=');
    String  value(good_strtok(NULL, '\n'));
    value.replace('+', ' ');
    decodeURL(value);
    String  *str = (String *) pairs->Find(name);
    if (str)
    {
      //
      // Entry was already there.  Append it to the string.
      //
      str->append('\001');
      str->append(value);
    }
    else
    {
      //
      // New entry.  Add a new string
      //
      pairs->Add(name, new String(value));
    }
  }
}


//*****************************************************************************
// cgi::~cgi()
//
cgi::~cgi()
{
  delete pairs;
}


//*****************************************************************************
// char *cgi::operator [] (char *name)
//
const char *cgi::operator [] (const char *name)
{
  return get(name);
}


//*****************************************************************************
// char *cgi::get(char *name)
//
const char *cgi::get(const char *name)
{
  String  *str = (String *) (*pairs)[name];
  if (str)
    return str->get();
  else
  {
    if (query)
    {
      char  buffer[1000];
      cerr << "Enter value for " << name << ": ";
      cin.getline(buffer, sizeof(buffer));
      pairs->Add(name, new String(buffer));
      str = (String *) (*pairs)[name];
      return str->get();
    }
    return 0;
  }
}


//*****************************************************************************
// int cgi::exists(char *name)
//
int
cgi::exists(char *name)
{
  return pairs->Exists(name);
}

//*****************************************************************************
// char *cgi::path()
//
char *cgi::path()
{
  static char  buffer[1000] = "";

  if (query)
  {
    if (*buffer)
      return buffer;
    cerr << "Enter PATH_INFO: ";
    cin.getline(buffer, sizeof(buffer));
    return buffer;
  }
  return getenv("PATH_INFO");
}


