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
#include "StringList.h"
#endif /* SWIG */

#define	WORD_KEY_LOCATION	0
#define	WORD_KEY_FLAGS	1
#define	WORD_KEY_DOCID	2
#define	WORD_KEY_WORD	0



// WORDSUFFIX:
// field in set flag that says if a word is just a prefix (incomplete word)
// WORD_KEY_WORDSUFFIX_DEFINED -> means that word IS complete (not a prefix)
#define  WORD_KEY_WORDSUFFIX_DEFINED (1 << 30)
#define WORD_KEY_WORD_DEFINED 1
#define WORD_KEY_WORDFULLY_DEFINED ( WORD_KEY_WORDSUFFIX_DEFINED | WORD_KEY_WORD_DEFINED )


#define WORD_KEY_MAX_NFIELDS 20
#define WORDKEYFIELD_BITS_MAX 1000

#ifndef SWIG
// C comparison function interface for Berkeley DB (bt_compare)
//
int word_db_cmp(const DBT *a, const DBT *b);
#endif /* SWIG */


//
// Type number associated to each possible type for a key element
// (type field of struct WordKeyInfo).
//
#define WORD_ISA_NUMBER		1
#define WORD_ISA_String		2


#ifndef SWIG
//
// Describes the structure of the key, ie meta information
// for the key. This includes the layout of the packed version
// stored on disk.
//

typedef unsigned int WordKeyNum;

class WordKeyField
{
 public:
    String name;				// Symbolic name of the field
    int type;				// WORD_ISA_<type> of the field
    int lowbits;			// Packed info (see word_builder.pl)
    int lastbits;			// Packed info (see word_builder.pl)
    int bytesize;			// Packed info (see word_builder.pl)
    int bytes_offset;			// Packed info (see word_builder.pl)
    int bits;				// Packed info (see word_builder.pl)
//      int index;				// Index of the object in the pool_<type> array
    int direction;			// Sorting direction
    int encoding_position;              
    int sort_position;
    int bits_offset;


//      inline WordKeyNum Unpack(const String &data)
//      { 
//  	WordKeyNum to;
//  	WordKey::UnpackNumber(&data[bytes_offset], bytesize, &to, lowbits, bits);
//  	return (to);
//      }
	
    void nprint(char c,int n);
    void show();
    WordKeyField(WordKeyField *previous,char *nname,int nbits,int encoding_position, int sort_position );
    WordKeyField(){;}
};


class WordKeyInfo;

extern WordKeyInfo *word_key_info;

class WordKeyInfo 
{
    friend WordKeyField;
protected:
    //
    // Array describing the fields, in encoding order. 
    //
#define WORD_SORT_ASCENDING	1
#define WORD_SORT_DESCENDING	2

public:
    //
    // Array describing the fields, in sort order.
    //
    WordKeyField *sort;
    //
    // Array describing the fields, in encoding order.
    //
    WordKeyField *encode;
    //
    // Total number of fields
    //
    int nfields;
    //
    // Minimum length of key on disk
    //
    int minimum_length;

//      inline WordKeyNum UnpackFieldInSortOrder( const String &data, int position ){return(sort[position].Unpack(data));}

    WordKeyField *previous;
    int encoding_position;

    void        Initialize( int nnfields);
    void        Initialize( String &line);
    void        AddFieldInEncodingOrder(String &name,int bits, int sort_position);
    void        AddFieldInEncodingOrder(const String &line);
    void        SetDescriptionFromFile  (const String &filename);
    static void SetKeyDescriptionFromFile  (const String &filename);
    void        SetDescriptionFromString(const String &desc);
    static void SetKeyDescriptionFromString(const String &desc);

    void  show();


    ~WordKeyInfo()
    {
	if(sort){delete [] sort;}
	if(sort){delete [] encode;}
    }
    WordKeyInfo()
    {
	sort   = NULL;
	nfields = -1;
    }
};


#endif /* SWIG */


//
// A word occurence description.
//
// !!!!!!!DEBUGTMP
#define FATAL_ABORT fflush(stdout);fprintf(stderr,"FATAL ERROR at file:%s line:%d !!!\n",__FILE__,__LINE__);fflush(stderr);(*(int *)NULL)=1
#define errr(s) {fprintf(stderr,"FATAL ERROR:%s\n",s);FATAL_ABORT;}
class WordKey
{
protected:

  void initialize()
  {
      if(!word_key_info)
      {
	  cerr << "WordKey::WordKey used before word_key_info set" << endl;
	  errr("WordKey::initialize");
      }
      
      numerical_fields = new WordKeyNum[nfields()-1]; 
      Clear();
  }

