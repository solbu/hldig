//
// search.cc
//
// search: Sample implementation of search algorithms using
//         a mifluz inverted index. 
//
//         Each class is documented in the class definition. Before
//         each method declaration a comment explains the semantic of
//         the method. In the method definition comments in the code
//         may contain additional information.
//
//         Each virtual function is documented in the base class, not
//         in the derived classes except for semantic differences.
//         
//         The class tree is:
//         
//         WordKeySemantic
//
//         WordExclude
//           WordExcludeMask
//             WordPermute
//
//         WordSearch
//
//         WordMatch
//
//         WordTree
//           WordTreeOperand
//             WordTreeOptional
//              WordTreeOr
//              WordTreeAnd
//              WordTreeNear
//             WordTreeMandatory
//             WordTreeNot
//           WordTreeLiteral
//
//         WordParser
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: search.cc,v 1.5 2003/05/27 12:51:27 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include <htconfig.h>
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

// If we have this, we probably want it.
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif /* HAVE_GETOPT_H */
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif /* HAVE_MALLOC_H */
#include <stdlib.h>

#include <htString.h>
#include <WordList.h>
#include <WordContext.h>
#include <WordCursor.h>

//
// Verbosity level set with -v (++)
// 
static int verbose = 0;

// ************************* Document definition implementation ***********

#define TAG		1
#define SERVER		2
#define URL		3
#define LOCATION	4

// *********************** WordKeySemantic implementation ********************
//
// NAME
//
// encapsulate WordKey semantic for document and location
//
// SYNOPSIS
//
// #include <WordKeySemantic.h>
//
// #define SERVER 1
// #define URL 2
// #define LOCATION 3
//
// static int document[] = {
//   SERVER,
//   URL
// };
// 
// WordKeySemantic semantic;
// semantic.Initialize(document, sizeof(document)/sizeof(int), LOCATION);
//
// DESCRIPTION
//
// Encapsulate the semantic of a WordKey object fields. It defines
// what a document and a location are. It implements the set of
// operation that a search needs to perform given the fact that it
// implements a search whose purpose is to retrieve a document and
// wants to implement proximity search based on a word location.
//
//
// END
//
// A document is a set of fields in a given order. 
// A location is a field.
// The actual fields used to implement WordKeySemantic methods are
// set with the Initialize method.
//
class WordKeySemantic {
public:
  WordKeySemantic();
  ~WordKeySemantic();

  //-
  // Set the actual field numbers that define what a document is and
  // what a location is. The <b>document_arg<b> is a list of WordKey field
  // positions of length <b>document_length_arg</b> that must be adjacent.
  // The <b>location_arg</b> is the WordKey field position of the word
  // location within a document.
  // Return OK on success, NOTOK on failure.
  //
  int Initialize(int* document_arg, int document_length_arg, int location_arg);

  //
  // These functions and only these know what a document is. 
  // This should really be a class containing function pointers and be
  // given as argument to the search algorithm.
  //
  //-
  // Copy the document in <b>from</b> into <b>to.</b>
  //
  void DocumentSet(const WordKey& from, WordKey& to);
  //-
  // Increment the document in <b>key</b> using the <i>SetToFollowing</i>
  // method of WordKey. <b>uniq</b> is the WordKey position at which the 
  // increment starts.
  //
  void DocumentNext(WordKey& key, int uniq);
  //-
  // Compare the document fields defined in both <b>a</b> and <b>b</b>
  // and return the difference a - b, as in strcmp. If all document
  // fields in <b>a</b> or <b>b</b> are undefined return 1.
  //
  int DocumentCompare(const WordKey& a, const WordKey& b);
  //-
  // Set all document fields to 0.
  //
  int DocumentClear(WordKey& key);

  //
  // These functions and only these know what a location is. 
  // This should really be a class containing function pointers and be
  // given as argument to the search algorithm.
  //
  //-
  // Copy the document and location in <b>from</b> into <b>to.</b>
  //
  void LocationSet(const WordKey& from, WordKey& to);
  //-
  // Increment the document and location in <b>key</b> 
  // using the <i>SetToFollowing</i>
  // method of WordKey. 
  //
  void LocationNext(WordKey& key);
  //-
  // Compare <b>expected</b> location to <b>actual</b> location. Compares equal
  // as long as expected location is at a maximum distance of <b>proximity</b>
  // of actual. If <b>actual</b> only has undefined field, return > 0.
  // <b>expected</b> must always be the lowest possible bound.
  // <b>actual</b> is tolerated if it is greater than <b>actual</b> but not
  // greater than <b>proximity</b> if <b>proximity</b> > 0 or abs(<b>proximity</b>) * 2 if
  // <b>proximity</b> < 0.
  // Return the difference expected - actual.
  //
  int  LocationCompare(const WordKey& expected, const WordKey& actual, int proximity = 0);
  //-
  // <b>key</b> is the expected location of a searched key. 
  // LocationNearLowest modifies <b>key</b> to add tolerance accroding to
  // <b>proximity</b>. 
  //
  // The idea is that <b>key</b> will be the lowest possible match for 
  // for the <b>proximity</b> range. If <proxmity> is positive, <b>key</b>
  // is already the lowest possible match since we accept [0 proximity].
  // If <b>proximity</b> is negative, substract it since we accept
  // [-proximity proximity].
  //
  // For better understanding see the functions in which it is used.
  //
  void LocationNearLowest(WordKey& key, int proximity);

  //-
  // Undefined the location field in <b>key.</b>.
  //
  void Location2Document(WordKey& key);

protected:
  int* document;
  int document_length;
  int location;
};

WordKeySemantic::WordKeySemantic()
{
  int nfields = WordKey::NFields();
  document = new int[nfields];
  document_length = 0;
  location = -1;
}

WordKeySemantic::~WordKeySemantic()
{
  if(document) delete [] document;
}

int WordKeySemantic::Initialize(int* document_arg, int document_length_arg, int location_arg)
{
  memcpy((char*)document, (char*)document_arg, document_length_arg * sizeof(int));
  document_length = document_length_arg;
  location = location_arg;
  return OK;
}

void WordKeySemantic::DocumentSet(const WordKey& from, WordKey& to)
{
  to.Clear();
  for(int i = 0; i < document_length; i++)
    to.Set(document[i], from.Get(document[i]));
}

int WordKeySemantic::DocumentCompare(const WordKey& a, const WordKey& b)
{
  int ret = 1;
  for(int i = 0; i < document_length; i++) {
    int idx = document[i];
    if((a.IsDefined(idx) && b.IsDefined(idx)) &&
       (ret = a.Get(idx) - b.Get(idx)) != 0) return ret;
  }
  return ret;
}

int WordKeySemantic::DocumentClear(WordKey& key)
{
  for(int i = 0; i < document_length; i++)
    key.Set(document[i], 0);
  return 0;
}

void WordKeySemantic::DocumentNext(WordKey& key, int uniq)
{
  if(uniq)
    key.SetToFollowing(uniq);
  else
    key.SetToFollowing(document[document_length-1]);
}


void WordKeySemantic::LocationSet(const WordKey& from, WordKey& to)
{
  DocumentSet(from, to);
  to.Set(location, from.Get(location));
}

int WordKeySemantic::LocationCompare(const WordKey& expected, const WordKey& actual, int proximity)
{
  int ret = 1;
  if((ret = DocumentCompare(expected, actual)) != 0) return ret;
  //
  // Only compare location if defined.
  //
  if((expected.IsDefined(location) && actual.IsDefined(location)) &&
     (ret = expected.Get(location) - actual.Get(location))) {
    if(proximity < 0) {
      //
      // -N means ok if in range [-N +N]
      //
      proximity *= 2;
      if(ret < 0 && ret >= proximity)
	ret = 0;
    } else {
      //
      // N means ok if in range [0 +N]
      //
      if(ret < 0 && ret >= -proximity)
	ret = 0;
    }
  }
  return ret;
}

void WordKeySemantic::LocationNext(WordKey& key)
{
  key.SetToFollowing(location);
}

void WordKeySemantic::LocationNearLowest(WordKey& key, int proximity)
{
  if(proximity < 0) {
    if(key.Underflow(location, proximity))
      key.Get(location) = 0;
    else
      key.Get(location) += proximity;
  }
}

void WordKeySemantic::Location2Document(WordKey& key)
{
  key.Undefined(location);
}

// ************************* WordExclude implementation ********************
//
// NAME
// 
// permute bits in bit field
//
// SYNOPSIS
//
// #include <WordExclude.h>
//
// #define BITS 5
//
// WordExclude permute;
// permute.Initialize(BITS);
// while(permute.Next() == WORD_EXCLUDE_OK)
//    ...
//
// DESCRIPTION
//
// Count from 1 to the specified maximum. A variable++ loop does the same.
// The <b>WordExclude</b> class counts in a specific order.
// It first step thru all the permutations containing only 1 bit set, in
// increasing order. Then thru all the permutations containing 2 bits set,
// in increasing order. As so forth until the maximum number is reached.
// See the <b>Permute</b> method for more information.
//
//
// END

//
// Helper that displays an unsigned int in binary/hexa/decimal
//
static inline void show_bits(unsigned int result)
{
  int i;
  for(i = 0; i < 10; i++) {
    fprintf(stderr, "%c", (result & (1 << i)) ? '1' : '0');
  }
  fprintf(stderr, " (0x%08x - %15d)\n", result, result);
}

//
// WordExclude methods return values
//
#define WORD_EXCLUDE_OK		1
#define WORD_EXCLUDE_END	2

//
// Maximum number of bits
//
#define WORD_EXCLUDE_MAX	(sizeof(unsigned int) * 8)

//
// Convert a position <p> in a <l> bits mask into a bit offset (from 0)
//
#define WORD_EXCLUDE_POSITION2BIT(l,p) ((l) - (p) - 1)

class WordExclude {
public:
  //-
  // Reset the generator and prepare it for <b>length</b> bits generation.
  // The <b>length</b> cannot be greater than <i>WORD_EXCLUDE_MAX.</i>
  // Returns OK if no error occurs, NOTOK otherwise.
  //
  virtual int Initialize(unsigned int length);
  //-
  // Move to next exclude mask. Returns WORD_EXCLUDE_OK if successfull,
  // WORD_EXCLUDE_END if at the end of the permutations. It starts by
  // calling <i>Permute</i> with one bit set, then two and up to 
  // <i>Maxi()</i> included. The last permutation only generates one
  // possibility since all the bits are set.
  //
  virtual int Next();
  //-
  // Exclude bit for <b>position</b> starts at most significant bit. That is
  // position 0 exclude bit is most significant bit of the current mask.
  // Returns true if position is excluded, false otherwise.
  //
  virtual inline unsigned int Excluded(int position) { return mask & (1 << WORD_EXCLUDE_POSITION2BIT(maxi, position)); }
  //-
  // Returns how many bits are not excluded with current mask.
  //
  virtual inline int NotExcludedCount() const { return maxi - bits; }
  //-
  // Returns how many bits are excluded with current mask.
  //
  virtual inline int ExcludedCount() const { return bits; }
  //
  // Save and restore in string
  //
  //-
  // Write an ascii representation of the WordExclude object in <b>buffer.</b>
  // Each bit is represented by the character 0 or 1. The most significant
  // bit is the last character in the string. For instance
  // 1000 is the string representation of a WordExclude object initialized
  // with length = 4 after the first <i>Next</i> operation.
  //
  virtual void Get(String& buffer) const;
  //-
  // Initialize the object from the string representation in <b>buffer.</b>
  // Returns OK on success, NOTOK on failure.
  //
  virtual int Set(const String& buffer);

