//
// HtURLRewriter.cc
//
// HtURLRewriter:  Container for a HtRegexReplaceList (not subclassed from it due to
//                 portability-problems using initializers).
//                 Not for subclassing.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtURLRewriter.cc,v 1.1.2.1 2000/08/21 02:33:12 ghutchis Exp $
//

#include "HtURLRewriter.h"
#include "defaults.h" // For "config"

// Constructor: parses the appropriate parameters using the
// encapsulated RegexReplaceList class.
// Only used in privacy.
HtURLRewriter::HtURLRewriter()
{
	StringList list(config["url_rewrite_rules"], " \t");

	myRegexReplace = new HtRegexReplaceList(list);
}


HtURLRewriter::~HtURLRewriter()
{
	delete myRegexReplace;
}

// Supposedly used as HtURLRewriter::instance()->ErrMsg()
// to check if RegexReplaceList liked what was fed.
const String& HtURLRewriter::ErrMsg()
{
	return myRegexReplace->lastError();
}


// Canonical singleton interface.
HtURLRewriter *
HtURLRewriter::instance()
{
	static HtURLRewriter *_instance = 0;

	if (_instance == 0)
	{
		_instance = new HtURLRewriter();
	}

	return _instance;
}

// End of HtURLRewriter.cc
