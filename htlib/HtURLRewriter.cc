//
// Methods for HtURLRewriter
//
// $Id: HtURLRewriter.cc,v 1.1.2.1 2001/09/27 22:02:11 grdetil Exp $
//
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
