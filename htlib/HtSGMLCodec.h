//
// HtSGMLCodec.h
//
// HtSGMLCodec: A Specialized HtWordCodec class to convert between SGML 
//              ISO 8859-1 entities and high-bit characters.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtSGMLCodec.h,v 1.2 1999/09/08 14:42:29 loic Exp $
//
#ifndef __HtSGMLCodec_h
#define __HtSGMLCodec_h

#include "HtWordCodec.h"

// Container for a HtWordCodec (not subclassed from it due to
// portability-problems using initializers).
// Not for subclassing.
class HtSGMLCodec
{
public:
  static HtSGMLCodec *instance();
  virtual ~HtSGMLCodec();

  // Same as in the HtWordCodec class.  Each string may contain
  // zero  or more of words from the lists.
  inline String encode(const String &uncoded) const
  { return myWordCodec->encode(uncoded); }

  String decode(const String &coded) const
  { return myWordCodec->decode(coded); }

  // If an error was discovered during the parsing of
  // entities, this returns an error message
  String& ErrMsg();

  // egcs-1.1 (and some earlier versions) always erroneously
  // warns (even without warning flags) about classic singleton
  // constructs ("only defines private constructors and has no
  // friends").  Rather than adding autoconf tests to shut these
  // versions up with -Wno-ctor-dtor-privacy, we fake normal
  // conformism for it here (the minimal effort).
  friend void my_friend_Harvey__a_faked_friend_function();

private:
  // Hide default-constructor, copy-constructor and assignment
  // operator, making this a singleton.
  HtSGMLCodec();
  HtSGMLCodec(const HtSGMLCodec &);
  void operator= (const HtSGMLCodec &);

  HtWordCodec *myWordCodec;
  String myErrMsg;
};

#endif /* __HtSGMLCodec_h */
