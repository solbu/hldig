// WordKey.h
//
// NAME
// inverted index key.
//
// SYNOPSIS
//
// #include <WordKey.h>
// 
// #define WORD_KEY_DOCID    1
// #define WORD_KEY_LOCATION 2
//
// WordList* words = ...;
// WordKey key = words->Key("word 100 20");
// WordKey searchKey;
// words->Dict()->SerialExists("dog", searchKey.Get(WORD_KEY_WORD));
// searchKey.Set(WORD_KEY_LOCATION, 5);
// WordCursor* cursor = words->Key(searchKey);
// 
// DESCRIPTION
//
// Describes the key used to store a entry in the inverted index.
// Each field in the key has a bit in the <b>set</b>
// member that says if it is set or not. This bit allows to
// say that a particular field is <i>undefined</i> regardless of
// the actual value stored. The methods
// <b>IsDefined, SetDefined</b> and <b>Undefined</b> are used to manipulate
// the <i>defined</i> status of a field. The <b>Pack</b> and <b>Unpack</b>
// methods are used to convert to and from the disk storage representation
// of the key. 
// 
// Although constructors may be used, the prefered way to create a 
// WordKey object is by using the <b>WordContext::Key</b> method.
//
// The following constants are defined:
// <dl>
// <dt> WORD_KEY_WORD
// <dd> the index of the word identifier with the key for Set and Get
// methods.
// <dt> WORD_KEY_VALUE_INVALID
// <dd> a value that is invalid for any field of the key.
// </dl>
//
// ASCII FORMAT
//
// The ASCII description is a string with fields separated by tabs or
// white space.
// <pre>
// Example: 200 <UNDEF> 1 4 2
// Field 1: The word identifier or <UNDEF> if not defined
// Field 2 to the end: numerical value of the field or <UNDEF> if
//                     not defined
//
// </pre>
//
// END
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
//

#ifndef _WordKey_h_
#define _WordKey_h_

#ifndef SWIG
#include "db.h"
#include "htString.h"
#include "StringList.h"
#include "WordContext.h"
#endif /* SWIG */

//
// Possible return values of Outbound/Overflow/Underflow methods
//
#define WORD_INBOUND	0
#define WORD_OVERFLOW	1
#define WORD_UNDERFLOW	2

//
// Possible return values of SetToFollowing
//
#define WORD_FOLLOWING_ATEND	0x0001
//
// Default value for position argument of SetToFollowing
// meaning NFields() - 1
//
#define WORD_FOLLOWING_MAX	-1

//
// No value in a key may be 0
//
#define WORD_KEY_VALUE_INVALID 0

//
// Unknown field position
//
#define WORD_KEY_UNKNOWN_POSITION	-1

//
// Index of the word identifier within the key
// 
#define WORD_KEY_WORD	0

#ifndef SWIG
//
// C comparison function interface for Berkeley DB (bt_compare)
//
int word_db_cmp(const DBT *a, const DBT *b);
#endif /* SWIG */

#ifndef SWIG
#include"WordKeyInfo.h"
#endif /* SWIG */

//
// Describe a word occurrence
//
class WordKey
{
 public:
  //
  // Constructors, destructors, copy and clear 
  //
  //-
  // Constructor. Build an empty key.
  // The <b>ncontext</b> argument must be a pointer to a valid
  // WordContext object.
  //
  WordKey(WordContext* ncontext) {
    context = ncontext;
    Clear();
  }
#ifndef SWIG
  //-
  // Constructor. Initialize from an ASCII description of a key.
  // See <i>ASCII FORMAT</i> section.
  // The <b>ncontext</b> argument must be a pointer to a valid
  // WordContext object.
  //
  WordKey(WordContext* ncontext, const String& desc) {
    context = ncontext;
    Set(desc); 
  }
 public:
#endif /* SWIG */
  //-
  // Reset to empty key. 
  //
  void	Clear() { 
    setbits = 0;
    for(int i = 0; i < NFields(); i++) {
      values[i] = 0;
    }
  }

  //-
  // Convenience functions to access the total number of fields
  // in a key (see <i>WordKeyInfo(3)</i>).
  //
  inline int 	           NFields() const { return context->GetKeyInfo().nfields; }
  //-
  // Convenience functions to access the 
  // maximum possible value for field at <b>position.</b>
  // in a key (see <i>WordKeyInfo(3)</i>).
  //
  inline WordKeyNum         MaxValue(int position) { return context->GetKeyInfo().MaxValue(position); }

