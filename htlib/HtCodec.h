//
// HtCodec.h
//
// HtCodec:  Provide a generic means to take a String, code
//           it, and return the encoded string.  And vice versa.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtCodec.h,v 1.3 1999/09/11 05:03:51 ghutchis Exp $
//
#ifndef __HtCodec_h
#define __HtCodec_h

#include "htString.h"

class HtCodec : public Object
{
public:
  HtCodec();
  virtual ~HtCodec();

  // Code what's in this string.
  virtual String encode(const String &) const = 0;

  // Decode what's in this string.
  virtual String decode(const String &) const = 0;

private:
  HtCodec(const HtCodec &);     // Not supposed to be implemented.
  void operator= (const HtCodec &); // Not supposed to be implemented.
};

#endif /* __HtCodec_h */