  //-
  // Generate all the permutations
  // containing <i>n</i> bits in a <b>bits</b> bit word in increasing order.
  // The <b>mask</b> argument is originally filled by the caller
  // with the <i>n</i> least significant bits set. A call to Permute
  // generates the next permutation immediately greater (numerically)
  // than the one contained in <b>mask</b>.
  // 
  // Permute returns the next permutation or 0 if it reached the
  // maximum.
  //
  // To understand the algorithm, imagine 1 is a ball and 0 a space.
  // 
  // When playing the game you start with a rack of <b>bits</b> slots filled
  // with <i>n</i> balls all on the left side. You end the game when all
  // the balls are on the right side.
  //
  // Sarting from the left, search for the first ball that has an empty
  // space to the right. While searching remove all the balls you find.
  // Place a ball in the empty space you found, at the right of the last
  // ball removed. Sarting from the left, fill all empty spaces with
  // the removed balls. Repeat until all balls are to the right.
  // 
  // Here is a sample generated by repeated calls to WordExclude::Permute:
  // (left most bit is least significant)
  // <pre>
  // mask = 1111100000 
  // while(mask = WordExclude::Permute(mask, 7))
  //    show_bits(mask)
  //
  // 1111100000 (0x0000001f -              31)
  // 1111010000 (0x0000002f -              47)
  // 1110110000 (0x00000037 -              55)
  // 1101110000 (0x0000003b -              59)
  // 1011110000 (0x0000003d -              61)
  // 0111110000 (0x0000003e -              62)
  // 1111001000 (0x0000004f -              79)
  // 1110101000 (0x00000057 -              87)
  // 1101101000 (0x0000005b -              91)
  // 1011101000 (0x0000005d -              93)
  // 0111101000 (0x0000005e -              94)
  // 1110011000 (0x00000067 -             103)
  // 1101011000 (0x0000006b -             107)
  // 1011011000 (0x0000006d -             109)
  // 0111011000 (0x0000006e -             110)
  // 1100111000 (0x00000073 -             115)
  // 1010111000 (0x00000075 -             117)
  // 0110111000 (0x00000076 -             118)
  // 1001111000 (0x00000079 -             121)
  // 0101111000 (0x0000007a -             122)
  // 0011111000 (0x0000007c -             124)
  // </pre>
  // A recursive implementation would be:
  // <pre>
  // /* Recursive */
  // void permute(unsigned int result, int bits_count, int bits_toset)
  // {
  //  if(bits_toset <= 0 || bits_count <= 0) {
  //    if(bits_toset <= 0)
  //      do_something(result);
  //  } else {
  //    permute(result, bits_count - 1, bits_toset);
  //    permute(result | (1 << (bits_count - 1)), bits_count - 1, bits_toset - 1);
  //  }
  // }
  // </pre>
  // Which is more elegant but not practical at all in our case. 
  //
  inline unsigned int Permute(unsigned int mask, unsigned int bits);
  
  //-
  // Return the current bit field value.
  //
  virtual inline unsigned int& Mask() { return mask; }
  virtual inline unsigned int Mask() const { return mask; }

  virtual inline unsigned int& Maxi() { return maxi; }
  virtual inline unsigned int Maxi() const { return maxi; }

  virtual inline unsigned int& Bits() { return bits; }
  virtual inline unsigned int Bits() const { return bits; }

private:
  unsigned int mask;
  unsigned int maxi;
  unsigned int bits;
};

int WordExclude::Initialize(unsigned int length)
{
  if(length > WORD_EXCLUDE_MAX) {
    fprintf(stderr, "WordExclude::Initialize: length must be < %d\n", (int)WORD_EXCLUDE_MAX);
    return NOTOK;
  }

  mask = 0;
  bits = 0;
  maxi = length;

  return OK;
}

inline unsigned int WordExclude::Permute(unsigned int mask, unsigned int bits)
{
  unsigned int bits_cleared = 0;
  unsigned int j;
  for(j = 0; j < bits; j++) {
    if(mask & (1 << j)) {
      bits_cleared++;
      mask &= ~(1 << j);
    } else {
      if(bits_cleared) {
	bits_cleared--;
	mask |= (1 << j);
	break;
      }
    }
  }
    
  if(j >= bits)
    return 0;

  for(j = 0; j < bits_cleared; j++)
    mask |= (1 << j);

  return mask;
}

int WordExclude::Next()
{
  mask = Permute(mask, maxi);

  int ret = WORD_EXCLUDE_OK;

  if(mask == 0) {
    bits++;
    if(bits > maxi)
      ret = WORD_EXCLUDE_END;
    else {
      unsigned int i;
      for(i = 0; i < bits; i++)
	mask |= (1 << i);
      ret = WORD_EXCLUDE_OK;
    }
  }
  
  if(verbose > 2) show_bits(mask);

  return ret;
}

void WordExclude::Get(String& buffer) const
{
  buffer.trunc();
  unsigned int i;
  for(i = 0; i < maxi; i++) {
    buffer << ((mask & (1 << i)) ? '1' : '0');
  }
}

int WordExclude::Set(const String& buffer)
{
  if(Initialize(buffer.length()) == NOTOK)
    return NOTOK;
  unsigned int i;
  for(i = 0; i < maxi; i++) {
    if(buffer[i] == '1') {
      mask |= (1 << i);
      bits++;
    }
  }
  return OK;
}

// ************************* WordExcludeMask implementation *******************
//
// NAME
//
// WordExclude specialization that ignore some bits
//
// SYNOPSIS
//
// #include <WordExcludeMask.h>
//
// #define BITS              9
// #define IGNORE        0x0f0
// #define IGNORE_MASK   0x050
//
// WordExcludeMask permute;
// permute.Initialize(BITS, IGNORE, IGNORE_MASK);
// while(permute.Next() == WORD_EXCLUDE_OK)
//    ...
//
// DESCRIPTION
//
// Only perform WordExclude operations on the bits that are not set in
// <i>ignore.</i> The bits of <i>ignore_mask</i> that are set in
// <i>ignore</i> are untouched. In the synopsis section, for instance,
// bits 1,2,3,4 and 9 will be permuted and the bits 5,6,7,8 will be
// left untouched.
// 
//
// END
//
#define WORD_EXCLUDE_IGNORED	(-1)

class WordExcludeMask : public WordExclude {
public:
  //-
  // <b>ignore</b> gives the mask of bits to ignore. The actual WordExclude
  // operations are made on a number of bits that is <b>length</b> - (the number
  // of bits set in <b>ignore).</b>
  // The <b>ignore_mask_arg</b> contains the actual values of the bits ignored by 
  // the <b>ignore</b> argument.
  //
  virtual inline int Initialize(unsigned int length, unsigned int ignore, unsigned int ignore_mask_arg) {
    ignore_mask = ignore_mask_arg;
    ignore_maxi = length;
    unsigned int maxi = 0;
    unsigned int i;
    for(i = 0, ignore_bits = 0; i < length; i++) {
      if(ignore & (1 << i)) {
	bit2bit[i] = WORD_EXCLUDE_IGNORED;
	if(ignore_mask & (1 << i)) ignore_bits++;
      } else {
	bit2bit[i] = maxi++;
      }
    }

    return WordExclude::Initialize(maxi);
  }

  virtual inline unsigned int Excluded(int position) {
    position = WORD_EXCLUDE_POSITION2BIT(ignore_maxi, position);
    if(bit2bit[position] == WORD_EXCLUDE_IGNORED)
      return ignore_mask & (1 << position);
    else
      return WordExclude::Mask() & (1 << bit2bit[position]);
  }

  virtual inline int NotExcludedCount() const {
    return ignore_maxi - ignore_bits - WordExclude::Bits();
  }

  virtual inline int ExcludedCount() const {
    return ignore_bits - WordExclude::Bits();
  }

  //-
  // The semantic is the same as the Get method of Wordexclude
  // except that ignored bits are assigned 3 and 2 instead of 1 and 0
  // respectively.
  //
  virtual void Get(String& buffer) const;
  //-
  // The semantic is the same as the Get method of Wordexclude
  // except that ignored bits are assigned 3 and 2 instead of 1 and 0
  // respectively.
  //
  virtual int Set(const String& buffer);

  virtual inline unsigned int Mask() const {
    unsigned int ret = ignore_mask;
    unsigned int i;
    for(i = 0; i < ignore_maxi; i++) {
      if(bit2bit[i] != WORD_EXCLUDE_IGNORED) {
	if(WordExclude::Mask() & (1 << bit2bit[i]))
	  ret |= (1 << i);
      }
    }
    return ret;
  }

  virtual inline unsigned int Maxi() const { return ignore_maxi; }

  virtual inline unsigned int Bits() const { return ignore_bits + WordExclude::Bits(); }

private:
  unsigned int ignore_mask;
  unsigned int ignore_maxi;
  unsigned int ignore_bits;
  int bit2bit[WORD_EXCLUDE_MAX];
};

void WordExcludeMask::Get(String& buffer) const
{
  buffer.trunc();
  unsigned int i;
  for(i = 0; i < ignore_maxi; i++) {
    if(bit2bit[i] == WORD_EXCLUDE_IGNORED)
      buffer << ((ignore_mask & (1 << i)) ? '3' : '2');
    else
      buffer << ((WordExclude::Mask() & (1 << bit2bit[i])) ? '1' : '0');
  }
}

int WordExcludeMask::Set(const String& buffer)
{
  WordExclude::Initialize(0);

  unsigned int& maxi = WordExclude::Maxi();
  unsigned int& mask = WordExclude::Mask();
  unsigned int& bits = WordExclude::Bits();
  ignore_mask = 0;
  ignore_bits = 0;
  ignore_maxi = buffer.length();

  unsigned int i;
  for(i = 0; i < ignore_maxi; i++) {
    if(buffer[i] == '1' || buffer[i] == '0') {
      if(buffer[i] == '1') {
	mask |= (1 << maxi);
	bits++;
      }
      bit2bit[i] = maxi;
      maxi++;
    } else if(buffer[i] == '3' || buffer[i] == '2') {
      if(buffer[i] == '3') {
	ignore_mask |= (1 << i);
	ignore_bits++;
      }
      bit2bit[i] = WORD_EXCLUDE_IGNORED;
    }
  }

  return OK;
}

// ************************* WordPermute implementation ********************
//
// NAME
//
// WordExclude specialization with proximity toggle
//
// SYNOPSIS
//
// #include <WordPermute.h>
//
// #define BITS 5
//
// WordPermute permute;
// permute.Initialize(BITS);
// while(permute.Next() == WORD_EXCLUDE_OK)
//    if(permute.UseProximity()) ...
//
// DESCRIPTION
//
// Each WordExclude permutation is used twice by Next. Once with
// the proximity flag set and once with the proximity flag cleared.
// If the length of the bit field (length argument of Initialize) is
// lower or equal to 1, then the proximity flag is always false.
//
//
// END
//
// WordPermute methods return values
//
#define WORD_PERMUTE_OK		WORD_EXCLUDE_OK
#define WORD_PERMUTE_END	WORD_EXCLUDE_END

//
// Use or don't use proximity flag
//
#define WORD_PERMUTE_PROXIMITY_NO	0
#define WORD_PERMUTE_PROXIMITY_TOGGLE	1
#define WORD_PERMUTE_PROXIMITY_ONLY	2

//
// Deals with word exclusion and proximity permutations for
// the implementation of the Optional retrieval model.
//
class WordPermute : public WordExcludeMask {
public:
  //-
  // The <b>nuse_proximity</b> may be set to the following:
  //
  // WORD_PERMUTE_PROXIMITY_NO so that the object behaves as
  // WordExcludeMask and Proximity() always return false.
  //
  // WORD_PERMUTE_PROXIMITY_TOGGLE so that each permutation is issued twice: 
  // once with the proximity flag set (Proximity() method) and once with
  // the proximity flag cleared. 
  //
  // WORD_PERMUTE_PROXIMITY_ONLY so that the object behaves as
  // WordExcludeMask and Proximity() always return true.
  //
  virtual inline int Initialize(unsigned int length, unsigned int ignore, unsigned int ignore_mask_arg, int nuse_proximity) {
    use_proximity = nuse_proximity;
    switch(use_proximity) {
    case WORD_PERMUTE_PROXIMITY_NO:
      proximity = 0;
      break;
    case WORD_PERMUTE_PROXIMITY_TOGGLE:
      //
      // Don't bother to try proximity search if only one word
      // is involved.
      //
      proximity = length > 1;
      break;
    case WORD_PERMUTE_PROXIMITY_ONLY:
      proximity = 1;
      break;
    default:
      fprintf(stderr, "WordPermute::Initialize: unexpected use_proximity = %d\n", use_proximity);
      return 0;
    }
    return WordExcludeMask::Initialize(length, ignore, ignore_mask_arg);
  }

  //-
  // Return true if the proximity flag is set, false if it is 
  // cleared.
  //
  inline int Proximity() {
    switch(use_proximity) {
    case WORD_PERMUTE_PROXIMITY_NO:
      return 0;
      break;
    case WORD_PERMUTE_PROXIMITY_TOGGLE:
      return proximity;
      break;
    case WORD_PERMUTE_PROXIMITY_ONLY:
      return 1;
      break;
    default:
      fprintf(stderr, "WordPermute::Proximity: unexpected use_proximity = %d\n", use_proximity);
      return 0;
      break;
    }
  }

