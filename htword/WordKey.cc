//
// WordKey.cc
//
// WordKey: All the functions are implemented regardless of the actual
//          structure of the key using word_key_info.
//          WARNING: although it may seem that you can have two String 
//          fields in the key, some code does not support that. This should
//          not be a problem since the goal of the WordKey class is to
//          implement the keys of an inverted index.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordKey.cc,v 1.3.2.14 2000/01/10 16:19:13 loic Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <iostream.h>
#include <ctype.h>
#include <fstream.h>

#include "HtRandom.h"

#include "WordKey.h"

//
// Returns OK if fields set in 'object' and 'other' are all equal.
//
// Fields not set in either 'object' or 'other' are ignored 
// completely. If the prefix_length is > 0 the 'object' String
// fields are compared to the prefix_length bytes of the 'other'
// String fields only.
//
// This function is usefull to compare existing keys with a search
// criterion that may be incomplete. For instance if we look for keys
// that contain words starting with a given prefix or keys that
// are located in a specific document, regardless of their location
// in the document.
//
int WordKey::Equal(const WordKey& other) const
{
  const WordKeyInfo& info0 = *WordKey::Info();
  //
  // Walk the fields in sorting order. As soon as one of them
  // does not compare equal, return.
  //
  for(int j = 0; j < info0.nfields; j++) 
  {
    //
    // Only compare fields that are set in both key
    //
    if(!IsDefined(j) || !other.IsDefined(j)) continue;

    switch(info0.sort[j].type) {
    case WORD_ISA_STRING:
      if(!IsDefinedWordSuffix()) {
//  	  cout << "COMPARING UNCOMPLETE WORDS IN WORDKEY" << endl;
	if(kword != other.kword.sub(0, kword.length()))
	  return 0;
      } else {
	if(kword != other.kword)
	  return 0;
      }
      break;
    default:
      if(Get(j) != other.Get(j)) return 0;
      break;
    }
  }
  return 1;
}

//
// Compare <a> and <b> in the Berkeley DB fashion. 
// <a> and <b> are packed keys.
//
inline int 
WordKey::Compare(const char *a, int a_length, const char *b, int b_length)
{
  const WordKeyInfo& info0 = *WordKey::Info();

  if(a_length < info0.minimum_length || b_length < info0.minimum_length) {
      cerr << "WordKey::Compare: key length for a or b < info.minimum_length\n";
      return NOTOK;
  }

  //
  // Walk the fields in sorting order. As soon as one of them
  // does not compare equal, return.
  //


  //  first field: string
  {
      const int byte_offset=info0.sort[0].bytes_offset;
      const int p1_length = a_length - byte_offset;
      const int p2_length = b_length - byte_offset;
      int len = p1_length > p2_length ? p2_length : p1_length;
      const unsigned char* p1 = (unsigned char *)a + byte_offset;
      const unsigned char* p2 = (unsigned char *)b + byte_offset;

      for (;len--; ++p1, ++p2)
      {
	  if (*p1 != *p2)
	  {
	      return (info0.sort[0].direction == WORD_SORT_DESCENDING ?
		      ((int)*p2 - (int)*p1) :
		      ((int)*p1 - (int)*p2)    );
	  }
      }
      if(p1_length != p2_length)
      {
	  return (info0.sort[0].direction == WORD_SORT_DESCENDING ?
		  p2_length - p1_length :
		  p1_length - p2_length );
      }
  }
  //  following fields: numerical
  for(int j = 1; j < info0.nfields; j++) 
  {
	WordKeyNum p1;
	WordKeyNum p2;
	WordKey::UnpackNumber((unsigned char *)&a[info0.sort[j].bytes_offset],
			      info0.sort[j].bytesize,
			      p1,
			      info0.sort[j].lowbits,
			      info0.sort[j].bits);
	
	WordKey::UnpackNumber((unsigned char *)&b[info0.sort[j].bytes_offset],
			      info0.sort[j].bytesize,
			      p2,
			      info0.sort[j].lowbits,
			      info0.sort[j].bits);
	if(p1 != p2)
	{
	    return (info0.sort[j].direction == WORD_SORT_DESCENDING ?
		    p2 - p1 :
		    p1 - p2   );
	}
  }

  //
  // If we reach this point, everything compared equal
  //
  return 0;
}

