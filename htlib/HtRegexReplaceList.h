//
// HtRegexReplaceList.h
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
// $Id: HtRegexReplaceList.h,v 1.1.2.1 2000/08/21 02:33:13 ghutchis Exp $
//

#ifndef __HtRegexReplaceList_h
#define __HtRegexReplaceList_h

#include "HtRegexReplace.h"
#include "List.h"
#include "StringList.h"

class HtRegexReplaceList : public Object
{
public:
	// Construct a HtRegexReplaceList. |list| should contain an even
	// number of strings that constitute from/to pairs.
	HtRegexReplaceList(StringList &list, int case_sensitive = 0);
	virtual ~HtRegexReplaceList();
	int replace(String &str, int nullpattern = 0, int nullstr = 0);
	virtual const String &lastError();

private:
	List replacers;
	String lastErrorMessage;
};

#endif /* __HtRegexReplaceList_h */