  //
  // Accessors
  //
  //-
  // Return a pointer to the WordContext object used to create
  // this instance.
  //
  inline WordContext* GetContext() { return context; }
#ifndef SWIG
  //-
  // Return a pointer to the WordContext object used to create
  // this instance as a const.
  //
  inline const WordContext* GetContext() const { return context; }
#endif /* SWIG */

  //
  // Get/Set fields
  //
  //-
  // Return value of numerical field at <b>position</b> as const.
  //
  inline WordKeyNum Get(int position) const {
    return(values[position]);
  }
#ifndef SWIG
  //-
  // Return value of numerical field at <b>position.</b>
  //
  inline WordKeyNum& Get(int position) {
    return(values[position]);
  }
  //-
  // Return value of numerical field at <b>position</b> as const.
  //
  inline const WordKeyNum &      operator[] (int position) const  { return(values[position]); }
  //-
  // Return value of numerical field at <b>position.</b>
  //
  inline       WordKeyNum &      operator[] (int position)        { return(values[position]); }
#endif /* SWIG */
  //-
  // Set value of numerical field at <b>position</b> to <b>val.</b>
  //
  inline void Set(int position, WordKeyNum val) {
    SetDefined(position);
    values[position] = val;
  }
    
  //
  // Key field value existenz. Defined means the value of the field contains
  // a valid value. Undefined means the value of the field is not valid.
  //
  //-
  // Returns true if field at <b>position</b> is <i>defined</i>, false
  // otherwise.
  //
  int	IsDefined(int position) const { return setbits & (1 << position); }
  //-
  // Value in field <b>position</b> becomes <i>defined.</i>
  //
  void	SetDefined(int position)      { setbits |= (1 << position); }
  //-
  // Value in field <b>position</b> becomes <i>undefined.</i>
  //
  void	Undefined(int position)       { setbits &= ~(1 << position); }

#ifndef SWIG
  //
  // Set and Get the whole structure from/to ASCII description
  //-
  // Set the whole structure from ASCII string in <b>bufferin.</b>
  // See <i>ASCII FORMAT</i> section.
  // Return OK if successfull, NOTOK otherwise.
  //
  int Set(const String& bufferin);
  int SetList(StringList& fields);
  //-
  // Convert the whole structure to an ASCII string description 
  // in <b>bufferout.</b>
  // See <i>ASCII FORMAT</i> section.
  // Return OK if successfull, NOTOK otherwise.
  //
  int Get(String& bufferout) const;
  //-
  // Convert the whole structure to an ASCII string description 
  // and return it.
  // See <i>ASCII FORMAT</i> section.
  // 
  String Get() const;
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
  int 		Unpack(const char* string, int length);
  //
  //-
  // Set structure from disk storage format as found in 
  // <b>data</b> string.
  // Return OK if successfull, NOTOK otherwise.
  //
  inline int    Unpack(const String& data) { return(Unpack(data,data.length())); }
  //
  //-
  // Convert object into disk storage format as found in 
  // and place the result in <b>data</b> string.
  // Return OK if successfull, NOTOK otherwise.
  //
  int 		Pack(String& data) const;
#endif /* SWIG */

  //
  // Transformations
  //
  //-
  // Copy each <i>defined</i> field from other into the object, if 
  // the corresponding field of the object is not defined. 
  // Return OK if successfull, NOTOK otherwise.
  //
  int		Merge(const WordKey& other);
  //-
  // Undefine all fields found after the first undefined field. The
  // resulting key has a set of defined fields followed by undefined fields.
  // Returns NOTOK if the word is not defined because the resulting key would 
  // be empty and this is considered an error. Returns OK on success.
  //
  int		PrefixOnly();
#ifndef SWIG
  //-
  // Implement ++ on a key.
  //
  // It behaves like arithmetic but follows these rules:
  // <pre>
  // . Increment starts at field <position>
  // . If a field value overflows, increment field <b>position</b> - 1
  // . Undefined fields are ignored and their value untouched
  // . When a field is incremented all fields to the left are set to 0
  // </pre>
  // If position is not specified it is equivalent to NFields() - 1.
  // It returns OK if successfull, NOTOK if <b>position</b> out of range or
  // WORD_FOLLOWING_ATEND if the maximum possible value was reached.
  //
  int           SetToFollowing(int position = WORD_FOLLOWING_MAX);
#endif /* SWIG */