  //-
  // Return WORD_PERMUTE_PROXIMITY_NO, WORD_PERMUTE_PROXIMITY_TOGGLE or
  // WORD_PERMUTE_PROXIMITY_ONLY.
  //
  inline int UseProximity() { return use_proximity; }

  //-
  // Find the next permutation. If <b>WORD_PERMUTE_PROXIMITY_TOGGLE<b> was
  // specified in Initialize each permutation is issued twice (see
  // Proximity() to differentiate them), except when the mask 
  // only contains one non exluded bit (NotExcludeCount() <= 1).
  // In both case the last permutation with all bits excluded
  // (i.e. when NotExcludedCount() <= 0) is never returned because 
  // it is useless.
  // 
  virtual int Next() {
    if(Maxi() <= 0)
      return WORD_PERMUTE_END;

    int ret = WORD_PERMUTE_OK;
    int check_useless = 0;
    if(use_proximity == WORD_PERMUTE_PROXIMITY_TOGGLE) {
      //
      // Move to next permutation as follows: 
      // exclude mask 1 + use proximity
      // exclude mask 1 + don't use proximity
      // exclude mask 2 + use proximity 
      // exclude mask 2 + don't use proximity
      // and so on.
      // If only one word is involved never use proximity.
      //
      if(proximity) {
	proximity = 0;
      } else {
	proximity = 1;
	if((ret = WordExcludeMask::Next()) == WORD_PERMUTE_OK) {
	  //
	  // Do not toggle proximity for only one non excluded word
	  //
	  if(NotExcludedCount() <= 1)
	    proximity = 0;
	  check_useless = 1;
	} else if(ret == WORD_PERMUTE_END)
	  proximity = 0;
      }
    } else {
      ret = WordExcludeMask::Next();
      check_useless = 1;
    }

    if(check_useless && ret == WORD_PERMUTE_OK) {
      //
      // If no bits are ignored or all ignore_mask bits are set to
      // one, the last permutation has all exclude bits set, which
      // is useless. Just skip it and expect to be at the end of
      // all permutations.
      //
      if(NotExcludedCount() <= 0) {
	ret = WordExcludeMask::Next();
	if(ret != WORD_PERMUTE_END) {
	  fprintf(stderr, "WordPermute::Next: expected WORD_PERMUTE_END\n");
	  ret = NOTOK;
	}
      }
    }

    return ret;
  }

  //-
  // The semantic is the same as the Get method of Wordexclude
  // but a letter T is appended to the string if the proximity
  // flag is set, or F is appended to the string if the proximity
  // is clear.
  //
  virtual inline void Get(String& buffer) const {
    WordExcludeMask::Get(buffer);
    if(use_proximity == WORD_PERMUTE_PROXIMITY_TOGGLE)
      buffer << (proximity ? 'T' : 'F');
  }

  //-
  // The semantic is the same as the Get method of Wordexclude
  // but if the string end with a T the proximity flag is set
  // and if the string end with a F the proximity flag is cleared.
  //
  virtual inline int Set(const String& buffer) {
    if(buffer.length() < 1) {
      fprintf(stderr, "WordPermute::Set: buffer length < 1\n");
      return NOTOK;
    }
    int ret = OK;
    if(use_proximity == WORD_PERMUTE_PROXIMITY_TOGGLE) {
      if((ret = WordExcludeMask::Set(buffer.sub(0, buffer.length() - 1))) == OK)
	proximity = buffer.last() == 'T';
    } else {
      ret = WordExcludeMask::Set(buffer);
    }

    return ret;
  }

protected:
  int use_proximity;
  int proximity;
};

// ************************* WordTree implementation ********************
//
// NAME
// 
// Base class for query resolution nodes
//
// SYNOPSIS
//
// #include <WordTree.h>
//
// class WordTreeMethod : public WordTree {
// ...
// };
//
// DESCRIPTION
//
// The WordTree class is derived from the WordCursor class and implement
// the basic operations and data structures needed for query resolution.
// It is the common base class of all the classes that actually implement
// a query resolution. The derived classes must be implemented to follow
// the WordCursor semantic for Walk* operations.
//
// 
// END
// 

#define WORD_WALK_REDO		0x1000
#define WORD_WALK_RESTART	0x2000
#define WORD_WALK_NEXT		0x4000

//
// Return values of CursorsObeyProximity method
//
#define WORD_SEARCH_NOPROXIMITY	1

//
// operand values
//
#define WORD_TREE_OR		1
#define WORD_TREE_AND		2
#define WORD_TREE_NEAR		3
#define WORD_TREE_OPTIONAL	4
#define WORD_TREE_LITERAL	5
#define WORD_TREE_MANDATORY	6
#define WORD_TREE_NOT		7

#define WORD_TREE_OP_SIZE	20

//
// Default proximity is to search for adjacent words in order
//
#ifndef WORD_SEARCH_DEFAULT_PROXIMITY
#define WORD_SEARCH_DEFAULT_PROXIMITY 1
#endif /* WORD_SEARCH_DEFAULT_PROXIMITY */

static char* operator_name[WORD_TREE_OP_SIZE] = {
  "",
  "or",
  "and",
  "near",
  "optional",
  "literal",
  "mandatory",
  "not",
  0
};

class WordTree : public WordCursor {
public:
  WordTree() {
    proximity = 0;
    uniq = 0;
  }

  virtual int ContextSaveList(StringList& list) const {
    return OK;
  }

  virtual int ContextRestoreList(StringList& list) {
    return OK;
  }

  //-
  // Initialize the object. <b>words</b> is used to initialize the 
  // WordCursor base class, <b>document, document_length</b> and 
  // <b>location</b> are used to initialize the WordKeySemantic data
  // member. The <b>nuniq</b> is the WordKey field position used by
  // the WordKeySemantic::DocumentNext function. The <b>nproximity</b>
  // is the proximity factor used by the WordKeySemantic::LocationCompare
  // method.
  // Return OK on success, NOTOK on failure.
  //
  virtual int Prepare(WordList *words, int nuniq, int nproximity, int *document, int document_length, int location) {
    int ret;
    proximity = nproximity;
    uniq = nuniq;
    if((ret = key_semantic.Initialize(document, document_length, location)) != OK)
      return ret;
    WordKey key;
    if(!scope.empty()) {
      if(key.Set(scope) != OK) {
	fprintf(stderr, "WordTree::Prepare: setting scope %s failed\n", (char*)scope);
	return NOTOK;
      }
    }
    key.SetWord(search);
    return WordCursor::Initialize(words, key, 0, 0, HTDIG_WORDLIST_WALKER);
  }

  //-
  // Return a copy of the last document found.
  //
  WordKey GetDocument() {
    WordKey found;
    key_semantic.DocumentSet(GetFound().Key(), found);
    return found;
  }

  //-
  // Store in the <i>info</i> data member textual information about
  // the latest match found.
  //
  virtual void SetInfo() { info = GetFound().Key().GetWord(); }

  //-
  // Return a copy of the <i>info</i> data member. Should be 
  // called after SetInfo().
  //
  String GetInfo() { return info; }

  //-
  // Sort WordTree data members (if any) in ascending frequency order.
  // Return OK on success, NOTOK on failure.
  //
  virtual int AscendingFrequency() { return OK; }

  //-
  // Delete WordTree data members (if any) that have a zero frequency.
  // The number of data members deleted is returned in <b>stripped</b>.
  // Return OK on success, NOTOK on failure.
  //
  virtual int StripNonExistent(unsigned int& stripped) {
    stripped = 0;
    return OK;
  }

  //
  // Input
  //
  //-
  // Proximity factor. See WordKeySemantic::LocationCompare.
  //
  int proximity;
  //-
  // Uniq WordKey field position. See WordKeySemantic::DocumentNext.
  //
  int uniq;
  //-
  // Semantic of the WordKey object.
  //
  WordKeySemantic key_semantic;
  //-
  // Textual representation of the search scope.
  //
  String scope;
  //-
  // Original search criterion that may be different from the 
  // WordCursor::searchKey data member.
  //
  String search;

  //
  // Internal state
  //
  //-
  // Textual information about the latest match.
  //
  String info;
};

// ************************* WordTreeLiteral implementation ****************

class WordTreeLiteral : public WordTree {
public:
  //-
  // Constructor. The search criterion is <b>string</b> and the
  // scope is <b>nscope.</b>.
  //
  WordTreeLiteral(const char* string, const char* nscope = "") {
    search.set((char*)string);
    scope.set((char*)nscope);
  }

  //-
  // Returns WORD_TREE_LITERAL.
  //
  int IsA() const { return WORD_TREE_LITERAL; }

  virtual int WalkRewind();
  //-
  // Only return a match for each distinct document.
  //
  virtual int WalkNext();
  virtual int Seek(const WordKey& patch);

  //-
  // If scope is set the <b>bufferout</b> is filled with
  // <pre>
  // ( word "scope" )
  // </pre>
  // otherwise the <b>bufferout</b> only contains the word.
  //
  virtual int Get(String& bufferout) const {
    if(scope.empty())
      bufferout << search;
    else
      bufferout << "( " << operator_name[IsA()] << " \"" << scope << "\" " << search << " )";
    return OK;
  }
 
protected:
  WordKey current_document;
};

int WordTreeLiteral::WalkRewind()
{
  current_document.Clear();
  return WordCursor::WalkRewind();
}

int WordTreeLiteral::WalkNext()
{
  int ret;
  do {
    ret = WordCursor::WalkNext();
    if(verbose > 3) fprintf(stderr, "WordTreeLiteral::WalkNext: reached %s\n", (char*)GetDocument().Get());
  } while(ret == OK &&
	  key_semantic.DocumentCompare(current_document, GetDocument()) == 0);

  if(ret == OK)
    current_document = GetDocument();
  else
    current_document.Clear();

  return ret;
}

int WordTreeLiteral::Seek(const WordKey& position)
{
  current_document.Clear();
  return WordCursor::Seek(position);
}

// ************************* WordTreeOperand implementation ****************
//
// NAME
// 
// Base class for boolean query resolution nodes
//
// SYNOPSIS
//
// #include <WordTree.h>
//
// class WordTreeMethod : public WordTreeOperand {
// ...
// };
//
// DESCRIPTION
//
// The WordTreeOperand class is derived from WordTree and implemet
// the basic operations and data structures needed for query resultion
// of boolean operators. It contains a list of WordTree objects (the
// operands or cursors) and redefine the basic WordCursor methods
// to operate on all of them according to the logic defined by the
// derived class.
//
//
// END
//

//
// Helper for debugging that returns the string representation
// of the return codes.
//
static char* ret2str(int ret)
{
  if(ret == WORD_WALK_REDO)
    return "REDO";

  if(ret == WORD_WALK_RESTART)
    return "RESTART";

  if(ret == WORD_WALK_NEXT)
    return "NEXT";

  if(ret == OK)
    return "OK";

  if(ret == NOTOK)
    return "NOTOK";

  if(ret == WORD_WALK_ATEND)
    return "ATEND";

  return "???";
}

class WordTreeOperand : public WordTree
{
public:
  //-
  // Constructor. The scope is <b>nscope</b>.
  //
  WordTreeOperand(const char* nscope) {
    scope.set((char*)nscope);
  }
  //-
  // Free the objects pointed by <i>cursors</i> with delete as well
  // as the <i>cursors</i> array itself with delete [].
  //
  virtual ~WordTreeOperand();

  virtual void Clear() {
    cursors = 0;
    cursors_length = 0;
    WordCursor::Clear();
  }

  //-
  // Recursively call Optimize on each <i>cursors</i>.
  //
  virtual int Optimize();

  //-
  // Change the <i>permutation</i> data member ignore mask according
  // to WORD_TREE_MANDATORY and WORD_TREE_NOT nodes found in 
  // <i>cursors</i>. MANDATORY and NOT nodes are reduced (replaced
  // by their first child cursor. For each MANDATORY and NOT nodes
  // the bit (see WordExcludeMask for information) 
  // corresponding to their position is ignored (set in the <b>ignore</b>
  // argument of the WordExcludeMask::Initialize function. For NOT
  // nodes, the bit corresponding to their position is set in 
  // the <b>ignore_mask</b> of the WordExcludeMask::Initialize function
  // (i.e. implementing a <i>not</i> operation).
  // The <b>proximity</b> argument may be WORD_PERMUTE_PROXIMITY_TOGGLE or 
  // WORD_PERMUTE_PROXIMITY_NO.
  // Returns OK on success, NOTOK on failure.
  //
  int OptimizeOr(int proximity);

