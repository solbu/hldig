//
// HtRegexReplaceList.cc
//
// HtRegexReplaceList: Perform RegexReplace on a list of from/to pairs.
// 		       Patterns are applied in order; pattern matching 
// 		       doesn't stop when a match occurs.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtRegexReplaceList.cc,v 1.1.2.1 2000/08/21 02:33:13 ghutchis Exp $
//
//

#include "HtRegexReplaceList.h"
#include <stream.h>

HtRegexReplaceList::HtRegexReplaceList(StringList &list, int case_sensitive = 0)
{
	if (list.Count() & 1)
	{
		lastErrorMessage = "HtRegexReplaceList needs an even number of strings";
		return;
	}

	int i;
	String err;

	for (i = 0; i < list.Count(); i += 2)
	{
		String from = list[i];
		String to	= list[i+1];
		HtRegexReplace *replacer = new HtRegexReplace(from.get(), to.get(), case_sensitive);
		replacers.Add(replacer);		// Stash it even if there's an error so it will get destroyed later
		const String &err = replacer->lastError();
		if (err.length() != 0)
		{
			lastErrorMessage = err;
			return;
		}
	}
}

HtRegexReplaceList::~HtRegexReplaceList()
{
	// replacers gets chucked away
}

int HtRegexReplaceList::replace(String &str, int nullpattern = 0, int nullstr = 0)
{
	int repCount = replacers.Count();
	int doneCount = 0;

	for (int rep = 0; rep < repCount; rep++)
	{
		HtRegexReplace *replacer = (HtRegexReplace *) replacers[rep];
		if (replacer->replace(str, nullpattern, nullstr) > 0)
			doneCount++;
	}

	return doneCount;
}

const String &HtRegexReplaceList::lastError()
{
	return lastErrorMessage;
}

// End of HtRegexReplaceList.cc
