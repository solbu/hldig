//
// Methods for HtURLCodec
//
// $Id: HtURLCodec.cc,v 1.2 1999/02/28 22:30:57 hp Exp $
//

#include "HtURLCodec.h"
#include "defaults.h" // For "config"

// Constructor: parses the appropriate parameters using the
// encapsulated HtWordCodec class.
// Only used in privacy.
HtURLCodec::HtURLCodec()
{
  StringList l1(config["url_part_aliases"], " \t");
  StringList l2(config["common_url_parts"], " \t");

  myWordCodec = new HtWordCodec(l1, l2, myErrMsg);
}


HtURLCodec::~HtURLCodec()
{
  delete myWordCodec;
}


// Supposedly used as HtURLCodec::instance()->ErrMsg()
// to check if HtWordCodec liked what was fed.
String& HtURLCodec::ErrMsg()
{
  return myErrMsg;
}


// Canonical singleton interface.
HtURLCodec *
HtURLCodec::instance()
{
  static HtURLCodec *_instance = 0;

  if (_instance == 0)
  {
    _instance = new HtURLCodec();
  }

  return _instance;
}

// End of HtURLCodec.cc
