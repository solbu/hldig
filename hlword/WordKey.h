// WordKey.h
//
// NAME
// inverted index key.
//
// SYNOPSIS
//
// #include <WordKey.h>
// 
// #define DOCID 1
// #define LOCATION 1
//
// WordKey key("word <DEF> 1 2");
// key.Set(DOCID, 100);
// key.SetWord("other");
//
// DESCRIPTION
//
// Describes the key used to store a entry in the inverted index.
// The structure of a key is described by the <i>WordKeyInfo</i>
// Each field in the key has a bit in the <b>set</b>
// member that says if it is set or not. This bit allows to
// say that a particular field is <i>undefined</i> regardless of
// the actual value stored. The methods
// <b>IsDefined, SetDefined</b> and <b>Undefined</b> are used to manipulate
// the <i>defined</i> status of a field. The <b>Pack</b> and <b>Unpack</b>
// methods are used to convert to and from the disk storage representation
// of the key. 
// 
// Generic functions to manipulate the key should use the <i>WordKeyInfo</i>
// information to work regardless of the actual structure of the key.
//
// Suffix definition: a word suffix is a kind of marker that says if
// the word is a full word or only the beginning of a
// word. If a word has a suffix then it's a full word. If it
// has no suffix then it's only the beginning of a word.
// This is mostly useful when specifying search keys. If a
// search key word has no suffix, the search mechanism is
// expected to return all words that begin with the word. If
// the search key word has a suffix, only words that exactly
// match the search key word will be returned.
//
// ASCII FORMAT
//
// The ASCII description is a string with fields separated by tabs or
// white space.
// <pre>
// Example: Foo <DEF> 0 1 4 2
// Field 1: The word as a string or <UNDEF> if not defined
// Field 2: <DEF> if suffix defined, <UNDEF> if suffix undefined
// Field 3 to nfield + 1: numerical value of the field or <UNDEF> if
//                        not defined
//
// </pre>
//
// END
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
//

#ifndef _WordKey_h_
#define _WordKey_h_

#ifndef SWIG
#include "db.h"
#include "htString.h"
#include "StringList.h"
#endif /* SWIG */

//
// WORDSUFFIX:
// 
// field in set flag that says if a word is just a prefix (incomplete word)
// WORD_KEY_WORDSUFFIX_DEFINED -> means that word IS complete (not a prefix)
//
#define WORD_KEY_WORDSUFFIX_DEFINED   (1 << 30)
#define WORD_KEY_WORD_DEFINED     1
#define WORD_KEY_WORDFULLY_DEFINED   ( WORD_KEY_WORDSUFFIX_DEFINED | WORD_KEY_WORD_DEFINED )

//
// Possible return values of Outbound/Overflow/Underflow methods
//
#define WORD_INBOUND  0
#define WORD_OVERFLOW  1
#define WORD_UNDERFLOW  2

//
// Possible return values of SetToFollowing
//
#define WORD_FOLLOWING_ATEND  0x0001
//
// Default value for position argument of SetToFollowing
// meaning NFields() - 1
//
#define WORD_FOLLOWING_MAX  -1

//
// Position of the first numerical field (just after the word)
//
#define WORD_FIRSTFIELD  1

//
// Unknown field position
//
#define WORD_KEY_UNKNOWN_POSITION  -1

#ifndef SWIG
// C comparison function interface for Berkeley DB (bt_compare)
//
int word_db_cmp (const DBT * a, const DBT * b);
int word_only_db_cmp (const DBT * a, const DBT * b);
#endif /* SWIG */

#ifndef SWIG
#include"WordKeyInfo.h"
#endif /* SWIG */

