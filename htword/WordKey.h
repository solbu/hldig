// WARNING : this file was generated from WordKey.h.tmpl
// by word_builder.pl using instructions from word.desc
//
// WordKey.h
//
// WordKey: Describes the key used to store a word in the word database.
//          The fields may be unsigned char, unsigned short, unsigned int or String
//          (see htString.h). 
//          The exact content of the key structure is defined by the file used
//          by word_builder.pl to build the actual WordKey.h from WordKey.h.tmpl (word.desc).
//          The class implements the in-memory form of the key, with accessors (Set, Get, Undefined)
//          and specialized accessors (SetWord, SetFlags...) according to the actual
//          definition of the key.
//          Each field has a bit in the 'set' member that says if it is set or not. This
//          bit allows to say that a particular field is 'undefined' regardless of the actual
//          value stored in the byte. The members IsDefined, SetDefined, Undefined are used to manipulate
//          the 'defined' status of a field.
//          The Pack and Unpack functions are used to convert to and from the disk storage of
//          the key.
//          To allow implementation that does not depend on compile time key structure the
//          word_key_info object contains meta information about the key. Generic functions
//          may be written using this object so that they work regardless of the actual
//          structure of the key.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
//

#ifndef _WordKey_h_
#define _WordKey_h_

#ifndef SWIG
#include "db.h"
#include "htString.h"
#endif /* SWIG */

#define	WORD_KEY_LOCATION	0
#define	WORD_KEY_FLAGS	1
#define	WORD_KEY_DOCID	2
#define	WORD_KEY_WORD	0


#define	WORD_KEY_LOCATION_POSITION	 0
#define	WORD_KEY_LOCATION_DEFINED	 (1 << 0)
#define	WORD_KEY_FLAGS_POSITION	 1
#define	WORD_KEY_FLAGS_DEFINED	 (1 << 1)
#define	WORD_KEY_DOCID_POSITION	 2
#define	WORD_KEY_DOCID_DEFINED	 (1 << 2)
#define	WORD_KEY_WORD_POSITION	 3
#define	WORD_KEY_WORD_DEFINED	 (1 << 3)

// WORDSUFFIX:
// field in set flag that says if a word is just a prefix (incomplete word)
// WORD_KEY_WORDSUFFIX_DEFINED -> means that word IS complete (not a prefix)
#define  WORD_KEY_WORDSUFFIX_DEFINED (1 << 30)
#define WORD_KEY_WORDFULLY_DEFINED ( WORD_KEY_WORDSUFFIX_DEFINED | WORD_KEY_WORD_DEFINED )


#define WORD_KEY_MAX_NFIELDS 20

#ifndef SWIG
// C comparison function interface for Berkeley DB (bt_compare)
//
int word_db_cmp(const DBT *a, const DBT *b);
#endif /* SWIG */

//
// Symbolic names for numerical type used
//
typedef unsigned int TypeA;


//
// Type number associated to each possible type for a key element
// (type field of struct WordKeyInfo).
//
#define WORD_ISA_TypeA		1
#define WORD_ISA_String		2


#ifndef SWIG
//
// Describes the structure of the key, ie meta information
// for the key. This includes the layout of the packed version
// stored on disk.
//
struct WordKeyInfo {
  //
  // Array describing the fields, in encoding order. 
  //
#define WORD_SORT_ASCENDING	1
#define WORD_SORT_DESCENDING	2

  struct {
    char* name;				// Symbolic name of the field
    int type;				// WORD_ISA_<type> of the field
    int lowbits;			// Packed info (see word_builder.pl)
    int lastbits;			// Packed info (see word_builder.pl)
    int bytesize;			// Packed info (see word_builder.pl)
    int bytes_offset;			// Packed info (see word_builder.pl)
    int bits;				// Packed info (see word_builder.pl)
    int index;				// Index of the object in the pool_<type> array
    int direction;			// Sorting direction
    int encoding_position;              
    int sort_position;
  } fields[4];
  //
  // Array describing the fields, in sort order.
  //
  struct {
    char* name;				// Symbolic name of the field
    int type;				// WORD_ISA_<type> of the field
    int lowbits;			// Packed info (see word_builder.pl)
    int lastbits;			// Packed info (see word_builder.pl)
    int bytesize;			// Packed info (see word_builder.pl)
    int bytes_offset;			// Packed info (see word_builder.pl)
    int bits;				// Packed info (see word_builder.pl)
    int index;				// Index of the object in the pool_<type> array
    int direction;			// Sorting direction
    int encoding_position;              
    int sort_position;
  } sort[4];
  //
  // Total number of fields
  //
  int nfields;
  //
  // Minimum length of key on disk
  //
  int minimum_length;
};

