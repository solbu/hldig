//
// HtRegexList.cc
//
// HtRegex: A list of HtRegex objects for handling large regex patterns
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2001 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtRegexList.cc,v 1.1.2.2 2001/02/15 17:06:21 ghutchis Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "HtRegexList.h"
#include <locale.h>

class listnode
{
public:

  listnode		*next;
  listnode		*prev;
  Object		*object;
};

HtRegexList::HtRegexList()
{
	compiled = 0;
}

HtRegexList::~HtRegexList()
{
	compiled = 0;
}

const String &HtRegexList::lastError()
{
	return lastErrorMessage;
}

int
HtRegexList::setEscaped(StringList &list, int case_sensitive)
{
    String  *str;
    String  transformedLimits;
    HtRegex *limit;

    compiled = 1; // Assume we'll succeed for now
    list.Start_Get();
    while ((str = (String *) list.Get_Next()))
      {
	if (str->indexOf('[') == 0 && str->lastIndexOf(']') == str->length()-1)
	  {
	    transformedLimits = str->sub(1,str->length()-2).get();
	  }
	else 	// Backquote any regex special characters
	  {
	    transformedLimits = 0;
	    for (int pos = 0; pos < str->length(); pos++)
	      { 
		if (strchr("^.[$()|*+?{\\", str->Nth(pos)))
		  transformedLimits << '\\';
		transformedLimits << str->Nth(pos);
	      }
	  }
	limit = new HtRegex;
	compiled = compiled && limit->set(transformedLimits.get(), case_sensitive);
	lastErrorMessage = limit->lastError();
	Add(limit);
      }
    return compiled;
}

int
HtRegexList::match(const char * str, int nullpattern, int nullstr)
{
  HtRegex *regx;
	
  if (compiled == 0) return(nullpattern);
  if (str == NULL) return(nullstr);
  if (strlen(str) <= 0) return(nullstr);

  if (number == 0) return(1);	// An empty pattern matches everything

  Start_Get();
  while ((regx = (HtRegex *) Get_Next()))
    {
      if (regx->match(str, nullpattern, nullstr))
	{
	  // Move this one to the front and update pointers
	  if (cursor.current_index != -1)
	    {
	      if (cursor.current->prev)
		cursor.current->prev->next = cursor.current->next;
	      if (cursor.current->next)
		cursor.current->next->prev = cursor.current->prev;
	      cursor.current->prev = 0;
	      cursor.current->next = head;
	      head->prev = cursor.current;
	      head = cursor.current;
	      cursor.current = head;
	      cursor.current_index = 0;
	    }
	  return(1);
	}
    }

  return(0);
}

