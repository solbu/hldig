//
// HtWordType:  Wrap some attributes to make is...()-type methods.
//
// $Id: HtWordType.h,v 1.1.2.2 1999/03/29 19:01:44 grdetil Exp $
//

#ifndef __HtWordType_h
#define __HtWordType_h

#include "htString.h"
#include "Configuration.h"
#include <ctype.h>

//
// Inline friend-functions are used together with an all-statics
// class (name that pattern!) to spare the user from having
// to manage the valid_punctuation and extra_word_characters
// attributes, while in theory still having the runtime
// performance of strchr() + isalnum().
//

class HtWordType
{
public:
  friend int HtIsWordChar(int c);
  friend int HtIsStrictWordChar(int c);
  friend void HtStripPunctuation(String &s);

  // To be called once all attributes and configuration files
  // have been read.
  static void Initialize(Configuration & config);

private:

  // Wrap them up in a struct, so the compiler has a good chance
  // to see the proximity.
  static struct InnerStatics
  {
    char *valid_punctuation;     // The same as the attribute.
    char *extra_word_characters; // Likewise.
    char *other_chars_in_word;   // Attribute "valid_punctuation" plus
                                 // "extra_word_characters".
  } statics;

  // These methods are not supposed to be implemented (or accessed).
  HtWordType();
  HtWordType(const HtWordType &);
  void operator=(const HtWordType &);
};

// One for characters that when put together are a word
// (including punctuation).
inline int
HtIsWordChar(int c)
{
  return isalnum(c) || (c && strchr(HtWordType::statics.other_chars_in_word, c));
}

// Similar, but no punctuation characters.
inline int
HtIsStrictWordChar(int c)
{
  return isalnum(c) || (c && strchr(HtWordType::statics.extra_word_characters, c));
}

// Let caller get rid of getting and holding a configuration parameter.
inline void
HtStripPunctuation(String &s)
{
  s.remove(HtWordType::statics.valid_punctuation);
}

#endif /* __HtWordType_h */
