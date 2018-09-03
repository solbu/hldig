//
// HtRegexReplaceList.cc
//
// HtRegexReplaceList: Perform RegexReplace on a list of from/to pairs.
//            Patterns are applied in order; pattern matching 
//            doesn't stop when a match occurs.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 2000-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: HtRegexReplaceList.cc,v 1.5 2004/05/28 13:15:21 lha Exp $
//
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "HtRegexReplaceList.h"

#ifdef HAVE_STD
#include <iostream>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif
#else
#include <iostream.h>
#endif /* HAVE_STD */

HtRegexReplaceList::HtRegexReplaceList (StringList & list, int case_sensitive)
{
  if (list.Count () & 1)
  {
    lastErrorMessage = "HtRegexReplaceList needs an even number of strings";
    return;
  }

  int i;
  String err;

  for (i = 0; i < list.Count (); i += 2)
  {
    String from = list[i];
    String to = list[i + 1];
    HtRegexReplace *replacer =
      new HtRegexReplace (from.get (), to.get (), case_sensitive);
    replacers.Add (replacer);   // Stash it even if there's an error so it will get destroyed later
    const String & err = replacer->lastError ();
    if (err.length () != 0)
    {
      lastErrorMessage = err;
      return;
    }
  }
}

HtRegexReplaceList::~HtRegexReplaceList ()
{
  // replacers gets chucked away
}

int
HtRegexReplaceList::replace (String & str, int nullpattern, int nullstr)
{
  int repCount = replacers.Count ();
  int doneCount = 0;

  for (int rep = 0; rep < repCount; rep++)
  {
    HtRegexReplace *replacer = (HtRegexReplace *) replacers[rep];
    if (replacer->replace (str, nullpattern, nullstr) > 0)
      doneCount++;
  }

  return doneCount;
}

const String &
HtRegexReplaceList::lastError ()
{
  return lastErrorMessage;
}

// End of HtRegexReplaceList.cc
