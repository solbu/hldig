//
// HtWordType:  Wrap some attributes to make is...() type
// functions and other common functions without having to manage
// the attributes or the exact attribute combination semantics.
//
// $Id: HtWordType.cc,v 1.1.2.1 1999/03/23 23:23:01 grdetil Exp $
//

#include "HtWordType.h"

HtWordType::InnerStatics HtWordType::statics;

// Must only be called once (no tests, though).
void
HtWordType::Initialize(Configuration &config)
{
  char *valid_punct = config["valid_punctuation"];
  char *extra_word_chars = config["extra_word_characters"];

  static String punct_and_extra(extra_word_chars);
  punct_and_extra.append(valid_punct);

  HtWordType::statics.extra_word_characters = extra_word_chars;
  HtWordType::statics.valid_punctuation = valid_punct;
  HtWordType::statics.other_chars_in_word = punct_and_extra;
}
