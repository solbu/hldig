//
// HtZlibCodec:
// Provide a generic access to the zlib compression routines.
// If zlib is not present, encode and decode are simply assignment functions.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtZlibCodec.h,v 1.1 1999/06/12 22:23:52 ghutchis Exp $
//
//
#ifndef __HtZlibCodec_h
#define __HtZlibCodec_h

#include "htString.h"
#include "HtCodec.h"

class HtZlibCodec : public HtCodec
{
public:
  HtZlibCodec();
  ~HtZlibCodec();

  // Code what's in this string.
  String encode(const String &);

  // Decode what's in this string.
  String decode(const String &);

private:

};

#endif /* __HtZlibCodec_h */
