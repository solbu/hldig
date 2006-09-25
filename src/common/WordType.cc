//
// WordType.cc
//
// WordType:  Wrap some attributes to make is...() type
//            functions and other common functions without having to manage
//            the attributes or the exact attribute combination semantics.
//	      Configuration parameter used:
//            valid_punctuation,extra_word_characters,minimum_word_length,
//            maximum_word_length,allow_numbers,bad_word_list
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: WordType.cc,v 1.1.2.1 2006/09/25 23:50:31 aarnone Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */
#include "HtDebug.h"

#include <ctype.h>
#include <stdio.h>

#include "WordType.h"
#include "HtDebug.h"

WordType* WordType::instance = 0;

void 
WordType::Initialize(const Configuration &config_arg)
{
  if(instance != 0)
    delete instance;
  instance = new WordType(config_arg);
}

WordType::WordType(const Configuration &config)
{
  const String valid_punct = config["valid_punctuation"];
  const String extra_word_chars = config["extra_word_characters"];

  minimum_length = config.Value("minimum_word_length", 3);
  maximum_length = config.Value("maximum_word_length", 12);
  allow_numbers = config.Value("allow_numbers", 0);

  extra_word_characters = extra_word_chars;
  valid_punctuation = valid_punct;
  other_chars_in_word = extra_word_chars;
  other_chars_in_word.append(valid_punct);

  chrtypes[0] = 0;
  for (int i = 1; i < 256; i++)
  {
    chrtypes[i] = 0;
    if (isalpha(i))
	chrtypes[i] |= WORD_TYPE_ALPHA;
    if (isdigit(i))
	chrtypes[i] |= WORD_TYPE_DIGIT;
    if (iscntrl(i))
	chrtypes[i] |= WORD_TYPE_CONTROL;
    if (strchr(extra_word_chars, i))
	chrtypes[i] |= WORD_TYPE_EXTRA;
    if (strchr(valid_punct, i))
	chrtypes[i] |= WORD_TYPE_VALIDPUNCT;
  }

  {
    const String filename = config["bad_word_list"];
    FILE	*fl = fopen(filename, "r");
    char	buffer[1000];
    char	*word;
    String      new_word;

    // Read in the badwords file (it's just a text file)
    while (fl && fgets(buffer, sizeof(buffer), fl))
      {
	word = strtok(buffer, "\r\n \t");
	if (word && *word)
	  {
	    int flags;
	    new_word = word;
	    if((flags = Normalize(new_word)) & WORD_NORMALIZE_NOTOK) {
            HtDebug * debug = HtDebug::Instance();
            debug->outlog(4, "WordType::WordType: reading bad words from %s found %s, ignored because %s\n",
                    (const char*)filename, word, (char*)NormalizeStatus(flags & WORD_NORMALIZE_NOTOK));
	    } else {
	      badwords.Add(new_word, 0);
	    }
	  }
    }

    if (fl)
	fclose(fl);
  }
}

WordType::~WordType()
{
    HtDebug * debug = HtDebug::Instance();
    debug->outlog(10, "WordType destructor start\n");

    debug->outlog(10, "WordType destructor end\n");
}

//
// Normalize a word according to configuration specifications and 
// builting transformations. 
// *EVERY* word inserted in the inverted index goes thru this. If
// a word is rejected by Normalize there is 0% chance to find it
// in the word database.
//
int
WordType::Normalize(String& word) const
{
  int status = WORD_NORMALIZE_GOOD;

  //
  // Reject empty strings, always
  //
  if(word.empty())
    return status | WORD_NORMALIZE_NULL;

  //
  // Always convert to lowercase
  //
  if(word.lowercase())
    status |= WORD_NORMALIZE_CAPITAL;

  //
  // Remove punctuation characters according to configuration
  //
  if(StripPunctuation(word))
    status |= WORD_NORMALIZE_PUNCTUATION;

  //
  // Truncate words too long according to configuration
  //
  if(word.length() > maximum_length) {
    word.chop(word.length() - maximum_length);
    status |= WORD_NORMALIZE_TOOLONG;
  }

  //
  // Reject words too short according to configuration
  //
  if(word.length() < minimum_length)
    return status | WORD_NORMALIZE_TOOSHORT;

  //
  // Reject if contains control characters
  //
  int alpha = 0;
  for(const unsigned char *p = (const unsigned char*)(const char*)(char *)word; *p; p++) {
    if(IsStrictChar(*p) || (allow_numbers && IsDigit(*p))) {
      alpha = 1;
    } else if(IsControl(*p)) {
      return status | WORD_NORMALIZE_CONTROL;
    }
  }

  //
  // Reject if contains no alpha characters (according to configuration)
  //
  if(!alpha) return status | WORD_NORMALIZE_NOALPHA;

  //
  // Reject if listed in config[bad_word_list]
  //
  if(badwords.Exists(word))
    return status | WORD_NORMALIZE_BAD;

  //
  // Accept and report the transformations that occured
  //
  return status;
}

//
// Convert the integer status into a readable string
//
String
WordType::NormalizeStatus(int flags)
{
  String tmp;

  if(flags & WORD_NORMALIZE_TOOLONG) tmp << "TOOLONG ";
  if(flags & WORD_NORMALIZE_TOOSHORT) tmp << "TOOSHORT ";
  if(flags & WORD_NORMALIZE_CAPITAL) tmp << "CAPITAL ";
  if(flags & WORD_NORMALIZE_NUMBER) tmp << "NUMBER ";
  if(flags & WORD_NORMALIZE_CONTROL) tmp << "CONTROL ";
  if(flags & WORD_NORMALIZE_BAD) tmp << "BAD ";
  if(flags & WORD_NORMALIZE_NULL) tmp << "NULL ";
  if(flags & WORD_NORMALIZE_PUNCTUATION) tmp << "PUNCTUATION ";
  if(flags & WORD_NORMALIZE_NOALPHA) tmp << "NOALPHA ";

  if(tmp.empty()) tmp << "GOOD";

  return tmp;
}

//
// Non-destructive tokenizer using external int as pointer into String
//  does word separation by our rules (so it can be subclassed too)
//
String
WordType::WordToken(const String tokens, int &current) const
{
    unsigned char	text = tokens[current];
    String		ret;

    while (text && !IsStrictChar(text))
      text = tokens[++current];

    if (text)
    {
	while (text && IsChar(text))
	  {
	    ret << text;
	    text = tokens[++current];
	  }
    }
    return ret;
}
