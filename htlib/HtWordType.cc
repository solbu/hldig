//
// HtWordType:  Wrap some attributes to make is...() type
// functions and other common functions without having to manage
// the attributes or the exact attribute combination semantics.
//
// $Id: HtWordType.cc,v 1.1.2.4 1999/12/03 17:44:16 grdetil Exp $
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
  HtWordType::statics.chrtypes[0] = 0;
  for (int i = 1; i < 256; i++)
  {
    HtWordType::statics.chrtypes[i] = 0;
    if (isalpha(i))
	HtWordType::statics.chrtypes[i] |= HtWt_Alpha;
    if (isdigit(i))
	HtWordType::statics.chrtypes[i] |= HtWt_Digit;
    if (strchr(extra_word_chars, i))
	HtWordType::statics.chrtypes[i] |= HtWt_Extra;
    if (strchr(valid_punct, i))
	HtWordType::statics.chrtypes[i] |= HtWt_ValidPunct;
  }
}


//  much like strtok(), and destructive of the source string like strtok(),
//  but does word separation by our rules.
char *
HtWordToken(char *str)
{
    unsigned char		*text = (unsigned char *)str;
    char			*ret = 0;
    static unsigned char	*prev = 0;

    if (!text)
	text = prev;
    while (text && *text && !HtIsStrictWordChar(*text))
	text++;
    if (text && *text)
    {
	ret = (char *)text;
	while (*text && HtIsWordChar(*text))
	    text++;
	if (*text)
	    *text++ = '\0';
    }
    prev = text;
    return ret;
}
