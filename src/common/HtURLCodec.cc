//
// HtURLCodec.cc
//
// HtURLCodec:  Specialized HtWordCodec which just caters to the
//              needs of "url_part_aliases" and "common_url_parts".
//              Used for coding URLs when they are on disk; the key and the
//              href field in db.docdb.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: HtURLCodec.cc,v 1.1.2.1 2006/09/25 23:50:31 aarnone Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */
#include "HtDebug.h"

#include "HtURLCodec.h"
#include "defaults.h" // For "config"

// Constructor: parses the appropriate parameters using the
// encapsulated HtWordCodec class.
// Only used in privacy.
HtURLCodec::HtURLCodec()
{
  HtConfiguration* config= HtConfiguration::config();
  StringList l1(config->Find("url_part_aliases"), " \t");
  StringList l2(config->Find("common_url_parts"), " \t");

  myWordCodec = new HtWordCodec(l1, l2, myErrMsg);
}


HtURLCodec::~HtURLCodec()
{
  HtDebug * debug = HtDebug::Instance();
  debug->outlog(10, "HtURLCodec destructor start\n");

  delete myWordCodec;

  debug->outlog(10, "HtURLCodec destructor done\n");
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