//
// Describe a word occurrence
//
// !!!!!!!DEBUGTMP
#ifndef SWIG
#define WORD_FATAL_ABORT fflush(stdout);fprintf(stderr,"FATAL ERROR at file:%s line:%d !!!\n",__FILE__,__LINE__);fflush(stderr);abort()
#define word_errr(s) {fprintf(stderr,"FATAL ERROR:%s\n",s);WORD_FATAL_ABORT;}
#endif /* SWIG */
class WordKey
{
public:
  //
  // Constructors, destructors, copy and clear 
  //
  //-
  // Constructor. Build an empty key.
  //
  WordKey ()
  {
    Initialize ();
  }
#ifndef SWIG
  //-
  // Constructor. Initialize from an ASCII description of a key.
  // See <i>ASCII FORMAT</i> section.
  //
  WordKey (const String & word)
  {
    Initialize ();
    Set (word);
  }
  //
  // Copy constructor (needed because of the array pointer)
  //
  WordKey (const WordKey & other)
  {
    Initialize ();
    CopyFrom (other);
  }
#endif /* SWIG */
  ~WordKey ()
  {
    delete[]numerical_fields;
  }
#ifndef SWIG
protected:
  //
  // Constructor helper, allocate members and set to empty key
  //
  void Initialize ()
  {
    if (!Info ())
    {
      fprintf (stderr, "WordKey::WordKey used before word_key_info set\n");
      word_errr ("WordKey::initialize");
    }

    numerical_fields = new WordKeyNum[NFields () - 1];
    Clear ();
  }
public:
  //
  // Copy operator (needed because of the array pointer)
  //
  void operator = (const WordKey & other)
  {
    Clear ();
    CopyFrom (other);
  }
#endif /* SWIG */
  //-
  // Copy <b>other</b> into object.
  //
  void CopyFrom (const WordKey & other)
  {
    if (other.IsDefined (0))
    {
      SetWord (other.GetWord ());
    }
    for (int i = 1; i < NFields (); i++)
    {
      if (other.IsDefined (i))
      {
        Set (i, other.Get (i));
      }
    }
    setbits = other.setbits;
  }
  //-
  // Reset to empty key. 
  //
  void Clear ()
  {
    setbits = 0;
    kword.trunc ();
    for (int i = 0; i < NFields () - 1; i++)
    {
      numerical_fields[i] = 0;
    }
  }

#ifndef SWIG
  //-
  // Convenience function to access key structure
  // information (see <i>WordKeyInfo(3)</i>).
  //
  static inline const WordKeyInfo *Info ()
  {
    return WordKeyInfo::Instance ();
  }
#endif /* SWIG */
  //-
  // Convenience functions to access the total number of fields
  // in a key (see <i>WordKeyInfo(3)</i>).
  //
  static inline int NFields ()
  {
    return Info ()->nfields;
  }
  //-
  // Convenience functions to access the 
  // maximum possible value for field at <b>position.</b>
  // in a key (see <i>WordKeyInfo(3)</i>).
  //
  static inline WordKeyNum MaxValue (int position)
  {
    return Info ()->sort[position].MaxValue ();
  }

  //
  // Accessors
  //
  //-
  // Returns the word as a const.
  //
#ifndef SWIG
  inline const String & GetWord () const
  {
    return kword;
  }
#endif                          /* SWIG */

