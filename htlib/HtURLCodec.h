//
// HtURLCodec:  Specialized HtWordCodec which just caters to the
// needs of "url_part_aliases" and "common_url_parts".
//  Used for coding URLs when they are on disk; the key and the
// href field in db.docdb.
//
// $Id: HtURLCodec.h,v 1.1 1999/01/21 13:43:03 ghutchis Exp $
//
// $Log: HtURLCodec.h,v $
// Revision 1.1  1999/01/21 13:43:03  ghutchis
// New files.
//
//
#ifndef __HtURLCodec_h
#define __HtURLCodec_h

#include <HtWordCodec.h>

// Container for a HtWordCodec (not subclassed from it due to
// portability-problems using initializers).
// Not for subclassing.
class HtURLCodec
{
public:
  static HtURLCodec *instance();
  virtual ~HtURLCodec();

  // Same as in the HtWordCodec class.  Each string may contain
  // zero  or more of words from the lists.
  inline String encode(const String &uncoded) const
  { return myWordCodec->encode(uncoded); }

  String decode(const String &coded) const
  { return myWordCodec->decode(coded); }

  // If an error was discovered during the parsing of
  // url_part_aliases or common_url_parts, this member gives a
  // nonempty String with an error message.
  String& ErrMsg();

private:
  // Hide default-constructor, copy-constructor and assignment
  // operator, making this a singleton.
  HtURLCodec();
  HtURLCodec(const HtURLCodec &);
  void operator= (const HtURLCodec &);

  HtWordCodec *myWordCodec;
  String myErrMsg;
};

#endif /* __HtURLCodec_h */
