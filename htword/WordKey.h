// WordKey.h
//
// WordKey: Describes the key used to store a word in the word database.
//          The structure of a key is described by the WordKeyInfo class (WordKeyInfo).
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
//          Suffix definition: a word suffix is a kind of marker that says if the word
//          is a full word or only the beginning of a word. If a word has a suffix then
//          it's a full word. If it has no suffix then it's only the beginning of a word.
//          This is mostly usefull when specifying search keys. If a search key word has no
//          suffix, the search mechanism is expected to return all words that begin with
//          the word. If the search key word has a suffix, only words that exactly match
//          the search key word will be returned.
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
#include "StringList.h"
#endif /* SWIG */

//
// WORDSUFFIX:
// 
// field in set flag that says if a word is just a prefix (incomplete word)
// WORD_KEY_WORDSUFFIX_DEFINED -> means that word IS complete (not a prefix)
//
#define WORD_KEY_WORDSUFFIX_DEFINED	 (1 << 30)
#define WORD_KEY_WORD_DEFINED		 1
#define WORD_KEY_WORDFULLY_DEFINED	 ( WORD_KEY_WORDSUFFIX_DEFINED | WORD_KEY_WORD_DEFINED )

#define WORD_KEY_MAX_NFIELDS 20
#define WORDKEYFIELD_BITS_MAX 1000

#ifndef SWIG
// C comparison function interface for Berkeley DB (bt_compare)
//
int word_db_cmp(const DBT *a, const DBT *b);
#endif /* SWIG */

#ifndef SWIG
#include"WordKeyInfo.h"
#endif /* SWIG */

//
// Describe a word occurence
//
// !!!!!!!DEBUGTMP
#ifndef SWIG
#define WORD_FATAL_ABORT fflush(stdout);fprintf(stderr,"FATAL ERROR at file:%s line:%d !!!\n",__FILE__,__LINE__);fflush(stderr);(*(int *)NULL)=1
#define word_errr(s) {fprintf(stderr,"FATAL ERROR:%s\n",s);WORD_FATAL_ABORT;}
#endif /* SWIG */
class WordKey
{
 public:
  //
  // Constructors, destructors, copy and clear 
  //
  //
  // Empty key
  //
  WordKey() 
  {
      Initialize();
  }
#ifndef SWIG
  //
  // Initialize from an ascii description of a key
  //
  WordKey(const String& word) 
  {
      Initialize();
      Set(word); 
  }
  //
  // Copy constructor (needed because of the array pointer)
  //
  WordKey(const WordKey &other) 
  {
      Initialize();
      CopyFrom(other);
  }
#endif /* SWIG */
  ~WordKey()
  {
      delete [] numerical_fields;
  }
#ifndef SWIG
 protected:
  //
  // Constructor helper, allocate members and set to empty key
  //
  void Initialize()
    {
      if(!Info())
	{
	  cerr << "WordKey::WordKey used before word_key_info set" << endl;
	  word_errr("WordKey::initialize");
	}
      
      numerical_fields = new WordKeyNum[NFields()-1]; 
      Clear();
    }
 public:
  //
  // Copy operator (needed because of the array pointer)
  //
  void operator =(const WordKey &other)
  {
      Clear();
      CopyFrom(other);
  }
#endif /* SWIG */
  //
  // Copy other into object
  //
  void CopyFrom(const WordKey &other)
    {
      if(other.IsDefined(0)) { SetWord(other.GetWord()); }
      for(int i=1;i<NFields();i++)
	{
	  if(other.IsDefined(i))
	    {
	      Set(i, other.Get(i));
	    }
	}
      setbits=other.setbits;
    }
  //
  // Reset to empty key. 
  //
  void	Clear() 
  { 
      setbits = 0;
      kword.trunc();
      for(int i=0;i<NFields()-1;i++)
      {
	  numerical_fields[i] = 0;
      }
  }

#ifndef SWIG
  //
  // Convinience functions to access key structure information (see WordKeyInfo.h)
  //
  static inline const WordKeyInfo *Info()   { return WordKeyInfo::Get(); }
#endif /* SWIG */
  static inline const int         NFields() { return WordKeyInfo::Get()->nfields; }

  //
  // Accessors
  //
  //
  // The word and it suffix (see suffix definition above)
  //
#ifndef SWIG
  inline const String&  GetWord() const { return kword; }
#endif /* SWIG */