  //-
  // Returns the word.
  //
  inline String & GetWord ()
  {
    return kword;
  }
  //-
  // Set the word.
  //
  inline void SetWord (const String & arg)
  {
    kword = arg;
    setbits |= WORD_KEY_WORDFULLY_DEFINED;
  }
protected:
  //-
  // Set the word.
  //
  inline void SetWord (const char *arg, int arg_length)
  {
    kword.set (arg, arg_length);
    setbits |= WORD_KEY_WORDFULLY_DEFINED;
  }
public:
  //-
  // Change status of the word to <i>undefined.</i> Also undefines
  // its suffix.
  //
  inline void UndefinedWord ()
  {
    kword.trunc ();
    setbits &= ~WORD_KEY_WORDFULLY_DEFINED;
  }
  //-
  // Set the status of the word suffix to <i>undefined.</i> 
  //
  inline void UndefinedWordSuffix ()
  {
    setbits &= ~WORD_KEY_WORDSUFFIX_DEFINED;
  }
  //-
  // Set the status of the word suffix to <i>defined.</i> 
  //
  inline void SetDefinedWordSuffix ()
  {
    setbits |= WORD_KEY_WORDSUFFIX_DEFINED;
  }
  //-
  // Returns true if word suffix is <i>defined</i>, false otherwise.
  //
  inline int IsDefinedWordSuffix () const
  {
    return ((setbits & WORD_KEY_WORDSUFFIX_DEFINED) ==
            WORD_KEY_WORDSUFFIX_DEFINED);
  }
  //
  // Get/Set numerical fields
  //
  //-
  // Return value of numerical field at <b>position</b> as const.
  //
  inline WordKeyNum Get (int position) const
  {
    // if(position<1 || position>=NFields()){errr("Get: out of bounds");}
    return (numerical_fields[position - 1]);
  }
#ifndef SWIG
  //-
  // Return value of numerical field at <b>position.</b>
  //
  inline WordKeyNum & Get (int position)
  {
    return (numerical_fields[position - 1]);
  }
  //-
  // Return value of numerical field at <b>position</b> as const.
  //
  inline const WordKeyNum & operator[] (int position) const
  {
    return (numerical_fields[position - 1]);
  }
  //-
  // Return value of numerical field at <b>position.</b>
  //
  inline WordKeyNum & operator[] (int position)
  {
    return (numerical_fields[position - 1]);
  }
#endif /* SWIG */
  //-
  // Set value of numerical field at <b>position</b> to <b>val.</b>
  //
  inline void Set (int position, WordKeyNum val)
  {
    // if(position<1 || position>=NFields()){errr("Set: out of bounds");}
    SetDefined (position);
    numerical_fields[position - 1] = val;
  }

  //
  // Key field value existenz. Defined means the value of the field contains
  // a valid value. Undefined means the value of the field is not valid.
  //
  //-
  // Returns true if field at <b>position</b> is <i>defined</i>, false
  // otherwise.
  //
  int IsDefined (int position) const
  {
    return setbits & (1 << position);
  }
  //-
  // Value in field <b>position</b> becomes <i>defined.</i>
  //
  void SetDefined (int position)
  {
    setbits |= (1 << position);
  }
  //-
  // Value in field <b>position</b> becomes <i>undefined.</i>
  //
  void Undefined (int position)
  {
    setbits &= ~(1 << position);
  }

#ifndef SWIG
  //
  // Set and Get the whole structure from/to ASCII description
  //-
  // Set the whole structure from ASCII string in <b>bufferin.</b>
  // See <i>ASCII FORMAT</i> section.
  // Return OK if successfull, NOTOK otherwise.
  //
  int Set (const String & bufferin);
  int SetList (StringList & fields);
  //-
  // Convert the whole structure to an ASCII string description 
  // in <b>bufferout.</b>
  // See <i>ASCII FORMAT</i> section.
  // Return OK if successfull, NOTOK otherwise.
  //
  int Get (String & bufferout) const;
  //-
  // Convert the whole structure to an ASCII string description 
  // and return it.
  // See <i>ASCII FORMAT</i> section.
  // 
  String Get () const;
#endif /* SWIG */

  //
  // Storage format conversion
  //
#ifndef SWIG
  //-
  // Set structure from disk storage format as found in 
  // <b>string</b> buffer or length <b>length.</b>
  // Return OK if successfull, NOTOK otherwise.
  //
  int Unpack (const char *string, int length);
  //
  //-
  // Set structure from disk storage format as found in 
  // <b>data</b> string.
  // Return OK if successfull, NOTOK otherwise.
  //
  inline int Unpack (const String & data)
  {
    return (Unpack (data, data.length ()));
  }
  //
  //-
  // Convert object into disk storage format as found in 
  // and place the result in <b>data</b> string.
  // Return OK if successfull, NOTOK otherwise.
  //
  int Pack (String & data) const;
#endif /* SWIG */