  //
  // Predicates
  //
  //-
  // Return true if all the fields are <i>defined</i>, false otherwise.
  //
  int		Filled() const { return setbits == (unsigned int) (((1 << NFields()) - 1)); }
  //-
  // Return true if no fields are <i>defined</i>, false otherwise.
  //
  int		Empty() const  { return setbits == 0; }
  //-
  // Return true if the object and <b>other</b> are equal. 
  // Only fields defined in both keys are compared.
  //
  int 		Equal(const WordKey& other) const;
  //-
  // Return true if the object and <b>other</b> are equal. 
  // All fields are compared. If a field is defined in <b>object</b>
  // and not defined in the object, the key are not considered
  // equal.
  //
  int 		ExactEqual(const WordKey& other) const { return(Equal(other) && other.setbits == setbits); }
#ifndef SWIG
  //-
  // Return true if the object and <b>other</b> are equal. 
  // The packed string are compared. An <i>undefined</i> numerical field 
  // will be 0 and therefore undistinguishable from a <i>defined</i> field
  // whose value is 0.
  //
  int 		PackEqual(const WordKey& other) const;
  //-
  // Return true if adding <b>increment</b> in field at <b>position</b> makes
  // it overflow or underflow, false if it fits.
  //
  int		Outbound(int position, int increment) {
    if(increment < 0) return Underflow(position, increment);
    else if(increment > 0) return Overflow(position, increment);
    else return WORD_INBOUND;
  }
  //-
  // Return true if adding positive <b>increment</b> to field at 
  // <b>position</b> makes it overflow, false if it fits.
  //
  int		Overflow(int position, int increment) {
    return MaxValue(position) - Get(position) < (WordKeyNum)increment ? WORD_OVERFLOW : WORD_INBOUND;
  }
  //-
  // Return true if subtracting positive <b>increment</b> to field 
  // at <b>position</b> makes it underflow, false if it fits.
  //
  int		Underflow(int position, int increment) {
    return Get(position) < (WordKeyNum)(-increment) ? WORD_UNDERFLOW : WORD_INBOUND;
  }
#endif /* SWIG */
  //-
  // Return OK if the key may be used as a prefix for search.
  // In other words return OK if the fields set in the key
  // are all contiguous, starting from the first field.
  // Otherwise returns NOTOK
  //
  int		Prefix() const;

#ifndef SWIG
  //-
  // Compare <b>a</b> and <b>b</b> in the Berkeley DB fashion. 
  // <b>a</b> and <b>b</b> are packed keys. The semantics of the
  // returned int is as of strcmp and is driven by the key description
  // found in <i>WordKeyInfo.</i> Returns positive number if <b>a</b> is
  // greater than <b>b</b>, zero if they are equal, a negative number 
  // if <b>a</b> is lower than <b>b.</b>
  //
  static int 	    Compare(WordContext* context, const String& a, const String& b);
  //-
  // Compare <b>a</b> and <b>b</b> in the Berkeley DB fashion. 
  // <b>a</b> and <b>b</b> are packed keys. The semantics of the
  // returned int is as of strcmp and is driven by the key description
  // found in <i>WordKeyInfo.</i> Returns positive number if <b>a</b> is
  // greater than <b>b</b>, zero if they are equal, a negative number 
  // if <b>a</b> is lower than <b>b.</b>
  //
  static int        Compare(WordContext* context, const unsigned char *a, int a_length, const unsigned char *b, int b_length);
  //-
  // Compare object defined fields with <b>other</b> key defined fields only,
  // ignore fields that are not defined in object or <b>other.</b> 
  // Return 1 if different 0 if equal. 
  // If different, <b>position</b> is set to the field number that differ,
  // <b>lower</b> is set to 1 if Get(<b>position</b>) is lower than
  // other.Get(<b>position</b>) otherwise lower is set to 0.
  //
  int               Diff(const WordKey& other, int& position, int& lower);

  //-
  // Print object in ASCII form on <b>f</b> (uses <i>Get</i> method).
  // See <i>ASCII FORMAT</i> section.
  //
  int Write(FILE* f) const;
#endif /* SWIG */
  //-
  // Print object in ASCII form on <b>stdout</b> (uses <i>Get</i> method).
  // See <i>ASCII FORMAT</i> section.
  //
  void Print() const;

#ifndef SWIG

private:

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
  WordKeyNum   values[WORD_KEY_MAX_NFIELDS];

  WordContext  *context;
#endif /* SWIG */
};

#endif /* _WordKey_h */
