//
// HtURLCodec.cc
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// Methods for HtURLCodec
//
// $Id: HtURLCodec.cc,v 1.3 1999/09/08 14:42:29 loic Exp $
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