//
// Object describing the actual structure of a key.
// See struct WordKeyInfo for meaning of fields.
//
static const struct WordKeyInfo word_key_info = {
{
  	{
		"Location",
		WORD_ISA_TypeA,
		0,
		0,
		2,
		0,
		16,
		WORD_KEY_LOCATION,
		WORD_SORT_ASCENDING,
		0,					  
		3
	},
	{
		"Flags",
		WORD_ISA_TypeA,
		0,
		0,
		1,
		2,
		8,
		WORD_KEY_FLAGS,
		WORD_SORT_ASCENDING,
		1,					  
		2
	},
	{
		"DocID",
		WORD_ISA_TypeA,
		0,
		0,
		4,
		3,
		32,
		WORD_KEY_DOCID,
		WORD_SORT_ASCENDING,
		2,					  
		1
	},
	{
		"Word",
		WORD_ISA_String,
		-1,
		-1,
		-1,
		7,
		-1,
		WORD_KEY_WORD,
		WORD_SORT_ASCENDING,
		3,					  
		0
	},

},
{
  	{
		"Word",
		WORD_ISA_String,
		-1,
		-1,
		-1,
		7,
		-1,
		WORD_KEY_WORD,
		WORD_SORT_ASCENDING,
		3,					  
		0
	},
	{
		"DocID",
		WORD_ISA_TypeA,
		0,
		0,
		4,
		3,
		32,
		WORD_KEY_DOCID,
		WORD_SORT_ASCENDING,
		2,					  
		1
	},
	{
		"Flags",
		WORD_ISA_TypeA,
		0,
		0,
		1,
		2,
		8,
		WORD_KEY_FLAGS,
		WORD_SORT_ASCENDING,
		1,					  
		2
	},
	{
		"Location",
		WORD_ISA_TypeA,
		0,
		0,
		2,
		0,
		16,
		WORD_KEY_LOCATION,
		WORD_SORT_ASCENDING,
		0,					  
		3
	},

},
4,
7
};
#endif /* SWIG */

//
// A word occurence description.
//
class WordKey
{
public:
  WordKey() { set = 0; 	pool_TypeA[WORD_KEY_LOCATION] = 0;
	pool_TypeA[WORD_KEY_FLAGS] = 0;
	pool_TypeA[WORD_KEY_DOCID] = 0;
	pool_String[WORD_KEY_WORD] = 0;
; }
#ifndef SWIG
  WordKey(const String& word) { set = 0; 	pool_TypeA[WORD_KEY_LOCATION] = 0;
	pool_TypeA[WORD_KEY_FLAGS] = 0;
	pool_TypeA[WORD_KEY_DOCID] = 0;
	pool_String[WORD_KEY_WORD] = 0;
; Set(word); }
#endif /* SWIG */
  void	Clear() { 
    set = 0;
    	pool_TypeA[WORD_KEY_LOCATION] = 0;
	pool_TypeA[WORD_KEY_FLAGS] = 0;
	pool_TypeA[WORD_KEY_DOCID] = 0;
	pool_String[WORD_KEY_WORD] = 0;
;
  }

