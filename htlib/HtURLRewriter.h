//
// HtURLRewriter
//
// $Id: HtURLRewriter.h,v 1.1.2.1 2001/09/27 22:02:11 grdetil Exp $
//
#ifndef __HtURLRewriter_h
#define __HtURLRewriter_h

#include "HtRegexReplaceList.h"

// Container for a RegexReplaceList (not subclassed from it due to
// portability-problems using initializers).
// Not for subclassing.
class HtURLRewriter
{
public:
  static HtURLRewriter *instance();
  virtual ~HtURLRewriter();

  inline int Replace(String &source) { return myRegexReplace->Replace(source); }

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
  String myErrMsg;
};

#endif /* __HtURLRewriter_h */
