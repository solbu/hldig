//
// HtURLRewriter.cc
//
// HtURLRewriter:  Container for a HtRegexReplaceList (not subclassed from it due to
//                 portability-problems using initializers).
//                 Not for subclassing.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 2000-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: HtURLRewriter.cc,v 1.4 2004/05/28 13:15:12 lha Exp $
//

#include "HtURLRewriter.h"
#include "defaults.h" // For "config"

// Constructor: parses the appropriate parameters using the
// encapsulated RegexReplaceList class.
// Only used in privacy.
HtURLRewriter::HtURLRewriter()
{
  HtConfiguration* config= HtConfiguration::config();
  StringList list(config->Find("url_rewrite_rules"), " \t");

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
