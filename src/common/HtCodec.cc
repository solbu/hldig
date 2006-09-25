//
// HtCodec.cc
//
// HtCodec:  Provide a generic means to take a String, code
//           it, and return the encoded string.  And vice versa.
//
// Keep constructor and destructor in a file of its own.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: HtCodec.cc,v 1.1.2.1 2006/09/25 23:50:30 aarnone Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */
#include "HtDebug.h"

#include "HtCodec.h"

HtCodec::HtCodec()
{ }

HtCodec::~HtCodec()
{
    HtDebug * debug = HtDebug::Instance();
    debug->outlog(10, "HtCodec destructor start\n");

    debug->outlog(10, "HtCodec destructor done\n");
}


// End of HtCodec.cc