  virtual int ContextSave(String& buffer) const {
    StringList list;
    int ret;
    if((ret = ContextSaveList(list)) != OK)
      return ret;

    buffer.trunc();
    String* element;
    list.Start_Get();
    while((element = (String*)list.Get_Next())) {
      buffer << (*element) << ';';
    }
    //
    // Trim last ;
    //
    buffer.chop(1);

    return OK;
  }

  virtual int ContextSaveList(StringList& list) const {
    //
    // Apply to each cursor
    //
    unsigned int i;
    for(i = 0; i < cursors_length; i++)
      if(cursors[i]->ContextSaveList(list) == NOTOK)
	return NOTOK;
    return OK;
  }

  virtual int ContextRestore(const String& buffer) {
    if(!buffer.empty()) {
      StringList list(buffer, ";");
      return ContextRestoreList(list);
    } else {
      return OK;
    }
  }

  virtual int ContextRestoreList(StringList& list) {
    //
    // Apply to each cursor
    //
    unsigned int i;
    for(i = 0; i < cursors_length; i++)
      if(cursors[i]->ContextRestoreList(list) == NOTOK)
	return NOTOK;
    return OK;
  }

  //-
  // Recursively call WalkInit on each <i>cursors</i>.
  //
  virtual int WalkInit();
  //-
  // Recursively call WalkRewind on each <i>cursors</i>.
  // Reset the <i>pos</i> data member with WordKeySemantic::DocumentClear.
  //
  virtual int WalkRewind();
  //-
  // Recursively call WalkFinish on each <i>cursors</i>.
  //
  virtual int WalkFinish();
  //-
  // Recursively call Seek on each <i>cursors</i>.
  // Save the <b>patch</b> argument in the <i>pos</i> data
  // member.
  //
  virtual int Seek(const WordKey& patch);

  //-
  // The number of occurrence of a WordTreeOperand is the sum of the
  // number of occurrence of each term.
  //
  virtual int Noccurrence(unsigned int& noccurrence) const {
    noccurrence = 0;
    unsigned int i;
    for(i = 0; i < cursors_length; i++) {
      unsigned int frequency;
      if(cursors[i]->Noccurrence(frequency) != OK)
	return NOTOK;
      noccurrence += frequency;
    }
    return OK;
  }

  //-
  // The <b>bufferout</b> argument is filled with a lisp like representation
  // of the tree starting at this node.
  //
  virtual int Get(String& bufferout) const {
    bufferout << "( " << operator_name[IsA()] << " \"" << scope << "\" ";
    unsigned int i;
    for(i = 0; i < cursors_length; i++)
      bufferout << cursors[i]->Get() << " ";
    bufferout << " )";
    return OK;
  }

  //-
  // Call Prepare on each <i>cursors</i>. Set the <i>search</i> member
  // with an textual representation of the tree starting at this node.
  //
  virtual int Prepare(WordList *words, int nuniq, int nproximity, int *document, int document_length, int location) {
    int ret;
    if((ret = WordTree::Prepare(words, nuniq, nproximity, document, document_length, location)) != OK)
      return ret;
    unsigned int i;
    for(i = 0; i < cursors_length; i++) {
      if((ret = cursors[i]->Prepare(words, nuniq, nproximity, document, document_length, location)) != OK)
	return ret;
    }
    return Get(GetSearch().GetWord());
  }

  //-
  // The current cursor offset (set by Seek for instance). It
  // duplicates the function of the WordCursor <i>key</i> data member
  // because the data type is different (WordKey instead of String).
  //
  WordKey pos;
  //-
  // Sub nodes array.
  //
  WordTree** cursors;
  //-
  // Number of valid entries in the <i>cursors</i> member.
  //
  unsigned int cursors_length;
  //-
  // Permutation generator with proximity toggle
  //
  WordPermute permutation;
};

WordTreeOperand::~WordTreeOperand()
{
  if(cursors) {
    unsigned int i;
    for(i = 0; i < cursors_length; i++)
      delete cursors[i];
    free(cursors);
  }
}

int 
WordTreeOperand::Optimize()
{
  //
  // Apply to each cursor
  //
  unsigned int i;
  for(i = 0; i < cursors_length; i++)
    if(cursors[i]->Optimize() == NOTOK)
      return NOTOK;
  return OK;
} 

int WordTreeOperand::OptimizeOr(int proximity)
{
  unsigned int ignore = 0;
  unsigned int ignore_mask = 0;
  unsigned int i;
  for(i = 0; i < cursors_length; i++) {
    int reduce;
    //
    // Set ignore & ignore_mask if cursor is NOT or MANDATORY
    //
    switch(cursors[i]->IsA()) {
    case WORD_TREE_MANDATORY:
      ignore |= (1 << WORD_EXCLUDE_POSITION2BIT(cursors_length, i));
      reduce = 1;
      break;
    case WORD_TREE_NOT:
      ignore |= (1 << WORD_EXCLUDE_POSITION2BIT(cursors_length, i));
      ignore_mask |= (1 << WORD_EXCLUDE_POSITION2BIT(cursors_length, i));
      reduce = 1;
      break;
    default:
      reduce = 0;
      break;
    }
    //
    // Replace the NOT or MANDATORY node by its only child
    //
    if(reduce) {
      WordTreeOperand* old = (WordTreeOperand*)cursors[i];
      cursors[i] = old->cursors[0];
      old->cursors[0] = 0;
      old->cursors_length--;
      if(old->cursors_length > 0) {
	fprintf(stderr, "WordTreeOptional::OptimizeOr: too many cursors\n");
	return NOTOK;
      }
      delete old;
    }
  }
  return permutation.Initialize(cursors_length, ignore, ignore_mask, proximity);
}

int 
WordTreeOperand::WalkInit()
{
  unsigned int i;
  int ret = WORD_WALK_ATEND;
  for(i = 0; i < cursors_length; i++)
    if((ret = cursors[i]->WalkInit()) != OK)
      return ret;
  return (status = ret);
}

int 
WordTreeOperand::WalkRewind()
{
  unsigned int i;
  int ret = OK;
  for(i = 0; i < cursors_length; i++)
    if((ret = cursors[i]->WalkRewind()) != OK)
      return ret;
  status = OK;
  key_semantic.DocumentClear(pos);
  cursor_get_flags = DB_SET_RANGE;
  found.Clear();
  return ret;
}

int 
WordTreeOperand::WalkFinish()
{
  unsigned int i;
  int ret = OK;
  for(i = 0; i < cursors_length; i++)
    if((ret = cursors[i]->WalkFinish()) != OK)
      return ret;
  return ret;
}

int 
WordTreeOperand::Seek(const WordKey& patch)
{
  pos.CopyFrom(patch);
  cursor_get_flags = DB_SET_RANGE;

  unsigned int i;
  int ret = OK;
  for(i = 0; i < cursors_length; i++)
    if((ret = cursors[i]->Seek(patch)) != OK &&
       ret != WORD_WALK_ATEND)
      return ret;
  status = OK;
  return OK;
}

// ************************* WordTreeOptional implementation ****************

class WordTreeOptional : public WordTreeOperand {
 public:
  WordTreeOptional(const char* nscope) : WordTreeOperand(nscope) { }

  //-
  // Return WORD_TREE_OPTIONAL
  //
  virtual int IsA() const { return WORD_TREE_OPTIONAL; }

  virtual int Optimize();

  virtual int ContextSaveList(StringList& list) const;

  virtual int ContextRestoreList(StringList& list);

  //-
  // Multipass walk of the occurrences according to the <i>permutation</i>
  // data member specifications. First search for documents containing
  // all occurrences near to each other. Then documents that
  // contain all occurrences far appart. Then ignore the most frequent 
  // search criterion and search for documents that contain all the others
  // near to each other. The logic goes on until there only remains the
  // most frequent word. 
  //
  virtual int WalkNext();
  //-
  // Only seek the first non excluded cursor. The implementation
  // of WalkNext makes it useless to seek the others.
  //
  virtual int Seek(const WordKey& position);

  virtual int Prepare(WordList *words, int nuniq, int nproximity, int *document, int document_length, int location) {
    int ret;
    if((ret = permutation.Initialize(cursors_length, 0, 0, WORD_PERMUTE_PROXIMITY_TOGGLE)) != OK)
      return ret;
    return WordTreeOperand::Prepare(words, nuniq, nproximity, document, document_length, location);
  }

  virtual void SetInfo();

  virtual int UseProximity() const { return WORD_PERMUTE_PROXIMITY_TOGGLE; }

  virtual int UsePermutation() const { return 1; }

  //-
  // Returns true if all cursors must have a frequency > 0, false otherwise.
  // 
  virtual int AllOrNothing() const { return 0; }

  //-
  // Comparison between <b>cursor</b> and <b>constraint</b> is made
  // with WordKeySemantic::LocationCompare using the <b>proximity</b>
  // argument. If <b>master</b> is NULL it is set to point to <b>
  // <b>cursor</b>.
  //
  // Return WORD_WALK_NEXT if <b>cursor</b> is at <b>constraint</b> and
  // set <b>constraint</b> if <b>cursor</b> is <b>master</b>.
  //
  // Return WORD_WALK_REDO if <b>cursor</b> is above <b>constraint</b> and
  // call cursor.WalkNext().
  //
  // Return WORD_WALK_RESTART if <b>cursor</b> is below <b>constraint</b> and
  // set <b>constraint</b> from <b>cursor</b> using
  // WordKeySemantic::DocumentSet if <b>cursor</b> is not <b>master</b> 
  // otherwise also set location of <b>constraint</b> using 
  // WordKeySemantic::LocationSet and call WordKeySemantic::LocationNext
  // on <b>constraint.</b>
  //
  // Return WORD_WALK_ATEND if no more match possible.
  //
  // Return NOTOK on failure.
  //
  int SearchCursorNear(WordTree& cursor, WordTree*& master, WordKey& constraint, int proximity);
  //-
  // Comparison between <b>cursor</b> and <b>document</b> is made
  // with WordKeySemantic::DocumentCompare.
  //
  // Return WORD_WALK_NEXT if <b>cursor</b> is above <b>document.</b>
  //
  // Return WORD_WALK_REDO if <b>cursor</b> is below <b>document</b>
  // and call cursor.WalkNext().
  //
  // Return WORD_WALK_RESTART if <b>cursor</b> is at <b>document</b>
  // and call WordKeySemantic::DocumentNext method on <b>document.</b>
  //
  // Return WORD_WALK_ATEND if no more match possible.
  //
  // Return NOTOK on failure.
  //
  int SearchCursorNot(WordTree& cursor, WordKey& document);
  //-
  // Comparison between <b>cursor</b> and <b>document</b> is made
  // with WordKeySemantic::DocumentCompare.
  //
  // Return WORD_WALK_NEXT if <b>cursor</b> is at <b>document.</b>.
  //
  // Return WORD_WALK_REDO if <b>cursor</b> is below <b>document</b>
  //
  // Return WORD_WALK_RESTART if <b>cursor</b> is above <b>document</b>
  // and call WordKeySemantic::DocumentNext method on <b>document.</b>
  //
  // Return WORD_WALK_ATEND if no more match possible.
  //
  // Return NOTOK on failure.
  // 
  //
  int SearchCursorAnd(WordTree& cursor, WordKey& document, WordExclude& permutation);
  //
  // We know that : 
  // 1) document does not contain any excluded words.
  // 2) contains at least one occurrence of each non excluded word.
  // The logic, although very similar to WordSearchNear::SearchOne
  // is therefore simpler. We ignore all excluded cursors and 
  // return WORD_SEARCH_NOPROXIMITY as soon as a cursor move outside
  // <document>.
  //
  //-
  // If <b>document</b> contains words that match proximity
  // requirement, return OK. Return WORD_SEARCH_NOPROXIMITY if proximity
  // requirement cannot be matched for <document>. 
  //
  int CursorsObeyProximity(WordKey& document);