  void copy_from(const WordKey &other)
  {
      if(other.IsDefinedInSortOrder(0)){SetWord(other.GetWord());}
      for(int i=1;i<nfields();i++)
      {
	  if(other.IsDefinedInSortOrder(i))
	  {SetInSortOrder(i,other.GetInSortOrder(i));}
      }
      setbits=other.setbits;
  }
public:
  void	Clear() 
  { 
      setbits = 0;
      for(int i=0;i<nfields()-1;i++)
      {
	  numerical_fields[i] = 0;
      }
  }

 ~WordKey()
  {
      delete [] numerical_fields;
  }
  WordKey() 
  {
      initialize();
  }
  WordKey(const String& word) 
  {
      initialize();
      Set(word); 
  }
  WordKey(const WordKey &other) 
  {
      initialize();
      copy_from(other);
  }


  //
  // Accessors
  //
  void operator =(const WordKey &other)
  {
      initialize();
      copy_from(other);
  }

    static inline const int        nfields() {return word_key_info->nfields;}

#ifndef SWIG
    inline const String&  GetWord() const { return kword; }
#endif /* SWIG */
    inline String&	  GetWord() { return kword; }
    inline void	          SetWord(const String& arg) { kword = arg; setbits |= WORD_KEY_WORDFULLY_DEFINED;} 
    inline void	          UndefinedWord() { kword.trunc(); setbits &=  ~WORD_KEY_WORDFULLY_DEFINED; } 
    inline void		  UndefinedWordSuffix() {setbits &= ~WORD_KEY_WORDSUFFIX_DEFINED;}
    inline void		  SetDefinedWordSuffix() {setbits |= WORD_KEY_WORDSUFFIX_DEFINED;}
    inline int            IsDefinedWordSuffix() const {return( (setbits & WORD_KEY_WORDSUFFIX_DEFINED) == WORD_KEY_WORDSUFFIX_DEFINED);}

  //
  // Field value existenz information
  //
  int	IsDefinedInSortOrder(int position) const { return setbits & (1 << position); }
  void	SetDefinedInSortOrder(int position) { setbits |= (1 << position); }
  void	UndefinedInSortOrder(int position) { setbits &= ~(1 << position); }

  int	IsDefined(int position) const { return( IsDefined(word_key_info->encode[position].sort_position));}
  void	SetDefined(int   position)    {        SetDefined(word_key_info->encode[position].sort_position); }
  void	Undefined(int position)       {         Undefined(word_key_info->encode[position].sort_position); }

  int	IsFullyDefined() const 
  { 
      if(!IsDefinedWordSuffix()){return 0;}
      for(int i=0;i<nfields();i++){if(!IsDefinedInSortOrder(i)){return 0;}}
      return 1;
  }

  //
  // Set the whole structure from ascii string description
  //
  int Set(const String& buffer);
#ifndef SWIG
  int Set(StringList& fields);
#endif /* SWIG */
  //
  // Convert the whole structure to an ascii string description
  //
  int Get(String& buffer) const;

  // Get/Set numerical fields

  inline WordKeyNum GetInSortOrder(int position) const 
  {
      if(position<1 || position>=nfields()){errr("GetInSortOrder: out of bounds");}
      return(numerical_fields[position-1]);
  }

  inline void SetInSortOrder(int position,WordKeyNum val)
  {
      if(position<1 || position>=nfields()){errr("SetInSortOrder: out of bounds");}
      SetDefinedInSortOrder(position);
      numerical_fields[position-1] = val;
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
  int		Filled() const { return setbits == (unsigned int) (((1 << nfields()) - 1) | WORD_KEY_WORDSUFFIX_DEFINED); }
  int		Empty() const { return setbits == 0; }
  int 		Equal(const WordKey& other) const;
  int 		ExactEqual(const WordKey& other) const {return(Equal(other) && other.setbits == setbits);}
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
#endif /* SWIG */
    void Print() const;

private:

  //
  // Convert a single number from and to disk storage representation
  //
  static int UnpackNumber(const char* from, int from_size, WordKeyNum* top, int lowbits, int bits);
  static int PackNumber(WordKeyNum from, char* to, int to_size, int lowbits, int lastbits);

  //
  // Data members
  //
  unsigned int setbits;
  WordKeyNum *numerical_fields;
  String	kword;


 public:
// ***** DEBUGINIG ****
    static void show_packed(const String& key,int type=0);


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
inline int WordKey::UnpackNumber(const char* from, int from_size, WordKeyNum* top, int lowbits, int bits)
{
  WordKeyNum to = 0;

  to = ((from[0] & 0xff) >> lowbits);

  if(lowbits) to &= WORD_BIT_MASK(8 - lowbits);

  if(from_size == 1) 
    to &= WORD_BIT_MASK(bits);
  else {
    for(int i = 1; i < from_size; i++) {
      to |= (from[i] & 0xff) << ((i - 1) * 8 + (8 - lowbits));
    }
  }

  if(bits < (int)(sizeof(WordKeyNum) * 8))
    to &= ( 1 << bits ) - 1;
  
  *top = to;

  return OK;
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