  //
  // Accessors
  //


#ifndef SWIG
    inline const String&  GetWord() const { return pool_String[WORD_KEY_WORD]; }
#endif /* SWIG */
    inline String&	  GetWord() { return pool_String[WORD_KEY_WORD]; }
    inline void	          SetWord(const String& arg) { pool_String[WORD_KEY_WORD] = arg; set |= WORD_KEY_WORDFULLY_DEFINED; } 
    inline void	          UndefinedWord() { pool_String[WORD_KEY_WORD].trunc(); set &=  ~WORD_KEY_WORDFULLY_DEFINED; } 
    inline void		  UndefinedWordSuffix() {set &= ~WORD_KEY_WORDSUFFIX_DEFINED;}
    inline void		  SetDefinedWordSuffix() {set |= WORD_KEY_WORDSUFFIX_DEFINED;}
    inline int            IsDefinedWordSuffix() const {return( (set & WORD_KEY_WORDSUFFIX_DEFINED) == WORD_KEY_WORDSUFFIX_DEFINED);}
#ifndef SWIG
	inline TypeA   GetLocation() const { return pool_TypeA[WORD_KEY_LOCATION]; }
	inline TypeA&  GetLocation() { return pool_TypeA[WORD_KEY_LOCATION]; }
	inline void	SetLocation(TypeA arg) { pool_TypeA[WORD_KEY_LOCATION] = arg; set |= WORD_KEY_LOCATION_DEFINED; } 
#else /* SWIG */
	inline unsigned int  GetLocation() { return pool_TypeA[WORD_KEY_LOCATION]; }
	inline void	SetLocation(unsigned int arg) { pool_TypeA[WORD_KEY_LOCATION] = arg; set |= WORD_KEY_LOCATION_DEFINED; } 
#endif /* SWIG */
	inline void	UndefinedLocation() { pool_TypeA[WORD_KEY_LOCATION] = 0; set &= ~WORD_KEY_LOCATION_DEFINED; } 
#ifndef SWIG
	inline TypeA   GetFlags() const { return pool_TypeA[WORD_KEY_FLAGS]; }
	inline TypeA&  GetFlags() { return pool_TypeA[WORD_KEY_FLAGS]; }
	inline void	SetFlags(TypeA arg) { pool_TypeA[WORD_KEY_FLAGS] = arg; set |= WORD_KEY_FLAGS_DEFINED; } 
#else /* SWIG */
	inline unsigned int  GetFlags() { return pool_TypeA[WORD_KEY_FLAGS]; }
	inline void	SetFlags(unsigned int arg) { pool_TypeA[WORD_KEY_FLAGS] = arg; set |= WORD_KEY_FLAGS_DEFINED; } 
#endif /* SWIG */
	inline void	UndefinedFlags() { pool_TypeA[WORD_KEY_FLAGS] = 0; set &= ~WORD_KEY_FLAGS_DEFINED; } 
#ifndef SWIG
	inline TypeA   GetDocID() const { return pool_TypeA[WORD_KEY_DOCID]; }
	inline TypeA&  GetDocID() { return pool_TypeA[WORD_KEY_DOCID]; }
	inline void	SetDocID(TypeA arg) { pool_TypeA[WORD_KEY_DOCID] = arg; set |= WORD_KEY_DOCID_DEFINED; } 
#else /* SWIG */
	inline unsigned int  GetDocID() { return pool_TypeA[WORD_KEY_DOCID]; }
	inline void	SetDocID(unsigned int arg) { pool_TypeA[WORD_KEY_DOCID] = arg; set |= WORD_KEY_DOCID_DEFINED; } 
#endif /* SWIG */
	inline void	UndefinedDocID() { pool_TypeA[WORD_KEY_DOCID] = 0; set &= ~WORD_KEY_DOCID_DEFINED; } 

#ifndef SWIG
#define WORD_HAVE_TypeA 1
#endif /* SWIG */
#ifndef SWIG
	inline TypeA	GetTypeA(int position) const
		{ return pool_TypeA[word_key_info.fields[position].index]; }
	inline TypeA&	GetTypeA(int position)
		{ return pool_TypeA[word_key_info.fields[position].index]; }
	inline void	SetTypeA(TypeA arg, int position)
		{ pool_TypeA[word_key_info.fields[position].index] = arg; SetDefined(position); }
#else /* SWIG */
	inline unsigned int	GetTypeA(int position) const
		{ return pool_TypeA[word_key_info.fields[position].index]; }
	inline void	SetTypeA(TypeA arg, int position)
		{ pool_TypeA[word_key_info.fields[position].index] = arg; SetDefined(position); }
#endif /* SWIG */
#ifndef SWIG
#define WORD_HAVE_String 1
#endif /* SWIG */


  //
  // Field value existenz information
  //
  int	IsDefined(int position) const { return set & (1 << position); }
  void	SetDefined(int position) { set |= (1 << position); }
  void	Undefined(int position) { set &= ~(1 << position); }

  int	IsDefinedInSortOrder(int position) const { return( IsDefined(word_key_info.sort[position].encoding_position) ); }
  void	SetDefinedInSortOrder(int   position)    {        SetDefined(word_key_info.sort[position].encoding_position); }
  void	UndefinedInSortOrder(int position)       {         Undefined(word_key_info.sort[position].encoding_position); }