  //-
  // Sort the <i>cursors</i> in ascending frequency order using the
  // Noccurrence method on each cursor.
  // Return OK on success, NOTOK on failure.
  //
  virtual int AscendingFrequency();
  //-
  // Delete all elements of the <i>cursors</i> array that have a 
  // zero frequency. The <i>cursors</i> array is shrinked and the 
  // <i>cursors_length</i> set accordingly. Returns the number of
  // deletions in the <b>stripped</i> argument. 
  // Return OK on success, NOTOK on failure.
  //
  virtual int StripNonExistent(unsigned int& stripped);
};

int WordTreeOptional::Optimize()
{
  int ret;
  if((ret = WordTreeOperand::Optimize()) != OK)
    return ret;

  if(UseProximity() != WORD_PERMUTE_PROXIMITY_ONLY) {
    if((ret = AscendingFrequency()) != OK)
      return ret;
  }

  unsigned int stripped;
  if((ret = StripNonExistent(stripped)) != OK)
    return ret;

  if(AllOrNothing() && stripped) {
    //
    // One word is missing and everything is lost,
    // Just kill the remaining cursors.
    //
    unsigned int i;
    for(i = 0; i < cursors_length; i++)
      delete cursors[i];
    cursors_length = 0;

    return OK;
  } else {
    return OptimizeOr(UseProximity());
  }
}

int WordTreeOptional::ContextSaveList(StringList& list) const
{
  int ret;
  if((ret = WordTreeOperand::ContextSaveList(list)) != OK)
    return ret;

  if(UsePermutation()) {
    String* buffer = new String();
    permutation.Get(*buffer);
    
    list.Add(buffer);
  }
   
  {
    String* buffer = new String();
    if((ret = WordCursor::ContextSave(*buffer)) != OK)
      return ret;

    list.Add(buffer);
  }

  return OK;
}

int WordTreeOptional::ContextRestoreList(StringList& list)
{
  int ret;
  if((ret = WordTreeOperand::ContextRestoreList(list)) != OK)
    return ret;

  if(UsePermutation()) {
    char* buffer = list[0];
    if((ret = permutation.Set(buffer)) != OK)
      return ret;
    list.Remove(0);
  }

  {
    char* buffer = list[0];
    if(!buffer) return NOTOK;
    WordKey key(buffer);
    if((ret = Seek(key)) != OK)
      return ret;
    cursor_get_flags = DB_NEXT;

    list.Remove(0);
  }

  return OK;
}

int WordTreeOptional::WalkNext()
{
  WordKey& constraint = pos;
  //
  // Set constraint with all 0
  //
  if(constraint.Empty())
    key_semantic.DocumentClear(constraint);
  
  //
  // Advance cursors to next constraint, if not at the
  // beginning of the search.
  //
  int ret = OK;
  int match_ok = 0;
  do {
    //
    // Advance cursors so that next call fetches another constraint
    //
    if(cursor_get_flags == DB_NEXT)
      key_semantic.DocumentNext(constraint, uniq);
    
    if((ret = Seek(constraint)) != OK)
      return ret;

    int near = permutation.Proximity();
    WordTree* first = 0;
    for(unsigned int i = 0; i < cursors_length;) {
      WordTree& cursor = *(cursors[i]);
      near = permutation.Proximity();
      int excluded = permutation.Excluded(i);
      if(verbose) fprintf(stderr, "WordTreeOptional::WalkNext: %s excluded = %s, proximity = %s\n", (char*)cursor.GetSearch().GetWord(), (excluded ? "yes" : "no"), (near ? "yes" : "no" ));

      int ret;
      if(excluded) {
	ret = SearchCursorNot(cursor, constraint);
	if(verbose > 2) fprintf(stderr, "WordTreeOptional::WalkNext: Not -> %s\n", ret2str(ret));
      } else {
	if(near) {
	  ret = SearchCursorNear(cursor, first, constraint, proximity);
	  if(verbose > 2) fprintf(stderr, "WordTreeOptional::WalkNext: Near -> %s\n", ret2str(ret));
	} else {
	  ret = SearchCursorAnd(cursor, constraint, permutation);
	  if(verbose > 2) fprintf(stderr, "WordTreeOptional::WalkNext: And -> %s\n", ret2str(ret));
	}
      }

      switch(ret) {
      case WORD_WALK_ATEND:
	if(UsePermutation()) {
	  //
	  // The search is over with this permutation, try another one.
	  //
	  switch(permutation.Next()) {
	    //
	    // No permutations left, the end
	    //
	  case WORD_PERMUTE_END:
	    return (status = WORD_WALK_ATEND);
	    break;

	    //
	    // Sart over with this permutation
	    //
	  case WORD_PERMUTE_OK:
	    if(WalkRewind() != OK)
	      return NOTOK;
	    break;
	  }
	  first = 0;
	  i = 0;
	} else {
	  return (status = WORD_WALK_ATEND);
	}
	break;
      case WORD_WALK_REDO:
	break;
      case WORD_WALK_RESTART:
	first = 0;
	i = 0;
	break;
      case WORD_WALK_NEXT:
	i++;
	break;
      case NOTOK:
      default:
	return ret;
	break;
      }
    }

    cursor_get_flags = DB_NEXT;

    SetInfo();
    
    //
    // Save possible result, i.e. first non excluded cursor
    //
    for(unsigned int i = 0; i < cursors_length; i++) {
      WordTree& cursor = *(cursors[i]);
      if(!permutation.Excluded(i)) {
	found.Key().CopyFrom(cursor.GetFound().Key());
	break;
      }
    }

    match_ok = 1;
    //
    // Only bother if near and non near search are involved
    //
    if(UseProximity() == WORD_PERMUTE_PROXIMITY_TOGGLE) {
      //
      // If we reach this point in the function and 
      // either proximity search is active or there is
      // only one word involved, the match is valid. 
      // Otherwise it may be excluded, see below.
      //
      if(!near && permutation.NotExcludedCount() > 1) {
	//
	// If not using proximity, a match that fits the proximity 
	// requirements must be skipped because it was matched by 
	// the previous permutation (see WordPermute).
	//
	switch(CursorsObeyProximity(constraint)) {
	case OK:
	  match_ok = 0;
	  break;
	case WORD_SEARCH_NOPROXIMITY:
	  match_ok = 1;
	  break;
	default:
	case NOTOK:
	  return NOTOK;
	  break;
	}
      }
    }
  } while(!match_ok && ret == OK);

  return ret;
}

int WordTreeOptional::Seek(const WordKey& position)
{
  pos.CopyFrom(position);
  cursor_get_flags = DB_SET_RANGE;
  status = OK;

  unsigned int i;
  for(i = 0; i < cursors_length; i++) {
    if(!permutation.Excluded(i)) {
      WordTree& cursor = *(cursors[i]);
      return cursor.Seek(position);
    }
  }

  fprintf(stderr, "WordTreeOptional::Seek: failed\n");
  return NOTOK;
}  


void WordTreeOptional::SetInfo()
{
  unsigned int i;
  for(i = 0; i < cursors_length; i++)
    cursors[i]->SetInfo();

  info.trunc();

  for(i = 0; i < cursors_length; i++) {
    WordTree& cursor = *(cursors[i]);

    if(!permutation.Excluded(i))
      info << cursor.info << " ";
  }

  info << (permutation.Proximity() ? "proximity" : "");
}

int WordTreeOptional::SearchCursorNear(WordTree& cursor, WordTree*& master, WordKey& constraint, int proximity)
{
  int is_master = master == 0 || master == &cursor;
  if(master == 0) master = &cursor;
  const WordKey& masterKey = master->GetFound().Key();

  int direction = key_semantic.LocationCompare(constraint, cursor.GetFound().Key(), proximity);
  if(verbose > 2) fprintf(stderr, "WordTreeOptional::SearchCursorNear: LocationCompare(\n\t%s,\n\t%s)\n\t = %d\n", (char*)(constraint.Get()), (char*)(cursor.GetFound().Key().Get()), direction);

  //
  // If the cursor is in the authorized locations, consider
  // next cursor
  //
  if(direction == 0) {
    //
    // master cursor makes the rules for location : its location
    // is the base to calculate other words mandatory loacations.
    //
    if(is_master)
      key_semantic.LocationSet(cursor.GetFound().Key(), constraint);
    //
    // Fix location constraint to accomodate proximity tolerance.
    //
    key_semantic.LocationNearLowest(constraint, proximity);
    return WORD_WALK_NEXT;

    //
    // If current location is above cursor location
    //
  } else if(direction > 0) {
    //
    // Move the cursor up to the location.
    //
    cursor.Seek(constraint);
    if(verbose > 1) fprintf(stderr, "WordTreeOptional::SearchCursorNear: leap to %s\n", (char*)constraint.Get());
    int ret;
    if((ret = cursor.WalkNext()) == OK) {
      //
      // Remove the location constraint for the master word
      // so that it matches and then enforce location for other
      // keys.
      //
      if(is_master)
	key_semantic.Location2Document(constraint);
      //
      // Reconsider the situation for this cursor
      //
      return WORD_WALK_REDO;
    } else {
      return ret;
    }

    //
    // If current location is lower than cursor location,
    // meaning that the cursor found no match for the current
    // location.
    //
  } else if(direction < 0) {
    //
    // The cursor document becomes the current document. 
    // The master cursor is forced to catch up.
    //
    key_semantic.DocumentSet(cursor.GetDocument(), constraint);
    //
    // It is possible that this cursor document is the same
    // as the master cursor document (if this cursor hit in the
    // same document but a higher location). In this case we must
    // increase the location of the master cursor otherwise it will
    // match without moving and loop forever.
    //
    if(!is_master && key_semantic.DocumentCompare(masterKey, constraint) == 0) {
      key_semantic.LocationSet(masterKey, constraint);
      key_semantic.LocationNext(constraint);
    }
    //
    // Since the current location changed, start over.
    //
    return WORD_WALK_RESTART;
  } else {
    fprintf(stderr, "WordTreeOptional::WordCursorNear: reached unreachable statement\n");
    return NOTOK;
  }
  return NOTOK;
}

int WordTreeOptional::SearchCursorNot(WordTree& cursor, WordKey& document)
{
  int direction = key_semantic.DocumentCompare(document, cursor.GetFound().Key());
  if(verbose > 2) fprintf(stderr, "WordTreeOptional::SearchCursorNot: DocumentCompare(\n\t%s,\n\t%s)\n\t = %d\n", (char*)(document.Get()), (char*)(cursor.GetFound().Key().Get()), direction);

  //
  // If the cursor is above the current document
  // (being at the end of walk is being above all documents).
  //
  // Means that the cursor is positioned in an acceptable document
  // and proceed to the next cursor.
  //
  if(direction < 0 || cursor.IsAtEnd()) {
    return WORD_WALK_NEXT;

    //
    // If the cursor is below current document
    //
  } else if(direction > 0) {
    //
    // Move the cursor up to the document
    //
    cursor.Seek(document);
    if(verbose > 1) fprintf(stderr, "WordTreeOptional::SearchCursorNot: leap to %s\n", (char*)document.Get());
    int ret;
    if((ret = cursor.WalkNext()) != OK && ret != WORD_WALK_ATEND)
      return NOTOK;
    //
    // It is expected in this case that the cursor has moved above
    // the current document and another visit in the loop will
    // tell us.
    //
    return WORD_WALK_REDO;

    //
    // If the cursor matches the current document.
    //
    // Means that the current document is not a possible match
    // since it is pointed by this cursor.
    //
  } else if(direction == 0) {
    //
    // The cursor does not give any hint on a possible
    // next document, just go to the next possible one.
    //
    key_semantic.DocumentNext(document, uniq);
    //
    // Since the current document changed, start over.
    //
    return WORD_WALK_RESTART;
  } else {
    fprintf(stderr, "WordTreeOptional::WordCursorNot: reached unreachable statement\n");
    return NOTOK;
  }
  return NOTOK;
}

