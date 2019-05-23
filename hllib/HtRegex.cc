//
// HtRegex.cc
//
// HtRegex: A simple C++ wrapper class for the system regex routines.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: HtRegex.cc,v 1.13 2004/05/28 13:15:21 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "hlconfig.h"
#endif /* HAVE_CONFIG_H */

#include "HtRegex.h"
#include <locale.h>


HtRegex::HtRegex ():compiled (0)
{
}

HtRegex::HtRegex (const char *str, int case_sensitive):compiled (0)
{
  set (str, case_sensitive);
}

HtRegex::~HtRegex ()
{
  if (compiled != 0)
    regfree (&re);
  compiled = 0;
}

const String &
HtRegex::lastError ()
{
  return lastErrorMessage;
}

int
HtRegex::set (const char *str, int case_sensitive)
{
  if (compiled != 0)
    regfree (&re);

  int err;
  compiled = 0;
  if (str == NULL)
    return 0;
  if (strlen (str) <= 0)
    return 0;
  if (err =
      regcomp (&re, str,
               case_sensitive ? REG_EXTENDED : (REG_EXTENDED | REG_ICASE)),
      err == 0)
  {
    compiled = 1;
  }
  else
  {
    size_t len = regerror (err, &re, 0, 0);
    char *buf = new char[len];
    regerror (err, &re, buf, len);
    lastErrorMessage = buf;
    delete[] buf;
  }
  return compiled;
}

int
HtRegex::setEscaped (StringList & list, int case_sensitive)
{
  String *str;
  String transformedLimits;
  list.Start_Get ();
  while ((str = (String *) list.Get_Next ()))
  {
    if (str->indexOf ('[') == 0
        && str->lastIndexOf (']') == str->length () - 1)
    {
      transformedLimits << str->sub (1, str->length () - 2).get ();
    }
    else                        // Backquote any regex special characters
    {
      for (int pos = 0; pos < str->length (); pos++)
      {
        if (strchr ("^.[$()|*+?{\\", str->Nth (pos)))
          transformedLimits << '\\';
        transformedLimits << str->Nth (pos);
      }
    }
    transformedLimits << "|";
  }
  transformedLimits.chop (1);

  return set (transformedLimits, case_sensitive);
}

int
HtRegex::match (const char *str, int nullpattern, int nullstr)
{
  int rval;

  if (compiled == 0)
    return (nullpattern);
  if (str == NULL)
    return (nullstr);
  if (strlen (str) <= 0)
    return (nullstr);
  rval = regexec (&re, str, (size_t) 0, NULL, 0);
  if (rval == 0)
    return (1);
  else
    return (0);
}