//
// Compare <a> and <b> in the Berkeley DB fashion. 
// <a> and <b> are packed keys.
//
int 
WordKey::Compare(const String& a, const String& b)
{
  return WordKey::Compare(a, a.length(), b, b.length());
}

//
// C comparison function interface for Berkeley DB (bt_compare)
// Just call the static Compare function of WordKey. It is *critical*
// that this function is as fast as possible. See the Berkeley DB
// documentation for more information on the return values.
//
int
word_db_cmp(const DBT *a, const DBT *b)
{
    return WordKey::Compare((char*)a->data, a->size, (char*)b->data, b->size);
}


//
// Compare object and <other> using comparison of their packed form
//
int 
WordKey::PackEqual(const WordKey& other) const
{
  String this_pack;
  Pack(this_pack);

  String other_pack;
  other.Pack(other_pack);

  return this_pack == other_pack;
}

//  Set this key to a key immediately larger than "other" 
//  if j!=-1, all fields >= j (in sort order) are
//  set to 0 
int 
WordKey::SetToFollowing(int position)
{
    const WordKeyInfo& info0 = *WordKey::Info();
    if(position<0){position=NFields();}
    int i;

    if(position==0){cerr << "SetToFollowing with position=0" << endl;return NOTOK;}
    
    if(position>1)
    {// increment if numerical field
	int bits=info0.sort[position].bits;
	int f=Get(position-1);
	// check which direction we're going
	if(info0.sort[position].direction == WORD_SORT_ASCENDING)
	{
	    if( bits < 32){if((f+1) >= 1<<bits){return NOTOK;}}// overflow
            //TODO: add bits>=32 for overflow case....
	    Set(position-1,f+1);
	}
	else
	{
	    if(f>0){Set(position-1,f-1);}
	    else{return (NOTOK);}// underflow
	}
    }
    else
    {// if non numerical (field 0=word) add a (char)1 to string
//	if(WLdebug>1){cout << "oops,  previous field is string!" << endl;}
	GetWord() << (char) 1;
    }

    // zero fields >=position'th
    for(i=position;i<NFields();i++){if(IsDefined(i)){Set(i,0);}}

    return(OK);
}

//
// Return true if the key may be used as a prefix for search.
// In other words return true if the fields set in the key
// are all contiguous, starting from the first field in sort order.
//
int 
WordKey::Prefix() const
{
  const WordKeyInfo& info0 = *WordKey::Info();
  //
  // If all fields are set, it can be considered as a prefix although
  // it really is a fully qualified key.
  //
  if(Filled()) return OK;
  //
  // If the first field is not set this cannot be a prefix
  //
  if(!IsDefined(0)) return NOTOK;
  
  int found_unset = 0;
  if(!IsDefinedWordSuffix()){found_unset=1;}
  //
  // Walk the fields in sorting order. 
  //
  for(int j = 1; j < info0.nfields; j++) 
  {
    //
    // Fields set, then fields unset then field set -> not a prefix
    //
    if(IsDefined(j))
      if(found_unset) return NOTOK;
    else
      //
      // Found unset fields and this is fine as long as we do
      // not find a field set later on.
      //
      found_unset++;
  }

  return OK;
}

//
// Find and return the position of the first field that must be checked for skip.
// If no field is to be skipped, NFields() is returned.
//
int 
WordKey::FirstSkipField() const
{
    int first_skip_field=-2;

    for(int i=0;i<NFields();i++)
    {
	if(first_skip_field==-2 && !IsDefined(i)){first_skip_field=-1;}
	else
	if(first_skip_field==-2 && i==0 && !IsDefinedWordSuffix()){first_skip_field=-1;}
	else
	if(first_skip_field==-1 &&  IsDefined(i)){first_skip_field=i;break;}
    }
    if(first_skip_field<0){first_skip_field=NFields();}
    return(first_skip_field);
}