int WordTreeOptional::SearchCursorAnd(WordTree& cursor, WordKey& document, WordExclude& permutation)
{
  int direction = key_semantic.DocumentCompare(document, cursor.GetFound().Key());
  if(verbose > 2) fprintf(stderr, "WordTreeOptional::SearchCursorAnd: DocumentCompare(\n\t%s,\n\t%s)\n\t = %d\n", (char*)(document.Get()), (char*)(cursor.GetFound().Key().Get()), direction);

  //
  // If the cursor is in the current document.
  //
  // Means that the cursor is positioned in an acceptable document
  // and proceed to the next cursor.
  //
  if(direction == 0) {
    return WORD_WALK_NEXT;

    //
    // If the cursor is below current document
    //
  } else if(direction > 0) {
    //
    // Move the cursor up to the document
    //
    cursor.Seek(document);
    if(verbose > 1) fprintf(stderr, "WordTreeOptional::SearchCursorAnd: leap to %s\n", (char*)document.Get());
    int ret;
    if((ret = cursor.WalkNext()) == OK)
      return WORD_WALK_REDO;
    else
      return ret;

    //
    // If the cursor is above current document.
    //
    // Means the the current document is not a possible match
    // since it will never reach it because it's already
    // above it.
    //
  } else if(direction < 0) {
    //
    // The cursor document becomes the current document.
    //
    key_semantic.DocumentSet(cursor.GetDocument(), document);

    //
    // Since the current document changed, start over.
    //
    return WORD_WALK_RESTART;
  } else {
    fprintf(stderr, "WordTreeOptional::WordCursorAnd: reached unreachable statement\n");
    return NOTOK;
  }
  return NOTOK;
}

int WordTreeOptional::CursorsObeyProximity(WordKey& document)
{
  //
  // Run if more than one word is involved, proximity
  // is always true if there is only one word.
  //
  if(permutation.NotExcludedCount() <= 1) return OK;

  WordKey location;

  //
  // The first non excluded cursor contains anchor location.
  //
  unsigned int master_index = 0;
  for(unsigned int i = 0; i < cursors_length; i++) {
    if(!permutation.Excluded(i)) {
      master_index = i;
      break;
    }
  }
  const WordKey& masterKey = cursors[master_index]->GetFound().Key();
  key_semantic.DocumentSet(masterKey, location);

  for(unsigned int i = 0; i < cursors_length;) {
    if(permutation.Excluded(i)) {
      i++;
      continue;
    }
    
    WordTree& cursor = *(cursors[i]);
    if(cursor.IsAtEnd()) return WORD_SEARCH_NOPROXIMITY;
    //    if(cursor.status & WORD_WALK_FAILED) return NOTOK;

    //
    // If the cursor moved outside of the tested document,
    // no proximity match is possible.
    //
    if(key_semantic.DocumentCompare(cursor.GetFound().Key(), document) != 0)
      return WORD_SEARCH_NOPROXIMITY;

    int direction = key_semantic.LocationCompare(location, cursor.GetFound().Key(), proximity);
    
    //
    // If the cursor is in the authorized locations, consider
    // next cursor
    //
    if(direction == 0) {
      //
      // master cursor makes the rules for location : its location
      // is the base to calculate other words mandatory loacations.
      //
      if(i == master_index)
	key_semantic.LocationSet(cursor.GetFound().Key(), location);
      //
      // Fix location constraint to accomodate proximity tolerance.
      //
      key_semantic.LocationNearLowest(location, proximity);
      i++;

    //
    // If current location is greater than cursor location
    //
    } else if(direction > 0) {
      //
      // Move the cursor up to the location.
      //
      cursor.Seek(location);
      if(verbose > 1) fprintf(stderr, "WordTreeOptional::CursorsObeyProximity: leap to %s\n", (char*)location.Get());
      int ret;
      if((ret = cursor.WalkNext()) != OK) {
	if(ret == WORD_WALK_ATEND) {
	  return WORD_SEARCH_NOPROXIMITY;
	} else {
	  return NOTOK;
	}
      }
      //
      // Remove the location constraint for the master word
      // so that it matches and then enforce location for other
      // keys.
      //
      if(i == master_index)
	key_semantic.Location2Document(location);
      //
      // Reconsider the situation for this cursor
      //

    //
    // If current location is lower than cursor location,
    // meaning that the cursor found no match in the current
    // document.
    //
    } else if(direction < 0) {
      //
      // Move to next master key, if possible.
      //
      if(i != master_index) {
	key_semantic.LocationSet(masterKey, location);
	key_semantic.LocationNext(location);
      }
      //
      // Since the current location changed, start over.
      //
      i = 0;
    }
  }

  return OK;
}

//
// Helper class for AscendingFrequency method
//
class WordSort {
public:
  unsigned int frequency;
  WordTree *cursor;
};

//
// Helper function for AscendingFrequency method
//
static int ascending_frequency(const void *a, const void *b)
{
  const WordSort& a_cursor = *(WordSort*)a;
  const WordSort& b_cursor = *(WordSort*)b;

  return a_cursor.frequency - b_cursor.frequency;
}

int WordTreeOptional::AscendingFrequency()
{
  //
  // Reorder cursors
  //
  WordSort *tmp = new WordSort[cursors_length];

  memset((char*)tmp, '\0', sizeof(WordSort[cursors_length]));

  unsigned int i;
  for(i = 0; i < cursors_length; i++) {
    unsigned int frequency;
    if(cursors[i]->Noccurrence(frequency) != OK) {
      delete [] tmp;
      return NOTOK;
    }
    if(verbose > 2) fprintf(stderr, "WordTreeOptional::AscendingFrequency: %s occurs %d times\n", (char*)cursors[i]->GetSearch().Get(), frequency);
    tmp[i].frequency = frequency;
    tmp[i].cursor = cursors[i];
  }

  memset((char*)cursors, '\0', sizeof(WordTree*) * cursors_length);

  qsort((void *)tmp, cursors_length, sizeof(WordSort), &ascending_frequency);

  for(i = 0; i < cursors_length; i++)
    cursors[i] = tmp[i].cursor;

  delete [] tmp;
  return OK;
}

int WordTreeOptional::StripNonExistent(unsigned int& stripped)
{
  stripped = 0;

  WordTree** tmp = new WordTree*[cursors_length];
  memset((char*)tmp, '\0', sizeof(WordTree*[cursors_length]));

  unsigned int from;
  unsigned int to;

  for(to = from = 0; from < cursors_length; from++) {
    unsigned int frequency;
    if(cursors[from]->Noccurrence(frequency) != OK) {
      delete [] tmp;
      return NOTOK;
    }

    if(verbose > 2) fprintf(stderr, "WordTreeOptional::StripNonExistent: %s occurs %d times\n", (char*)cursors[from]->GetSearch().Get(), frequency);
    if(frequency > 0) {
      tmp[to++] = cursors[from];
    } else {
      delete cursors[from];
      stripped++;
    }
  }

  memset((char*)cursors, '\0', sizeof(WordTree*) * cursors_length);
  
  cursors_length = to;
  unsigned int i;
  for(i = 0; i < cursors_length; i++)
    cursors[i] = tmp[i];

  delete [] tmp;

  return OK;
}

// ************************* WordTreeOr implementation ********************

class WordTreeOr : public WordTreeOperand {
 public:
  WordTreeOr(const char* nscope) : WordTreeOperand(nscope) { }

  //-
  // Return WORD_TREE_OR
  //
  virtual int IsA() const { return WORD_TREE_OR; }

  virtual int Optimize();

  virtual int ContextSaveList(StringList& list) const;

  virtual int ContextRestoreList(StringList& list);

  virtual void SetInfo();

  virtual int WalkNext();

  virtual int UsePermutation() const { return 0; }

  virtual int UseProximity() const { return WORD_PERMUTE_PROXIMITY_NO; }
};

int WordTreeOr::Optimize()
{
  int ret;
  if((ret = WordTreeOperand::Optimize()) != OK)
    return ret;

  if((ret = AscendingFrequency()) != OK)
    return ret;

  unsigned int stripped;
  if((ret = StripNonExistent(stripped)) != OK)
    return ret;

  return OptimizeOr(WORD_PERMUTE_PROXIMITY_NO);
}

int WordTreeOr::ContextSaveList(StringList& list) const
{
  int ret;
  if((ret = WordTreeOperand::ContextSaveList(list)) != OK)
    return ret;

  {
    String* buffer = new String();
    permutation.Get(*buffer);
    
    list.Add(buffer);
  }
   
  {
    String* buffer = new String();
    if((ret = WordCursor::ContextSave(*buffer)) != OK)
      return ret;

    list.Add(buffer);
  }

  return OK;
}

int WordTreeOr::ContextRestoreList(StringList& list)
{
  int ret;
  if((ret = WordTreeOperand::ContextRestoreList(list)) != OK)
    return ret;

  {
    char* buffer = list[0];
    if((ret = permutation.Set(buffer)) != OK)
      return ret;
    list.Remove(0);
  }

  {
    char* buffer = list[0];
    if(!buffer) return NOTOK;
    WordKey key(buffer);
    if((ret = Seek(key)) != OK)
      return ret;
    cursor_get_flags = DB_NEXT;

    list.Remove(0);
  }

  return OK;
}

void WordTreeOr::SetInfo()
{
  unsigned int i;
  for(i = 0; i < cursors_length; i++)
    cursors[i]->SetInfo();

  info.trunc();

  for(i = 0; i < cursors_length; i++) {
    WordTree& cursor = *(cursors[i]);

    if(!permutation.Excluded(i) &&
       !cursor.IsAtEnd() &&
       key_semantic.DocumentCompare(cursor.GetFound().Key(), GetFound().Key()) == 0) {
      info << cursor.info << " ";
    }
  }
}

int WordTreeOr::WalkNext()
{
  WordKey& constraint = pos;
  //
  // Set constraint with all 0
  //
  if(constraint.Empty())
    key_semantic.DocumentClear(constraint);
  
  WordKey candidate;
  int match_ok;
  do {
      int ret;
      unsigned int i;
      candidate.Clear();
      //
      // Advance cursors so that next call fetches another constraint
      //
      if(cursor_get_flags == DB_NEXT)
	  key_semantic.DocumentNext(constraint, uniq);
    
      if((ret = Seek(constraint)) != OK)
	  return ret;

      match_ok = 1;
      //
      // All non excluded cursors are about to move
      // at or beyond constraint. Search for the one (candidate) that
      // is located at the lowest location beyond the constraint.
      //
      for(i = 0; i < cursors_length; i++) {
	  if(permutation.Excluded(i))
	      continue;
	  WordTree& cursor = *(cursors[i]);

	  switch((ret = cursor.WalkNext())) {
	  case WORD_WALK_ATEND:
	      //
	      // Constraint is above all matches for this cursor
	      //
	      break;
	  case OK:
	      //
	      // If candidate is not set or current cursor is below
	      // the current candidate, the curent cursor document becomes
	      // the candidate.
	      //
	      if(candidate.Empty() ||
		 key_semantic.DocumentCompare(candidate, cursor.GetFound().Key()) > 0) {
		  key_semantic.DocumentSet(cursor.GetDocument(), candidate);
	      }
	      break;
	  default:
	      return ret;
	      break;
	  }
      }

      //
      // No candidate ? It's the end of the match list.
      //
      if(candidate.Empty())
	  return WORD_WALK_ATEND;

      found.Key().CopyFrom(candidate);

      SetInfo();

      if(permutation.ExcludedCount() > 0) {
	if((ret = Seek(candidate)) != OK)
	  return ret;

	//
	// Restart loop if candidate matches an excluded cursor.
	//
	for(i = 0; i < cursors_length && match_ok; i++) {
	  if(!permutation.Excluded(i))
	    continue;
	  WordTree& cursor = *(cursors[i]);

	  switch((ret = cursor.WalkNext())) {
	  case WORD_WALK_ATEND:
	    //
	    // This excluded cursor can't match the candidate, fine.
	    //
	    break;
	  case OK:
	    //
	    // This excluded cursor matches candidate therefore it's
	    // not a valid candidate. Restart search with this candidate
	    // as the constraint.
	    //
	    if(key_semantic.DocumentCompare(candidate, cursor.GetFound().Key()) == 0) {
	      constraint = candidate;
	      match_ok = 0;
	    }
	    break;
	  default:
	    return ret;
	    break;
	  }
	  
	}
      }

      cursor_get_flags = DB_NEXT;

  } while(!match_ok);

  constraint = candidate;

  return OK;
}

// ************************* WordTreeAnd implementation ********************

class WordTreeAnd : public WordTreeOptional {
 public:
  WordTreeAnd(const char* nscope) : WordTreeOptional(nscope) { }

  //-
  // Return WORD_TREE_AND
  //
  virtual int IsA() const { return WORD_TREE_AND; }

  virtual int UsePermutation() const { return 0; }

  virtual int UseProximity() const { return WORD_PERMUTE_PROXIMITY_NO; }

  virtual int AllOrNothing() const { return 1; }
};

// ************************* WordTreeNear implementation ********************

class WordTreeNear : public WordTreeOptional {
 public:
  WordTreeNear(const char* nscope) : WordTreeOptional(nscope) { }

