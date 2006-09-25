//
// WordType.h
//
// WordType:  Wrap some attributes to make is...() type
//              functions and other common functions without having to manage
//              the attributes or the exact attribute combination semantics.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: WordType.h,v 1.1.2.1 2006/09/25 23:50:31 aarnone Exp $
//

#ifndef _WordType_h
#define _WordType_h

#include "htString.h"
#include "Configuration.h"
//
// Return values of Normalize, to get them in string form use NormalizeStatus
//
#define WORD_NORMALIZE_GOOD		0x0000
#define WORD_NORMALIZE_TOOLONG		0x0001
#define WORD_NORMALIZE_TOOSHORT		0x0002
#define WORD_NORMALIZE_CAPITAL		0x0004
#define WORD_NORMALIZE_NUMBER		0x0008
#define WORD_NORMALIZE_CONTROL		0x0010
#define WORD_NORMALIZE_BAD		0x0020
#define WORD_NORMALIZE_NULL		0x0040
#define WORD_NORMALIZE_PUNCTUATION	0x0080
#define WORD_NORMALIZE_NOALPHA		0x0100

//
// Under these conditions the word is said to be invalid.
// Some conditions (NUMBER,TOOSHORT and BAD) depends on the configuration
// parameters.
//
#define WORD_NORMALIZE_NOTOK		(WORD_NORMALIZE_TOOSHORT| \
					 WORD_NORMALIZE_NUMBER| \
					 WORD_NORMALIZE_CONTROL| \
					 WORD_NORMALIZE_BAD| \
					 WORD_NORMALIZE_NULL| \
					 WORD_NORMALIZE_NOALPHA)
					 
class WordType
{
public:
  //
  // Constructors
  //
  WordType(const Configuration& config);

  //
  // Destructor
  //
  virtual	~WordType();

  //
  // Unique instance handlers 
  //
  static void Initialize(const Configuration& config);
  static WordType* Instance() {
    if(instance) return instance;
    fprintf(stderr, "WordType::Instance: no instance\n");
    return 0;
  }
  
  //
  // Predicates
  // 
  virtual int IsChar(int c) const;
  virtual int IsStrictChar(int c) const;
  virtual int IsDigit(int c) const;
  virtual int IsControl(int c) const;

  //
  // Transformations
  //
  virtual int StripPunctuation(String &s) const;
  virtual int Normalize(String &s) const;

  //
  // Splitting
  //
  virtual String WordToken(const String s, int &pointer) const;

  //
  // Error handling
  //
  static String NormalizeStatus(int flags);

private:

  String		valid_punctuation;     // The same as the attribute.
  String		extra_word_characters; // Likewise.
  String		other_chars_in_word;   // Attribute "valid_punctuation" plus
  // "extra_word_characters".
  char			chrtypes[256];          // quick lookup table for types
  int			minimum_length;		// Minimum word length
  int			maximum_length;		// Maximum word length
  int			allow_numbers;		// True if a word may contain numbers
  Dictionary		badwords;		// List of excluded words

  //
  // Unique instance pointer
  //
  static WordType* instance;
};

// Bits to set in chrtypes[]:
#define WORD_TYPE_ALPHA	0x01
#define WORD_TYPE_DIGIT	0x02
#define WORD_TYPE_EXTRA	0x04
#define WORD_TYPE_VALIDPUNCT	0x08
#define WORD_TYPE_CONTROL	0x10

// One for characters that when put together are a word
// (including punctuation).
inline int
WordType::IsChar(int c) const
{
  return (chrtypes[(unsigned char)c] & (WORD_TYPE_ALPHA|WORD_TYPE_DIGIT|WORD_TYPE_EXTRA|WORD_TYPE_VALIDPUNCT)) != 0;
}

// Similar, but no punctuation characters.
inline int
WordType::IsStrictChar(int c) const
{
  return (chrtypes[(unsigned char)c] & (WORD_TYPE_ALPHA|WORD_TYPE_DIGIT|WORD_TYPE_EXTRA)) != 0;
}

// Reimplementation of isdigit() using the lookup table chrtypes[] 
inline int
WordType::IsDigit(int c) const
{
  return (chrtypes[(unsigned char)c] & WORD_TYPE_DIGIT) != 0;
}

// Similar to IsDigit, but for iscntrl()
inline int
WordType::IsControl(int c) const
{
  return (chrtypes[(unsigned char)c] & WORD_TYPE_CONTROL) != 0;
}

// Let caller get rid of getting and holding a configuration parameter.
inline int
WordType::StripPunctuation(String &s) const
{
  return s.remove(valid_punctuation);
}


#endif /* __WordType_h */
