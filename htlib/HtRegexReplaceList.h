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
// $Id: HtRegexReplaceList.h,v 1.1.4.1 2001/09/27 22:02:11 grdetil Exp $
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
	int Replace(String &str, int nullpattern = 0, int nullstr = 0);
	virtual const String &lastError();

private:
	List replacers;
	String lastErrorMessage;
};

#endif /* __HtRegexReplaceList_h */