  //-
  // Return WORD_TREE_NEAR
  //
  virtual int IsA() const { return WORD_TREE_NEAR; }

  virtual int UsePermutation() const { return 0; }

  virtual int UseProximity() const { return WORD_PERMUTE_PROXIMITY_ONLY; }

  virtual int AllOrNothing() const { return 1; }
};

// ************************* WordTreeMandatory implementation ***************

class WordTreeMandatory : public WordTreeOperand {
 public:
  WordTreeMandatory(const char* nscope) : WordTreeOperand(nscope) { }

  //-
  // Return WORD_TREE_MANDATORY
  //
  virtual int IsA() const { return WORD_TREE_MANDATORY; }
};

// ************************* WordTreeNot implementation ***************

class WordTreeNot : public WordTreeOperand {
 public:
  WordTreeNot(const char* nscope) : WordTreeOperand(nscope) { }

  //-
  // Return WORD_TREE_NOT
  //
  virtual int IsA() const { return WORD_TREE_NOT; }
};

// ************************* WordMatch implementation ********************

//
// Return value of the Search method, tells us which document
// matched and why.
//
class WordMatch {
public:

  //-
  // Return a textual representation of the object.
  //
  String Get() const;

  //-
  // The document that matched
  //
  WordKey match;
  //-
  // An ascii description of why it matched.
  //
  String info;
};

String WordMatch::Get() const
{
  String tmp;
  match.Get(tmp);
  if(!info.empty())
    tmp << "(" << info << ")";
  return tmp;
}

// ************************* WordSearch implementation ********************
//
// NAME
//
// Solve a query from a WordTree syntax tree
//
// SYNOPSIS
//
// #include <WordSearch.h>
//
// WordTree* expr = get_query();
// WordSearch search;
// search.limit_count = NUMBER_OF_RESULTS;
// WordMatch* search.Search(expr);
// ...
//  
// DESCRIPTION
//
// The WordSearch class is a wrapper to query an inverted index
// using a WordTree syntax tree. 
// 
// END
//
class WordSearch {
public:
  WordSearch();

  //-
  // Perform a search from the <b>expr</b> specifications.
  // Restore the context from <i>context_in</i> on <b>expr</b>.
  // Then skip (using WalkNext) <i>limit_bottom</i> entries.
  // Then collect in a WordMatch array of size <i>limit_count</i>
  // each match returned by WalkNext. When finished store
  // the context (ContextSave) in <i>context_out</i>.
  // It is the responsibility of the caller to free the WordMatch
  // array. If no match are found a null pointer is returned.
  //
  WordMatch *Search();

  //
  // Search backend, only run the WalkNext loop but does not
  // allocate/deallocate data.
  //
  int SearchLoop(WordTree *expr);

  //
  // Return a context description string to resume the
  // search at a given point.
  //
  const String& Context() const { return context_out; }

  //
  // Input
  //
  unsigned int limit_bottom;
  unsigned int limit_count;
  String context_in;
  WordTree* expr;
  
  //
  // Output
  //
  WordMatch* matches;
  unsigned int matches_size;
  unsigned int matches_length;
  String context_out;
};

WordSearch::WordSearch()
{
  //
  // Input
  //
  limit_bottom = 0;
  limit_count = 0;
  context_in.trunc();
  expr = 0;

  //
  // Output
  //
  matches = 0;
  matches_size = 0;
  matches_length = 0;
  context_out.trunc();
}

WordMatch *WordSearch::Search()
{
  int ret = 0;

  if(verbose) fprintf(stderr, "WordSearch::Search: non optimized expression %s\n", (char*)expr->Get());
  if(expr->Optimize() != OK)
    return 0;
  if(verbose) fprintf(stderr, "WordSearch::Search: optimized expression %s\n", (char*)expr->Get());
  
  
  //
  // Build space for results
  //
  matches_size = limit_count + 1;
  matches = new WordMatch[matches_size];
  matches_length = 0;

  //
  // Move to first possible position. 
  //
  if(expr->WalkInit() != OK)
    goto end;

  if(expr->ContextRestore(context_in) == NOTOK)
    goto end;
  ret = SearchLoop(expr);
  //
  // Don't bother saving the context if at end of 
  // search (WORD_WALK_ATEND) or error (NOTOK)
  //
  if(ret == OK && expr->ContextSave(context_out) == NOTOK)
    goto end;

end:
  expr->WalkFinish();

  if(ret == NOTOK || matches_length <= 0) {
    delete [] matches;
    matches = 0;
  }

  return matches;
}

int WordSearch::SearchLoop(WordTree *expr)
{
  int ret = OK;
  unsigned int i;
  //
  // Skip the first <limit_bottom> documents
  //
  {
    for(i = 0; i < limit_bottom; i++) {
      if((ret = expr->WalkNext()) != OK)
	return ret;
    }
  }
  //
  // Get documents up to <limit_count> or exhaustion
  //
  for(matches_length = 0; matches_length < limit_count; matches_length++) {
    if((ret = expr->WalkNext()) != OK) {
      break;
    } else {
      matches[matches_length].match = expr->GetDocument();
      if(expr->IsA() != WORD_TREE_LITERAL)
	matches[matches_length].info = ((WordTreeOperand*)expr)->GetInfo();
      if(verbose) fprintf(stderr, "WordSearch::Search: match %s\n", (char*)matches[matches_length].match.Get());
    }
  }

  if(ret == WORD_WALK_ATEND)
    matches[matches_length].match.Clear();

  return ret;
}

// ************************* WordParser implementation ********************
//
// NAME
//
// Textual query parser for test purpose
//
// SYNOPSIS
//
// #include <WordParser.h>
//
// WordParser parser;
// WordTree* expr = parser.Parse("( or \"scope1\" a query )");
// ...
// delete expr;
//
// DESCRIPTION
//
// The WordParser class implement a lisp-like parser for queries
// implemented by the WordTree derived classes. The syntax is rigid
// and should not be used for human input. The generic syntax of an
// expression is
// <pre>
// ( operator "scope" operand [operand ...] )
// </pre>
// The parenthesis must <b>always</b> be surrounded by white space otherwise
// the parser will be lost. The separator is white space and newline. 
// Tabulation may be used in scope to separate key fields.
//
// As a special case a single word is strictly equivalent
// to 
// <pre>
// ( literal "" word )
// </pre>
//
// Operators can be lower case or upper case. There is almost no syntax
// checking and it's the responsibility of the caller to associate meaningfull
// operands. For instance ( near ( not foo ) bar ) is meaningless.
// 
// OPERATORS
//
// <dl>
// 
// <dt> optional
// <dd> WordTreeOptional
// 
// <dt> or
// <dd> WordTreeOr
// 
// <dt> and
// <dd> WordTreeAnd
//
// <dt> near
// <dd> WordTreeNear
//
// <dt> not,forbiden
// <dd> WordTreeNot
//
// <dt> mandatory
// <dd> WordTreeMandatory
//
// <dt> literal
// <dd> WordTreeLiteral
//
// </dl>
//
//
// END

//
// Possible values of the info argument of ParseOperands
//
#define WORD_TREE_MANY	0x01
#define WORD_TREE_ONE	0x02
#define WORD_TREE_TWO	0x04

class WordParser {
public:
  WordTree *Parse(const String& expr);
  WordTree *ParseList(StringList& terms);

  WordTree *ParseExpr(StringList& terms);
  WordTree *ParseUnary(StringList& terms);
  WordTree *ParseConj(StringList& terms);
  void ParseOperands(StringList& terms, int info, WordTreeOperand* expr);
  WordTree *ParseLiteral(StringList& terms);
  char *ParseScope(StringList& terms);

  void Shift(StringList& terms);
  char *Term(StringList& terms);
};

WordTree *WordParser::Parse(const String& expr)
{
  StringList terms(expr, " \n");
  return ParseList(terms);
}

WordTree *WordParser::ParseList(StringList& terms)
{
  WordTree *expr = ParseExpr(terms);
  return expr;
}

WordTree *WordParser::ParseExpr(StringList& terms)
{
  WordTree *expr = 0;
  char* term = strdup(Term(terms));
  if(!strcmp(term, "(")) {
    Shift(terms);
    expr = ParseExpr(terms);
  } else if(!strcmp(term, ")")) {
    // 
    // At end of expression, return null
    //
  } else if(!mystrcasecmp(term, "optional") ||
	    !mystrcasecmp(term, "or") ||
	    !mystrcasecmp(term, "and") ||
	    !mystrcasecmp(term, "near")) {
    expr = ParseConj(terms);
  } else if(!mystrcasecmp(term, "not") ||
	    !mystrcasecmp(term, "mandatory") ||
	    !mystrcasecmp(term, "forbiden")) {
    expr = ParseUnary(terms);
  } else {
    expr = ParseLiteral(terms);
  }
  free(term);
  return expr;
}

WordTree *WordParser::ParseUnary(StringList& terms)
{
  int op = 0;
  if(!mystrcasecmp(Term(terms), "mandatory"))
    op = WORD_TREE_MANDATORY;
  else if(!mystrcasecmp(Term(terms), "forbiden") ||
	  !mystrcasecmp(Term(terms), "not"))
    op = WORD_TREE_NOT;

  Shift(terms);
  char* scope = ParseScope(terms);
  WordTreeOperand *expr = 0;
  switch(op) {
  case WORD_TREE_MANDATORY:
    expr = new WordTreeMandatory(scope);
    break;
  case WORD_TREE_NOT:
    expr = new WordTreeNot(scope);
    break;
  default:
    fprintf(stderr, "WordParser::ParseUnary: unexpected operator %d\n", op);
    exit(1);
    break;
  }
  free(scope);
  ParseOperands(terms, WORD_TREE_ONE, expr);
  return expr;
}

WordTree *WordParser::ParseConj(StringList& terms)
{
  int op = 0;
  if(!mystrcasecmp(Term(terms), "optional"))
    op = WORD_TREE_OPTIONAL;
  else if(!mystrcasecmp(Term(terms), "or"))
    op = WORD_TREE_OR;
  else if(!mystrcasecmp(Term(terms), "and"))
    op = WORD_TREE_AND;
  else if(!mystrcasecmp(Term(terms), "near"))
    op = WORD_TREE_NEAR;

  Shift(terms);
  char* scope = ParseScope(terms);
  WordTreeOperand *expr = 0;
  switch(op) {
  case WORD_TREE_OR:
    expr = new WordTreeOr(scope);
    break;
  case WORD_TREE_OPTIONAL:
    expr = new WordTreeOptional(scope);
    break;
  case WORD_TREE_AND:
    expr = new WordTreeAnd(scope);
    break;
  case WORD_TREE_NEAR:
    expr = new WordTreeNear(scope);
    break;
  default:
    fprintf(stderr, "WordParser::ParseOrAnd: unexpected operator %d\n", op);
    exit(1);
    break;
  }
  free(scope);
  ParseOperands(terms, WORD_TREE_MANY, expr);
  return expr;
}

void WordParser::ParseOperands(StringList& terms, int info, WordTreeOperand* expr)
{
  unsigned int operands_length = 0;
  unsigned int operands_size = 1;
  WordTree **operands = (WordTree**)malloc(operands_size * sizeof(WordTree*));
  WordTree *subexpr = 0;
  while((subexpr = ParseExpr(terms))) {
    operands_length++;
    if((info & WORD_TREE_ONE) && operands_length > 1) {
      fprintf(stderr, "WordParser::ParseOperands: expected only one operands\n");
      exit(1);
    } else if((info & WORD_TREE_TWO) && operands_length > 2) {
      fprintf(stderr, "WordParser::ParseOperands: expected only two operands\n");
      exit(1);
    }
    if(operands_length > operands_size) {
      operands_size = operands_length * 2;
      operands = (WordTree**)realloc(operands, operands_size * sizeof(WordTree*));
    }
    operands[operands_length - 1] = subexpr;
  }
  //
  // Discard close parenthesis
  //
  if(strcmp(Term(terms), ")")) {
    fprintf(stderr, "WordParser::ParseOperands: expected close parenthesis\n");
    exit(1);
  }
  Shift(terms);
  
  expr->cursors = operands;
  expr->cursors_length = operands_length;
}

WordTree *WordParser::ParseLiteral(StringList& terms)
{
  char* term = strdup(Term(terms));
  char* scope = 0;
  if(!mystrcasecmp(term, "literal")) {
    Shift(terms);
    scope = ParseScope(terms);
    free(term);
    term = strdup(Term(terms));
    Shift(terms);
  } else {
    scope = strdup("");
  }
  WordTreeLiteral *expr = new WordTreeLiteral(term, scope);
  Shift(terms);
  free(scope);
  free(term);
  return expr;
}