    int Set(const char *s);

// get/set numerical fields

    inline unsigned int GetInSortOrder(int position) const
    {
	switch(word_key_info.sort[position].type) 
	{
#define STATEMENT(type)  case WORD_ISA_##type:return pool_##type[word_key_info.sort[position].index]; break
#include"WordCaseIsAStatements.h"
	}
	return(0);
    }
    inline void         SetInSortOrder(int position,unsigned int val)
    {
	SetDefinedInSortOrder(position);
	switch(word_key_info.sort[position].type) 
	{
#define STATEMENT(type)  case WORD_ISA_##type:pool_##type[word_key_info.sort[position].index]=val;break
#include"WordCaseIsAStatements.h"
	}
    }
    
  //
  // Storage format conversion
  //
  int 		Unpack(const String& data);
  int 		Pack(String& data) const;

  //
  // Transformations
  //
  int		Merge(const WordKey& other);
  int		PrefixOnly();

  //
  // Predicates
  //
  int		Filled() const { return set == (unsigned int) (((1 << word_key_info.nfields) - 1) | WORD_KEY_WORDSUFFIX_DEFINED); }
  int		Empty() const { return set == 0; }
  int 		Equal(const WordKey& other) const;
  int 		ExactEqual(const WordKey& other) const {return(Equal(other) && other.set == set);}
  int 		PackEqual(const WordKey& other) const;
  int		Prefix() const;
  int           FirstSkipField() const;
  static int 	Compare(const String& a, const String& b);
#ifndef SWIG
  static int 	Compare(const char *a, int a_length, const char *b, int b_length);
#endif /* SWIG */

  int SetToFollowingInSortOrder(int position=-1);

#ifndef SWIG
    friend ostream &operator << (ostream &o, const WordKey &key);
    friend istream &operator >> (istream &is,  WordKey &key);
#endif /* SWIG */
    void Print() const;

private:

  //
  // Convert a single number from and to disk storage representation
  //
  static int UnpackNumber(const char* from, int from_size, unsigned int* top, int lowbits, int bits);
  static int PackNumber(unsigned int from, char* to, int to_size, int lowbits, int lastbits);

  //
  // Data members
  //
  unsigned int set;
	TypeA	pool_TypeA[3];
	String	pool_String[1];

};

#ifndef SWIG
//
// Set bit number <b> to 0 and others to 1. <b> may have a value from 0 to 8. If
// 8 then all bits are 1.
//
#define WORD_BIT_MASK(b) ((b) == 0 ? 0xff : ((( 1 << (b)) - 1) & 0xff))

//
// Decode integer found in <from> using <from_size> bytes. The integer starts at <lowbits> bit
// in the first byte and occupies a total of <bits> bits. The resulting integer is stored in *<top>
//
inline int WordKey::UnpackNumber(const char* from, int from_size, unsigned int* top, int lowbits, int bits)
{
  unsigned int to = 0;

  to = ((from[0] & 0xff) >> lowbits);

  if(lowbits) to &= WORD_BIT_MASK(8 - lowbits);

  if(from_size == 1) 
    to &= WORD_BIT_MASK(bits);
  else {
    for(int i = 1; i < from_size; i++) {
      to |= (from[i] & 0xff) << ((i - 1) * 8 + (8 - lowbits));
    }
  }

  if(bits < (int)(sizeof(unsigned int) * 8))
    to &= ( 1 << bits ) - 1;
  
  *top = to;

  return OK;
}

//
// Encode integer <from>, starting at bit <lowbits> in byte array <to>. It will span
// <to_size> bytes and only the <lastbits> bits of the last byte (to[to_size - 1]) are 
// filled. See word_builder.pl for more information.
//
inline int WordKey::PackNumber(unsigned int from, char* to, int to_size, int lowbits, int lastbits)
{
  if(lowbits) {
    to[0] |= ((from & WORD_BIT_MASK(8 - lowbits)) << lowbits) & 0xff;
  } else {
    to[0] = from & 0xff;
  }
  from >>= 8 - lowbits;

  for(int i = 1; i < to_size; i++) {
    to[i] = from & 0xff;
    from >>= 8;
  }

  if(lastbits) to[to_size - 1] &= WORD_BIT_MASK(lastbits);

  return OK;
}

#undef WORD_BIT_MASK
#endif /* SWIG */


#endif