  inline String&	GetWord()       { return kword; }
  inline void	        SetWord(const String& arg) { kword = arg; setbits |= WORD_KEY_WORDFULLY_DEFINED; } 
 protected:
  inline void	        SetWord(const char* arg, int arg_length) { kword.set(arg, arg_length); setbits |= WORD_KEY_WORDFULLY_DEFINED; } 
 public:
  inline void	        UndefinedWord() { kword.trunc(); setbits &=  ~WORD_KEY_WORDFULLY_DEFINED; } 
  inline void		UndefinedWordSuffix() {setbits &= ~WORD_KEY_WORDSUFFIX_DEFINED;}
  inline void		SetDefinedWordSuffix() {setbits |= WORD_KEY_WORDSUFFIX_DEFINED;}
  //
  // Returns true if word suffix is defined, false otherwise.
  //
  inline int            IsDefinedWordSuffix() const {return( (setbits & WORD_KEY_WORDSUFFIX_DEFINED) == WORD_KEY_WORDSUFFIX_DEFINED);}
  //
  // Get/Set numerical fields
  //
  inline WordKeyNum Get(int position) const 
  {
    // if(position<1 || position>=NFields()){errr("Get: out of bounds");}
      return(numerical_fields[position-1]);
  }
#ifndef SWIG
  inline       WordKeyNum &      operator[] (int n)        { return(numerical_fields[n-1]); }
  inline const WordKeyNum &      operator[] (int n) const  { return(numerical_fields[n-1]); }
#endif /* SWIG */
  inline void Set(int position, WordKeyNum val)
  {
    // if(position<1 || position>=NFields()){errr("Set: out of bounds");}
      SetDefined(position);
      numerical_fields[position-1] = val;
  }
    
  //
  // Key field value existenz. Defined means the value of the field contains
  // a valid value. Undefined means the value of the field is not valid.
  //
  //
  // Returns true if field at position is defined, false otherwise.
  //
  int	IsDefined(int position) const { return setbits & (1 << position); }
  //
  // Value in field position becomes defined
  //
  void	SetDefined(int position)      { setbits |= (1 << position); }
  //
  // Value in field position becomes undefined
  //
  void	Undefined(int position)       { setbits &= ~(1 << position); }

#ifndef SWIG
  //
  // Set and Get the whole structure from/to ascii description
  // The ascii description is one line, newline terminated with
  // fields separated by tabs or white space.
  // Example: Foo <DEF> 0 1 4 2
  // Field 1: The word as a string or <UNDEF> if not defined
  // Field 2: <DEF> if suffix defined, <UNDEF> if suffix undefined
  // Field 3 to nfield + 1: numerical value of the field or <UNDEF> if
  //                        not defined
  //
  // Set the whole structure from ascii string description
  // Return OK if successfull, NOTOK otherwise.
  //
  int Set(const String& bufferin);
  int Set(StringList& fields);
  //
  // Convert the whole structure to an ascii string description
  // Return OK if successfull, NOTOK otherwise.
  //
  int Get(String& bufferout) const;
#endif /* SWIG */

  //
  // Storage format conversion
  //
#ifndef SWIG
  //
  //  Return OK if successfull, NOTOK otherwise.
  //
  int 		Unpack(const char* string, int length);
  //
  //  Return OK if successfull, NOTOK otherwise.
  //
  inline int    Unpack(const String& data) { return(Unpack(data,data.length())); }
  //
  //  Return OK if successfull, NOTOK otherwise.
  //
  int 		Pack(String& data) const;
#endif /* SWIG */

  //
  // Transformations
  //
  //
  // Copy each defined field from other into the object, if the corresponding
  // field of the object is not defined. 
  //  Return OK if successfull, NOTOK otherwise.
  //
  int		Merge(const WordKey& other);
  //
  // Undefine all fields found after the first undefined field. The
  // resulting key has a set of defined fields followed by undefined fields.
  // Returns NOTOK if the word is not defined because the resulting key would 
  // be empty and this is considered an error. Returns OK on success.
  //
  int		PrefixOnly();
#ifndef SWIG
  //
  //  Set this key to a key immediately larger than "other" 
  //  if j!=-1, all fields >= j (in sort order) are
  //  set to 0. This is used when searching, see WordList.
  //  Return OK if successfull, NOTOK otherwise.
  //
  int           SetToFollowing(int position=-1);
#endif /* SWIG */

