//
// HtCodec.h
//
// HtCodec:  Provide a generic means to take a String, code
//           it, and return the encoded string.  And vice versa.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtCodec.h,v 1.4 2002/02/01 22:49:33 ghutchis Exp $
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
