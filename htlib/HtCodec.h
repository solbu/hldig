//
// HtCodec:  Provide a generic means to take a String, code
// it, and return the encoded string.  And vice versa.
//
// $Id: HtCodec.h,v 1.1 1999/01/21 13:43:03 ghutchis Exp $
//
// $Log: HtCodec.h,v $
// Revision 1.1  1999/01/21 13:43:03  ghutchis
// New files.
//
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
