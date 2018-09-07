//
// HtZlibCodec.h
//
// HtZlibCodec: Provide a generic access to the zlib compression routines.
//              If zlib is not present, encode and decode are simply 
//              assignment functions.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: HtZlibCodec.h,v 1.4 2004/05/28 13:15:12 lha Exp $
//
//
#ifndef __HtZlibCodec_h
#define __HtZlibCodec_h

#include "htString.h"
#include "HtCodec.h"

class HtZlibCodec:public HtCodec
{
public:
  static HtZlibCodec *instance ();
   ~HtZlibCodec ();

  // Code what's in this string.
  String encode (const String &) const;

  // Decode what's in this string.
  String decode (const String &) const;

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
    HtZlibCodec ();
    HtZlibCodec (const HtZlibCodec &);
  void operator= (const HtZlibCodec &);
};

#endif /* __HtZlibCodec_h */