  //
  // Transformations
  //
  //-
  // Copy each <i>defined</i> field from other into the object, if 
  // the corresponding field of the object is not defined. 
  // Return OK if successfull, NOTOK otherwise.
  //
  int Merge (const WordKey & other);
  //-
  // Undefine all fields found after the first undefined field. The
  // resulting key has a set of defined fields followed by undefined fields.
  // Returns NOTOK if the word is not defined because the resulting key would 
  // be empty and this is considered an error. Returns OK on success.
  //
  int PrefixOnly ();
#ifndef SWIG
  //-
  // Implement ++ on a key.
  //
  // It behaves like arithmetic but follows these rules:
  // <pre>
  // . Increment starts at field <position>
  // . If a field value overflows, increment field <b>position</b> - 1
  // . Undefined fields are ignored and their value untouched
  // . Incrementing the word field is done by appending \001
  // . When a field is incremented all fields to the left are set to 0
  // </pre>
  // If position is not specified it is equivalent to NFields() - 1.
  // It returns OK if successfull, NOTOK if <b>position</b> out of range or
  // WORD_FOLLOWING_ATEND if the maximum possible value was reached.
  //
  int SetToFollowing (int position = WORD_FOLLOWING_MAX);
#endif /* SWIG */

  //
  // Predicates
  //
  //-
  // Return true if all the fields are <i>defined</i>, false otherwise.
  //
  int Filled () const
  {
    return setbits ==
      (unsigned int) (((1 << NFields ()) - 1) | WORD_KEY_WORDSUFFIX_DEFINED);
  }
  //-
  // Return true if no fields are <i>defined</i>, false otherwise.
  //
  int Empty () const
  {
    return setbits == 0;
  }
  //-
  // Return true if the object and <b>other</b> are equal. 
  // Only fields defined in both keys are compared.
  //
  int Equal (const WordKey & other) const;
  //-
  // Return true if the object and <b>other</b> are equal. 
  // All fields are compared. If a field is defined in <b>object</b>
  // and not defined in the object, the key are not considered
  // equal.
  //
  int ExactEqual (const WordKey & other) const
  {
    return (Equal (other) && other.setbits == setbits);
  }
#ifndef SWIG
  //-
  // Return true if the object and <b>other</b> are equal. 
  // The packed string are compared. An <i>undefined</i> numerical field 
  // will be 0 and therefore undistinguishable from a <i>defined</i> field
  // whose value is 0.
  //
  int PackEqual (const WordKey & other) const;
  //-
  // Return true if adding <b>increment</b> in field at <b>position</b> makes
  // it overflow or underflow, false if it fits.
  //
  int Outbound (int position, int increment)
  {
    if (increment < 0)
      return Underflow (position, increment);
    else if (increment > 0)
      return Overflow (position, increment);
    else
      return WORD_INBOUND;
  }
  //-
  // Return true if adding positive <b>increment</b> to field at 
  // <b>position</b> makes it overflow, false if it fits.
  //
  int Overflow (int position, int increment)
  {
    return MaxValue (position) - Get (position) <
      (WordKeyNum) increment ? WORD_OVERFLOW : WORD_INBOUND;
  }
  //-
  // Return true if subtracting positive <b>increment</b> to field 
  // at <b>position</b> makes it underflow, false if it fits.
  //
  int Underflow (int position, int increment)
  {
    return Get (position) <
      (WordKeyNum) (-increment) ? WORD_UNDERFLOW : WORD_INBOUND;
  }
#endif /* SWIG */
  //-
  // Return OK if the key may be used as a prefix for search.
  // In other words return OK if the fields set in the key
  // are all contiguous, starting from the first field.
  // Otherwise returns NOTOK
  //
  int Prefix () const;

#ifndef SWIG
  //-
  // Compare <b>a</b> and <b>b</b> in the Berkeley DB fashion. 
  // <b>a</b> and <b>b</b> are packed keys. The semantics of the
  // returned int is as of strcmp and is driven by the key description
  // found in <i>WordKeyInfo.</i>
  //
  static int Compare (const String & a, const String & b);
  static int Compare_WordOnly (const String & a, const String & b);
  //-
  // Compare <b>a</b> and <b>b</b> in the Berkeley DB fashion. 
  // <b>a</b> and <b>b</b> are packed keys. The semantics of the
  // returned int is as of strcmp and is driven by the key description
  // found in <i>WordKeyInfo.</i>
  //
  static int Compare (const char *a, int a_length, const char *b,
                      int b_length);
  static int Compare_WordOnly (const char *a, int a_length, const char *b,
                               int b_length);
  //-
  // Compare object defined fields with <b>other</b> key defined fields only,
  // ignore fields that are not defined in object or <b>other.</b> 
  // Return 1 if different 0 if equal. 
  // If different, <b>position</b> is set to the field number that differ,
  // <b>lower</b> is set to 1 if Get(<b>position</b>) is lower than
  // other.Get(<b>position</b>) otherwise lower is set to 0.
  //
  int Diff (const WordKey & other, int &position, int &lower);

