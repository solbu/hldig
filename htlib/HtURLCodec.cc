//
// Methods for HtURLCodec
//
// $Id: HtURLCodec.cc,v 1.1 1999/01/21 13:43:03 ghutchis Exp $
//
// $Log: HtURLCodec.cc,v $
// Revision 1.1  1999/01/21 13:43:03  ghutchis
// New files.
//
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
{}


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
