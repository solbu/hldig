//
// HtWordCodec:  Given two lists of pair of "words" 'from' and 'to';
// simple one-to-one translations, use those lists to translate.
//  Only restriction are that no null (0) characters must be
// used in "words", and that there is a character "joiner" that
// does not appear in any word.  One-to-one consistency may be
// checked at construction.
//
// $Id: HtWordCodec.h,v 1.1 1999/01/21 13:43:03 ghutchis Exp $
//
// $Log: HtWordCodec.h,v $
// Revision 1.1  1999/01/21 13:43:03  ghutchis
// New files.
//
//
#ifndef __HtWordCodec_h
#define __HtWordCodec_h

#include <HtCodec.h>
#include "StringList.h"
#include "StringMatch.h"

class HtWordCodec : public HtCodec
{
public:
  HtWordCodec();
  virtual ~HtWordCodec();

  // Set the lists of asymmetric pairs of "words" in "from" and
  // "to", using:
  // * one list of requested encodings with two consecutive
  //   items "to" and "from" per translation
  // * one list of just words which HtWordCodec will generate
  //   space-saving encodings for.
  // Either may be empty.
  //  Items in frequent_substrings will be silently ignored if
  // they collide with anything in requested_encoding_pairs.
  // CodingError is empty on success, or has a failure message.
  HtWordCodec(StringList &requested_encodings,
              StringList &frequest_substrings, String &errmsg);

  // *Or*, set the lists directly, without checking coding
  // consistency.  HtWordCodec will delete these lists when
  // destroyed.  Not really recommended, but this class would be
  // incomplete without it.
  HtWordCodec (StringList *from, StringList *to, char joiner = char(1));

  // Same as those in the parent class.  Each string to
  // encode/decode may contain zero or more of words from the
  // lists.  Those words will be replaced.
  virtual String encode(const String &uncoded) const;
  virtual String decode(const String &coded) const;

private:
  HtWordCodec(const HtWordCodec &); // Not supposed to be implemented.
  void operator= (const HtWordCodec &); // Not supposed to be implemented.
  StringList *myFrom;
  StringList *myTo;
  StringMatch *myFromMatch;
  StringMatch *myToMatch;

  // Do coding/decoding symmetrically using the provided lookup and lists.
  String code(const String &, StringMatch& match, StringList& replacements) const;
};

#endif /* __HtWordCodec_h */