char *WordParser::ParseScope(StringList& terms)
{
  char *scope = Term(terms);
  int scope_length = strlen(scope);

  //
  // Remove surrounding quotes, if any
  //
  if(scope_length > 0) {
    if(scope[scope_length - 1] == '"')
      scope[--scope_length] = '\0';
    if(scope[0] == '"')
      scope++;
  }

  scope = strdup(scope);

  Shift(terms);

  return scope;
}

char *WordParser::Term(StringList& terms)
{
  char *term = terms[0];
  if(!term) {
    fprintf(stderr, "WordParser::Term: unexpected end of expression\n");
    exit(1);
  }
  return term;
}

void WordParser::Shift(StringList& terms)
{
  terms.Shift(LIST_REMOVE_DESTROY);
}

// ************************* main loop implementation ********************

//
// Store all options from the command line
//
class params_t
{
public:
  char* dbfile;
  char* find;
  unsigned int bottom;
  unsigned int count;
  char* context;
  int uniq_server;
  int proximity;
  int nop;
  int exclude;
};

//
// Explain options
//
static void usage();
//
// Torture WordExclude* classes
//
static void exclude_test();

int main(int ac, char **av)
{
  int			c;
  extern char		*optarg;
  params_t		params;

  params.dbfile = strdup("test");
  params.find = 0;
  params.bottom = 0;
  params.count = 10;
  params.context = 0;
  params.uniq_server = 0;
  params.proximity = WORD_SEARCH_DEFAULT_PROXIMITY;
  params.nop = 0;
  params.exclude = 0;

  while ((c = getopt(ac, av, "vB:f:b:c:C:SP:ne")) != -1)
    {
      switch (c)
	{
	case 'v':
	  verbose++;
	  break;
	case 'B':
	  free(params.dbfile);
	  params.dbfile = strdup(optarg);
	  break;
	case 'f':
	  params.find = strdup(optarg);
	  break;
	case 'b':
	  params.bottom = (unsigned int)atoi(optarg);
	  break;
	case 'c':
	  params.count = (unsigned int)atoi(optarg);
	  break;
	case 'C':
	  params.context = strdup(optarg);
	  break;
	case 'P':
	  params.proximity = atoi(optarg);
	  break;
	case 'S':
	  params.uniq_server = SERVER;
	  break;
	case 'n':
	  params.nop = 1;
	  break;
	case 'e':
	  params.exclude = 1;
	  break;
	case '?':
	  usage();
	  break;
	}
    }

  if(params.exclude) {
    exclude_test();
    exit(0);
  }

  if(!params.find)
    usage();

  Configuration* config = WordContext::Initialize();
  if(!config) {
    fprintf(stderr, "search: no config file found\n");
    exit(1);
  }

  //
  // Forward command line verbosity to htword library.
  //
  if(verbose > 1) {
    String tmp;
    tmp << (verbose - 1);
    config->Add("wordlist_verbose", tmp);
  }

  //
  // Prepare the index (-B).
  //
  WordList words(*config);
  words.Open(params.dbfile, O_RDONLY);

  //
  // Try the query parser alone
  //
  if(params.nop) {
    WordTree* expr = WordParser().Parse(params.find);
    printf("%s\n", (char*)expr->Get());
    exit(0);
  }

  //
  // Build a syntax tree from the expression provided by user
  //
  WordTree* expr = WordParser().Parse(params.find);

  //
  // Define the semantic of the key
  //
  {
#define DOCUMENT_LENGTH	3
    static int document[DOCUMENT_LENGTH] = {
      TAG,
      SERVER,
      URL
    };
    int document_length = DOCUMENT_LENGTH;
    int location = LOCATION;
    if(expr->Prepare(&words, params.uniq_server, params.proximity, document, document_length, location) != OK)
      exit(1);
  }

  WordSearch* search = new WordSearch();

  //
  // Forward query options to WordSearch object
  //
  search->limit_bottom = params.bottom;        // -b
  search->limit_count = params.count;          // -c
  if(params.context)                           // -C
    search->context_in.set(params.context, strlen(params.context));

  //
  // Perform the search (-f)
  //
  search->expr = expr;
  WordMatch* matches = search->Search();

  //
  // Display results, if any.
  //
  if(matches) {
    int i;
    for(i = 0; !matches[i].match.Empty(); i++)
      printf("match: %s\n", (char*)matches[i].Get());
    const String& context = search->Context();
    if(!context.empty())
      printf("context: %s\n", (const char*)context);
    delete [] matches;
  } else {
    printf("match: none\n");
  }

  //
  // Cleanup
  //
  delete search;
  if(params.context) free(params.context);
  if(params.find) free(params.find);
  if(params.dbfile) free(params.dbfile);
  delete expr;

  words.Close();
  delete config;
}

static void exclude_test()
{
  static unsigned int expected[] = {
    0x00000001,
    0x00000002,
    0x00000004,
    0x00000008,
    0x00000010,
    0x00000003,
    0x00000005,
    0x00000006,
    0x00000009,
    0x0000000a,
    0x0000000c,
    0x00000011,
    0x00000012,
    0x00000014,
    0x00000018,
    0x00000007,
    0x0000000b,
    0x0000000d,
    0x0000000e,
    0x00000013,
    0x00000015,
    0x00000016,
    0x00000019,
    0x0000001a,
    0x0000001c,
    0x0000000f,
    0x00000017,
    0x0000001b,
    0x0000001d,
    0x0000001e,
    0x0000001f
  };

  //
  // WordExclude
  //
  if(verbose) fprintf(stderr, "exclude_test: testing WordExclude\n");
  {
    WordExclude exclude;
    exclude.Initialize(5);
    int count = 0;
    while(exclude.Next() == WORD_EXCLUDE_OK) {
      if(expected[count] != exclude.Mask()) {
	fprintf(stderr, "exclude_test: WordExclude iteration %d expected 0x%08x but got 0x%08x\n", count, expected[count], exclude.Mask());
	exit(1);
      }
      count++;
    }
    if(count != sizeof(expected)/sizeof(unsigned int)) {
      fprintf(stderr, "exclude_test: WordExclude expected %d iterations but got %d\n", (int)(sizeof(expected)/sizeof(unsigned int)), count);
      exit(1);
    }
  }

  //
  // WordExcludeMask without ignore bits behaves exactly the same
  // as WordExclude.
  //
  if(verbose) fprintf(stderr, "exclude_test: testing WordExcludeMask behaving like WordExclude\n");
  {
    WordExcludeMask exclude;
    exclude.Initialize(5, 0, 0);
    int count = 0;
    while(exclude.Next() == WORD_EXCLUDE_OK) {
      if(expected[count] != exclude.Mask()) {
	fprintf(stderr, "exclude_test: WordExcludeMask 1 iteration %d expected 0x%08x but got 0x%08x\n", count, expected[count], exclude.Mask());
	exit(1);
      }
      count++;
    }
    if(count != sizeof(expected)/sizeof(unsigned int)) {
      fprintf(stderr, "exclude_test: WordExcludeMask 1 expected %d iterations but got %d\n", (int)(sizeof(expected)/sizeof(unsigned int)), count);
      exit(1);
    }
  }

  //
  // WordExcludeMask 
  //
  if(verbose) fprintf(stderr, "exclude_test: testing WordExcludeMask\n");
  {
    static unsigned int expected[] = {
      0x00000102,
      0x00000108,
      0x00000120,
      0x00000180,
      0x0000010a,
      0x00000122,
      0x00000128,
      0x00000182,
      0x00000188,
      0x000001a0,
      0x0000012a,
      0x0000018a,
      0x000001a2,
      0x000001a8,
      0x000001aa
    };
    static unsigned int excluded[] = {
      1,
      0,
      0,
      0,
      1,
      1,
      0,
      1,
      0,
      0,
      1,
      1,
      1,
      0,
      1
    };

    WordExcludeMask exclude;
    unsigned int ignore = 0x155;
    unsigned int ignore_mask = 0x100;
    exclude.Initialize(9, ignore, ignore_mask);
    if(verbose) {
      fprintf(stderr, "exclude_test: ignore\n");
      show_bits(ignore);
      fprintf(stderr, "exclude_test: ignore_mask\n");
      show_bits(ignore_mask);
    }
    if(exclude.NotExcludedCount() != 8) {
	fprintf(stderr, "exclude_test: WordExcludeMask 2 expected NoExcludedCount = 8 but got %d\n", exclude.NotExcludedCount());
	exit(1);
    }
    int count = 0;
    while(exclude.Next() == WORD_EXCLUDE_OK) {
      if(expected[count] != exclude.Mask()) {
	fprintf(stderr, "exclude_test: WordExcludeMask 2 iteration %d expected 0x%08x but got 0x%08x\n", count, expected[count], exclude.Mask());
	exit(1);
      }
      //
      // Test Excluded() method on ignored bit
      // Is bit 5 set ? (9 - 4) = 5 (counting from 1)
      //
      if(exclude.Excluded(4)) {
	fprintf(stderr, "exclude_test: WordExcludeMask 2 iteration %d bit 5 was set 0x%08x\n", count, exclude.Mask());
	exit(1);
      }
      //
      // Test Excluded() method on variable bit
      // Is bit 2 set ? (9 - 2) = 7 (counting from 1)
      //
      if((exclude.Excluded(7) && !excluded[count]) ||
	 (!exclude.Excluded(7) && excluded[count])) {
	fprintf(stderr, "exclude_test: WordExcludeMask 2 iteration %d expected bit 2 %s but was %s in 0x%08x\n", count, (excluded[count] ? "set" : "not set"), (excluded[count] ? "not set" : "set"), expected[count]);
	exit(1);
      }
      count++;
    }
    if(count != sizeof(expected)/sizeof(unsigned int)) {
      fprintf(stderr, "exclude_test: WordExcludeMask 2 expected %d iterations but got %d\n", (int)(sizeof(expected)/sizeof(unsigned int)), count);
      exit(1);
    }
  }

  {
    WordExclude exclude;
    String ascii("110101");
    String tmp;
    exclude.Set(ascii);
    exclude.Get(tmp);
    if(tmp != ascii) {
      fprintf(stderr, "exclude_test: WordExclude::Get/Set expected %s but got %s\n", (char*)ascii, (char*)tmp);
      exit(1);
    }
    if(exclude.Mask() != 0x2b) {
      fprintf(stderr, "exclude_test: WordExclude::Mask expected 0x2b but got 0x%02x\n", exclude.Mask());
      exit(1);
    }
  }
  {
    WordExcludeMask exclude;
    String ascii("12031");
    String tmp;
    exclude.Set(ascii);
    exclude.Get(tmp);
    if(tmp != ascii) {
      fprintf(stderr, "exclude_test: WordExcludeMask::Get/Set expected %s but got %s\n", (char*)ascii, (char*)tmp);
      exit(1);
    }
    if(exclude.Mask() != 0x19) {
      fprintf(stderr, "exclude_test: WordExcludeMask::Mask expected 0x19 but got 0x%02x\n", exclude.Mask());
      exit(1);
    }
  }
}

// *****************************************************************************
// void usage()
//   Display program usage information
//
static void usage()
{
    printf("usage:\tsearch -f words [options]\n");
    printf("\tsearch -e\n");
    printf("Options:\n");
    printf("\t-v\t\tIncreases the verbosity.\n");
    printf("\t-B dbfile\tUse <dbfile> as a db file name (default test).\n");
    printf("\t-f expr\t\tLisp like search expression.\n");
    printf("\t\t\tSee WordParser comments in source for more information.\n");
    printf("\t-b number\tSkip number documents before retrieving.\n");
    printf("\t-c number\tRetrieve number documents at most.\n");
    printf("\t-n\t\tOnly parse the search expression and print it.\n");
    printf("\t-P proximity\tUse with near/optional, proximity tolerance is <proximity>\n");
    printf("\t\t\tif negative order of terms is not meaningful\n");
    printf("\t\t\t(default 1).\n");
    printf("\t-C context\tResume search at <context>.\n");
    printf("\t-S\t\tReturn at most one match per server.\n");
    printf("\n");
    printf("\t-e\t\tRun tests on WordExclude and WordExcludeMask.\n");
    exit(1);
}