  //-
  // Print object in ASCII form on <b>f</b> (uses <i>Get</i> method).
  // See <i>ASCII FORMAT</i> section.
  //
  int Write (FILE * f) const;
#endif /* SWIG */
  //-
  // Print object in ASCII form on <b>stdout</b> (uses <i>Get</i> method).
  // See <i>ASCII FORMAT</i> section.
  //
  void Print () const;

#ifndef SWIG

private:

  //
  // Convert a single number from and to disk storage representation
  //
  static int UnpackNumber (const unsigned char *from, const int from_size,
                           WordKeyNum & res, const int lowbits,
                           const int bits);
  static int PackNumber (WordKeyNum from, char *to, int to_size, int lowbits,
                         int lastbits);

  //
  // Data members
  //
  //
  // Bit field for defined/undefined status of each key field
  //
  unsigned int setbits;
  //
  // Holds the numerical values of the key fields
  //
  WordKeyNum *numerical_fields;
  //
  // Holds the word key field
  //
  String kword;
#endif /* SWIG */
};

#ifndef SWIG
//
// Set bit number <b> to 0 and others to 1. <b> may have a value from 0 to 8. If
// 8 then all bits are 1.
//
#define WORD_BIT_MASK(b) ((b) == 0 ? 0xff : ((( 1 << (b)) - 1) & 0xff))
#define WORD_BIT_MASK2(b) ((1<<(b)) -1)
//
// Decode integer found in <from> using <from_size> bytes. The integer starts at <lowbits> bit
// in the first byte and occupies a total of <bits> bits. The resulting integer is stored in *<top>
//
inline int
WordKey::UnpackNumber (const unsigned char *from, const int from_size,
                       WordKeyNum & to, const int lowbits, const int bits)
{
  to = 0;
  to = ((from[0] & 0xff) >> lowbits);

  if (lowbits)
    to &= WORD_BIT_MASK (8 - lowbits);

  if (from_size == 1)
    to &= WORD_BIT_MASK (bits);
  else
  {
    for (int i = 1; i < from_size; i++)
    {
      to |= (from[i] & 0xff) << ((i - 1) * 8 + (8 - lowbits));
    }
  }

  if (bits < (int) (sizeof (WordKeyNum) * 8))
    to &= (1 << bits) - 1;

  return OK;
}

//
// Encode integer <from>, starting at bit <lowbits> in byte array <to>. It will span
// <to_size> bytes and only the <lastbits> bits of the last byte (to[to_size - 1]) are 
// filled. See word_builder.pl for more information.
//
inline int
WordKey::PackNumber (WordKeyNum from, char *to, int to_size, int lowbits,
                     int lastbits)
{
  // first byte
  if (lowbits)
  {
    to[0] |= ((from & WORD_BIT_MASK (8 - lowbits)) << lowbits) & 0xff;
  }
  else
  {
    to[0] = from & 0xff;
  }
  from >>= 8 - lowbits;

  // following bytes
  for (int i = 1; i < to_size; i++)
  {
    to[i] = from & 0xff;
    from >>= 8;
  }

  // clip the end off (clobbers anything left at the end of this byte)
  if (lastbits)
    to[to_size - 1] &= WORD_BIT_MASK (lastbits);

  return OK;
}

#undef WORD_BIT_MASK
#endif /* SWIG */

#endif