  //
  // Predicates
  //
  //
  // Return true if all the fields are defined, false otherwise
  //
  int		Filled() const { return setbits == (unsigned int) (((1 << NFields()) - 1) | WORD_KEY_WORDSUFFIX_DEFINED); }
  //
  // Return true if no fields are defined, false otherwise
  //
  int		Empty() const  { return setbits == 0; }
  //
  // Return true if the object and other are equal. Only fields defined in both keys
  // are compared.
  //
  int 		Equal(const WordKey& other) const;
  //
  // Return true if the object and other are equal. All fields are compared. If
  // a field is defined in object and not defined in equal, the key are not considered
  // equal.
  //
  int 		ExactEqual(const WordKey& other) const {return(Equal(other) && other.setbits == setbits);}
#ifndef SWIG
  //
  // Return true if the object and other are equal. The packed string are compared. 
  // An undefined numerical field will be 0 and therefore undistinguishable from a
  // defined field whose value is 0.
  //
  int 		PackEqual(const WordKey& other) const;
#endif /* SWIG */
  //
  // Return OK if the key may be used as a prefix for search.
  // In other words return OK if the fields set in the key
  // are all contiguous, starting from the first field in sort order.
  // Otherwise returns NOTOK
  //
  int		Prefix() const;
#ifndef SWIG
  //
  // Find and return the position of the first field that must be checked for skip.
  // If no field is to be skipped, NFields() is returned.
  // Skipping is a notion used when searching for a key and explained
  // in WordList. 
  //
  int           FirstSkipField() const;
#endif /* SWIG */

#ifndef SWIG
  //
  // Compare <a> and <b> in the Berkeley DB fashion. 
  // <a> and <b> are packed keys. The semantics of the
  // returned int is as of strcmp.
  //
  static int 	    Compare(const String& a, const String& b);
  static inline int Compare(const char *a, int a_length, const char *b, int b_length);

  //
  // Print, debug, benchmark
  //
  friend ostream &operator << (ostream &o, const WordKey &key);
#endif /* SWIG */
  void Print() const;

#ifndef SWIG
  //
  // Ascii display of packed key
  //
  static void ShowPacked(const String& key, int type=0);
  //
  // Initialize key with random values
  //
  void SetRandom();

private:

  //
  // Convert a single number from and to disk storage representation
  //
  static int UnpackNumber(const unsigned char* from, const int from_size, WordKeyNum &res, const int lowbits, const int bits);
  static int PackNumber(WordKeyNum from, char* to, int to_size, int lowbits, int lastbits);

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
  WordKeyNum   *numerical_fields;
  //
  // Holds the word key field
  //
  String       kword;
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
inline int WordKey::UnpackNumber(const unsigned char* from, const int from_size, WordKeyNum &res, const int lowbits, const int bits)
{
    // SPEED CRITICAL SECTION

    if((lowbits+bits)<=8)
    {
	// simplest case (everything fits on first byte)
	res = ((*from)>>lowbits) & WORD_BIT_MASK2(bits);
	return OK;
    }
    else
    if(!lowbits && !(bits & 0x07))
    {
	// simple case everything is byte alligned
	char *ctop=(char *)&res;
	res=0;
	for(int i=from_size;i;i--)
	{
	    *(ctop++)=*(from++);
	}
	return OK;
    }
    else
    {
	// general case

	// first byte
	res = ((*(from++))>>lowbits) & 0xff;

	const int ncentral=((lowbits + bits)>>3)-1;
	const int nbitsinfirstbyte=8-lowbits;
	const int nbitsremaining=bits-(  (ncentral<<3)+nbitsinfirstbyte );

	// central bytes
	if(ncentral)
	{
	    WordKeyNum v=0;
	    unsigned char *cv=(unsigned char *)&v;
	    for(int i=ncentral;i;i--)
	    {
		*(cv++)=*(from++);
	    }
	    res|=v<<nbitsinfirstbyte;
	}
    
	// last byte
	if(nbitsremaining)
	{
	    res|=((WordKeyNum)((*from) &  WORD_BIT_MASK2(nbitsremaining) )) << 
		(nbitsinfirstbyte +(ncentral<<3));
	}
  

	return OK;
    }
}

//
// Encode integer <from>, starting at bit <lowbits> in byte array <to>. It will span
// <to_size> bytes and only the <lastbits> bits of the last byte (to[to_size - 1]) are 
// filled. See word_builder.pl for more information.
//
inline int WordKey::PackNumber(WordKeyNum from, char* to, int to_size, int lowbits, int lastbits)
{
  // first byte
  if(lowbits) {
    to[0] |= ((from & WORD_BIT_MASK(8 - lowbits)) << lowbits) & 0xff;
  } else {
    to[0] = from & 0xff;
  }
  from >>= 8 - lowbits;

  // following bytes
  for(int i = 1; i < to_size; i++) {
    to[i] = from & 0xff;
    from >>= 8;
  }

  // clip the end off (clobbers anything left at the end of this byte)
  if(lastbits) to[to_size - 1] &= WORD_BIT_MASK(lastbits);

  return OK;
}

#undef WORD_BIT_MASK
#endif /* SWIG */

#endif
