//
// HtURLRewriter.h
//
// HtURLRewriter:  Container for a HtRegexReplaceList (not subclassed from it due to
//                 portability-problems using initializers).
//                 Not for subclassing.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 2000-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: HtURLRewriter.h,v 1.4 2004/05/28 13:15:12 lha Exp $
//
#ifndef __HtURLRewriter_h
#define __HtURLRewriter_h

#include "HtRegexReplaceList.h"

class HtURLRewriter
{
public:
  static HtURLRewriter *instance();
  virtual ~HtURLRewriter();

  inline int replace(String &src) { return myRegexReplace->replace(src); }

  // If an error was discovered during the parsing of
  // config directives, this member gives a
  // nonempty String with an error message.
  const String& ErrMsg();

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
  HtURLRewriter();
  HtURLRewriter(const HtURLRewriter &);
  void operator= (const HtURLRewriter &);

  HtRegexReplaceList *myRegexReplace;
};

#endif /* __HtURLRewriter_h */