//
// Unset all fields past the first unset field
// Return the number of fields in the prefix or 0 if
// first field is not set, ie no possible prefix.
//
int 
WordKey::PrefixOnly()
{
  const WordKeyInfo& info0 = *WordKey::Info();
  //
  // If all fields are set, the whole key is the prefix.
  //
  if(Filled()) return OK;
  //
  // If the first field is not set there is no possible prefix
  //
//    cerr << "prefix only: input key:" << *this <<endl;
  if(!IsDefined(0)) 
  {
//        cerr << "prefix only: word not defined " <<endl;
      return NOTOK;
  }
  
  int found_unset = 0;
  //
  // Walk the fields in sorting order. 
  //
  if(!IsDefinedWordSuffix()){found_unset=1;}

  for(int j = 1; j < info0.nfields; j++) 
  {
    //
    // Unset all fields after the first unset field
    //
    if(IsDefined(j)) 
    {
	if(found_unset) {Set(j,0);Undefined(j);}
    } 
    else {found_unset=1;}
  }

  return OK;
}

//
// Unpack from data and fill fields of object
// 
int 
WordKey::Unpack(const char* string,int length)
{
  const WordKeyInfo& info0 = *WordKey::Info();

  if(length < info0.minimum_length) {
    cerr << "WordKey::Unpack: key record length < info.minimum_length\n";
    return NOTOK;
  }

  SetWord(&(string[info0.sort[0].bytes_offset]), length - info0.minimum_length);

  for(int j = 1; j < info0.nfields; j++) 
  {
      WordKeyNum value = 0; 
      WordKey::UnpackNumber((unsigned char *)&string[info0.sort[j].bytes_offset], 
			    info0.sort[j].bytesize, 
			    value, 
			    info0.sort[j].lowbits, 
			    info0.sort[j].bits); 
      Set(j,value);
  }

  return OK;
}

//
// Pack object into the <packed> string
//
int 
WordKey::Pack(String& packed) const
{
  const WordKeyInfo& info0 = *WordKey::Info();

  char* string;
  int length = info0.minimum_length;

  length += kword.length();

  if((string = (char*)malloc(length)) == 0) {
    cerr << "WordKey::Pack: malloc returned 0\n";
    return NOTOK;
  }
  memset(string, '\0', length);

  memcpy(&string[info0.sort[0].bytes_offset], kword.get(), kword.length());
  for(int i = 0; i < info0.nfields-1; i++) 
  {
      WordKey::PackNumber(Get(info0.encode[i].sort_position), 
			  &string[info0.encode[i].bytes_offset], 
			  info0.encode[i].bytesize, 
			  info0.encode[i].lowbits, 
			  info0.encode[i].lastbits); 
  }
  
  packed.set(string, length);

  free(string);

  return OK;
}

//
// Copy all fields set in <other> to object, only if 
// the field is not already set in <other>
//
int WordKey::Merge(const WordKey& other)
{
  const WordKeyInfo& info0 = *WordKey::Info();

  
  for(int j = 0; j < info0.nfields; j++) 
  {
    if(!IsDefined(j) && other.IsDefined(j)) 
    {
      switch(info0.sort[j].type) 
      {
      case WORD_ISA_STRING: 
	  SetWord(other.GetWord());
	  break;
      default:
	  Set(j,other.Get(j)); 
	break;
      }
    }
  }

  return OK;
}

//
// Convert the whole structure to an ascii string description
//
int
WordKey::Get(String& buffer) const
{
  buffer.trunc();
  const WordKeyInfo& info0 = *WordKey::Info();

  //
  // Walk the fields in sorting order. As soon as one of them
  // does not compare equal, return.
  //
  for(int j = 0; j < info0.nfields; j++) 
  {
      if(!IsDefined(j)){buffer << "<UNDEF>";}
      else
      {
	  switch(info0.sort[j].type) 
	  {
	  case WORD_ISA_STRING:  buffer << GetWord();          break;
	  case WORD_ISA_NUMBER:  buffer << Get(j);  break;
  	  default:
  	      cerr << "WordKey::operator <<: invalid type " << info0.sort[j].type << " for field (in sort order) " << j << "\n";
  	      return NOTOK;
	  }
      }
      //
      // Output virtual word suffix field
      //
      if(j==0) {
	if(IsDefined(j) && !IsDefinedWordSuffix()) {
	  buffer << "\t<UNDEF>";
	} else {
	  buffer << "\t<DEF>";
	}
      }
      buffer << "\t";
  }
  return OK;
}

