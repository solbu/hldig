//
// HtRegex.cc
//
// HtRegex: A simple C++ wrapper class for the system regex routines.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtRegex.cc,v 1.9.2.3 2000/05/06 20:46:40 loic Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "HtRegex.h"
#include <locale.h>


HtRegex::HtRegex()
{
	compiled = 0;
}

HtRegex::HtRegex(const char *str, int case_sensitive)
{
        set(str, case_sensitive);
}

HtRegex::~HtRegex()
{
	if (compiled != 0) regfree(&re);
	compiled = 0;
}

void
HtRegex::set(const char * str, int case_sensitive)
{
	compiled = 0;
	if (str == NULL) return;
	if (strlen(str) <= 0) return;
	if (!case_sensitive)
	{
	  if (regcomp(&re, str, REG_EXTENDED|REG_ICASE) == 0)
		compiled = 1;
	}
	else
	{
	  if (regcomp(&re, str, REG_EXTENDED) == 0)
		compiled = 1;
	}
}

void
HtRegex::setEscaped(StringList &list, int case_sensitive)
{
    String *str;
    String transformedLimits;
    list.Start_Get();
    while ((str = (String *) list.Get_Next()))
      {
	if (str->indexOf('[') == 0 && str->lastIndexOf(']') == str->length()-1)
	  {
	    transformedLimits << str->sub(1,str->length()-2).get();
	  }
	else 	// Backquote any regex special characters
	  {
	    for (int pos = 0; pos < str->length(); pos++)
	      { 
		if (strchr("^.[$()|*+?{\\", str->Nth(pos)))
		  transformedLimits << '\\';
		transformedLimits << str->Nth(pos);
	      }
	  }
	transformedLimits << "|";
      }
    transformedLimits.chop(1);

    set(transformedLimits, case_sensitive);
}

int
HtRegex::match(const char * str, int nullpattern, int nullstr)
{
	int	rval;
	
	if (compiled == 0) return(nullpattern);
	if (str == NULL) return(nullstr);
	if (strlen(str) <= 0) return(nullstr);
	rval = regexec(&re, str, (size_t) 0, NULL, 0);
	if (rval == 0) return(1);
	else return(0);
}

