//
// HtWordType.cc
//
// HtWordType:  Wrap some attributes to make is...() type
//              functions and other common functions without having to manage
//              the attributes or the exact attribute combination semantics.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtWordType.cc,v 1.5 1999/09/24 10:29:03 loic Exp $
//

#include "HtWordType.h"

HtWordType::InnerStatics HtWordType::statics;

// Must only be called once (no tests, though).
void
HtWordType::Initialize(Configuration &config)
{
  const String valid_punct = config["valid_punctuation"];
  const String extra_word_chars = config["extra_word_characters"];

  HtWordType::statics.extra_word_characters = extra_word_chars;
  HtWordType::statics.valid_punctuation = valid_punct;
  HtWordType::statics.other_chars_in_word = extra_word_chars;
  HtWordType::statics.other_chars_in_word.append(valid_punct);
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
