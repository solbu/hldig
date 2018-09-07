//
// HtSGMLCodec.h
//
// HtSGMLCodec: A Specialized HtWordCodec class to convert between SGML 
//              ISO 8859-1 entities and high-bit characters.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: HtSGMLCodec.h,v 1.4 2004/05/28 13:15:12 lha Exp $
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
  static HtSGMLCodec *instance ();
    virtual ~ HtSGMLCodec ();

  // Similar to the HtWordCodec class.  Each string may contain
  // zero or more of words from the lists. Here we need to run
  // it through two codecs because we might have two different forms
  inline String encode (const String & uncoded) const
  {
    return myTextWordCodec->encode (myNumWordCodec->encode (uncoded));
  }

  // But we only want to decode into one form i.e. &foo; NOT &#nnn;
  String decode (const String & coded) const
  {
    return myTextWordCodec->decode (coded);
  }

  // If an error was discovered during the parsing of
  // entities, this returns an error message
  String & ErrMsg ();

  // egcs-1.1 (and some earlier versions) always erroneously
  // warns (even without warning flags) about classic singleton
  // constructs ("only defines private constructors and has no
  // friends").  Rather than adding autoconf tests to shut these
  // versions up with -Wno-ctor-dtor-privacy, we fake normal
  // conformism for it here (the minimal effort).
  friend void my_friend_Harvey__a_faked_friend_function ();

private:
  // Hide default-constructor, copy-constructor and assignment
  // operator, making this a singleton.
    HtSGMLCodec ();
    HtSGMLCodec (const HtSGMLCodec &);
  void operator= (const HtSGMLCodec &);

  HtWordCodec *myTextWordCodec; // For &foo;
  HtWordCodec *myNumWordCodec;  // For &#foo;
  String myErrMsg;
};

#endif /* __HtSGMLCodec_h */