//
// Set a key from an ascii representation
//
int
WordKey::Set(const String& buffer)
{
  StringList fields(buffer, "\t ");
  return Set(fields);
}

//
// Set a key from list of fields
//
int
WordKey::Set(StringList& fields)
{
  const WordKeyInfo& info0 = *WordKey::Info();
  int length = fields.Count();

  if(length < info0.nfields + 1) {
    cerr << "WordKey::Set: expected at least " << info0.nfields+1 << " fields and found " << length << " (ignored) " << endl;
    return NOTOK;
  }
  if(length < 2) {
    cerr << "WordKey::Set: expected at least two fields in line" << endl;
    return NOTOK;
  }

  Clear();

  //
  // Handle word and its suffix
  //
  int i = 0;
  {
    //
    // Get the word
    //
    String* word = (String*)fields.Get_First();
    if(word == 0) {
      cerr << "WordKey::Set: failed to word " << i << endl;
      return NOTOK;
    }
    if(word->nocase_compare("<undef>") == 0)
      UndefinedWord();
    else
      SetWord(*word);
    fields.Remove(word);
    i++;

    //
    // Get the word suffix status
    //
    String* suffix = (String*)fields.Get_First();
    if(suffix == 0) {
      cerr << "WordKey::Set: failed to word suffix " << i << endl;
      return NOTOK;
    }
    if(suffix->nocase_compare("<undef>") == 0)
      UndefinedWordSuffix();
    else
      SetDefinedWordSuffix();
    fields.Remove(suffix);
  }

  //
  // Handle numerical fields
  //
  int j;
  for(j = 1; i < info0.nfields; i++, j++) {
    String* field = (String*)fields.Get_First();

    if(field == 0) {
      cerr << "WordKey::Set: failed to retrieve field " << i << endl;
      return NOTOK;
    }
    
    if(field->nocase_compare("<undef>") == 0) {
      Undefined(j);
    } else {
      unsigned int value = atoi(field->get());
      Set(j, value);
    }

    fields.Remove(field);
  }

  return OK;
}

ostream &operator << (ostream &o, const WordKey &key)
{
  String tmp;
  key.Get(tmp);
  o << tmp;
  return o;
}

void WordKey::Print() const
{
  cout << *this;
}


// ********************************
// ************** DEBUGING ********
// ********************************
void
WordKey::ShowPacked(const String& key,int type/*=0*/)
{
    int i;
    char c;
    if(1)
    {
	for(i=0; i<key.length(); i++)
	{
	    c = (isprint(key[i]) ? key[i] : '#');
	    printf("%c-%2x ",c,key[i]);
	}
	printf("\n");
    }
    if(type>0)
    {
	extern void show_bits(int v,int n);
	for(i=0; i<key.length(); i++)
	{
	    show_bits(key[i],-8);
	}
	printf("\n");
	printf("^0      ^1      ^2      ^3      ^4      ^5      ^6      ^7\n");
	printf("0123456701234567012345670123456701234567012345670123456701234567\n");

    }
}

void
WordKey::SetRandom()
{
    int j;
    for(j=1;j<NFields();j++)
    {
	int nbits=WordKey::Info()->sort[j].bits;
	WordKeyNum max=(1<<(nbits));
	if(nbits==32){max=0xffffffff;}
	WordKeyNum val=HtRandom::rnd(0,max);
//  	if(nbits==1)printf("field:%d :val:%d max:%d********************************\n",j,val,max);
	Set(j,val);
    }
    int strl=HtRandom::rnd(0,50);
    int i;
    String Word;
    for(i=0;i<strl;i++)
    {
	char c;
	c=HtRandom::rnd('a','z');
  	Word << c;
    }
    SetWord(Word);
}

