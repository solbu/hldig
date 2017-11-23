//
// HtRegexList.cc
//
// HtRegex: A list of HtRegex objects for handling large regex patterns
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: HtRegexList.cc,v 1.5 2004/05/28 13:15:21 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "HtRegexList.h"
#include <locale.h>

class listnode
{
public:
  listnode * next;
  Object *object;
};

HtRegexList::HtRegexList ()
{
  compiled = 0;
}

HtRegexList::~HtRegexList ()
{
  compiled = 0;
}

const String &
HtRegexList::lastError ()
{
  return lastErrorMessage;
}

int
HtRegexList::setEscaped (StringList & list, int case_sensitive)
{
  if (list.Count () == 0)
  {
    compiled = false;
    return true;
  }

  String *str;
  String transformedLimits, currentPattern, prevPattern;
  HtRegex *limit = new HtRegex;

  list.Start_Get ();
  while ((str = (String *) list.Get_Next ()))
  {
    if (str->indexOf ('[') == 0
        && str->lastIndexOf (']') == str->length () - 1)
    {
      transformedLimits = str->sub (1, str->length () - 2).get ();
    }
    else                        // Backquote any regex special characters
    {
      transformedLimits = 0;
      for (int pos = 0; pos < str->length (); pos++)
      {
        if (strchr ("^.[$()|*+?{\\", str->Nth (pos)))
          transformedLimits << '\\';
        transformedLimits << str->Nth (pos);
      }
    }
    if (!currentPattern.empty ())
      currentPattern << "|";
    currentPattern << transformedLimits;
    if (!limit->set (currentPattern.get (), case_sensitive))
    {
      if (prevPattern.empty ()) // we haven't set anything yet!
      {
        lastErrorMessage = limit->lastError ();
        compiled = 0;
        return false;
      }
      limit->set (prevPattern.get (), case_sensitive);  // Go back a step
      Add (limit);
      limit = new HtRegex;
      currentPattern = transformedLimits;
      if (!limit->set (currentPattern.get (), case_sensitive))
      {
        lastErrorMessage = limit->lastError ();
        compiled = 0;
        return false;
      }
    }
    prevPattern = currentPattern;
  }
  Add (limit);                  // OK, we're done so just add the last compiled pattern

  compiled = 1;
  return true;
}

int
HtRegexList::match (const char *str, int nullpattern, int nullstr)
{
  HtRegex *regx;

  if (compiled == 0)
    return (nullpattern);
  if (str == NULL)
    return (nullstr);
  if (strlen (str) <= 0)
    return (nullstr);

  if (number == 0)
    return (1);                 // An empty pattern matches everything

  Start_Get ();
  while ((regx = (HtRegex *) Get_Next ()))
  {
    if (regx->match (str, nullpattern, nullstr))
    {
      // Move this one to the front and update pointers
      if (cursor.current_index != -1)
      {
        if (cursor.prev)
          cursor.prev->next = cursor.current->next;
        cursor.prev = 0;
        cursor.current->next = head;
        head = cursor.current;
        cursor.current = head;
        cursor.current_index = -1;
      }
      return (1);
    }
  }

  return (0);
}
