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
// $Id: WordKey.cc,v 1.3.2.12 2000/01/05 11:40:31 loic Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <iostream.h>

#include "WordKey.h"
#include <ctype.h>
#include<fstream.h>
#include"HtRandom.h"




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
  const WordKeyInfo& info0 = *WordKey::info();
  //
  // Walk the fields in sorting order. As soon as one of them
  // does not compare equal, return.
  //
  for(int j = 0; j < info0.nfields; j++) 
  {
    //
    // Only compare fields that are set in both key
    //
    if(!IsDefinedInSortOrder(j) || !other.IsDefinedInSortOrder(j)) continue;

    switch(info0.sort[j].type) {
    case WORD_ISA_String:
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
      if(GetInSortOrder(j) != other.GetInSortOrder(j)) return 0;
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
  const WordKeyInfo& info0 = *WordKey::info();

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
WordKey::SetToFollowingInSortOrder(int position)
{
    const WordKeyInfo& info0 = *WordKey::info();
    if(position<0){position=nfields();}
    int i;

    if(position==0){cerr << "SetToFollowingInSortOrder with position=0" << endl;return NOTOK;}
    
    if(position>1)
    {// increment if numerical field
	int bits=info0.sort[position].bits;
	int f=GetInSortOrder(position-1);
	// check which direction we're going
	if(info0.sort[position].direction == WORD_SORT_ASCENDING)
	{
	    if( bits < 32){if((f+1) >= 1<<bits){return NOTOK;}}// overflow
            //TODO: add bits>=32 for overflow case....
	    SetInSortOrder(position-1,f+1);
	}
	else
	{
	    if(f>0){SetInSortOrder(position-1,f-1);}
	    else{return (NOTOK);}// underflow
	}
    }
    else
    {// if non numerical (field 0=word) add a (char)1 to string
//	if(WLdebug>1){cout << "oops,  previous field is string!" << endl;}
	GetWord() << (char) 1;
    }

    // zero fields >=position'th
    for(i=position;i<nfields();i++){if(IsDefinedInSortOrder(i)){SetInSortOrder(i,0);}}

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
  const WordKeyInfo& info0 = *WordKey::info();
  //
  // If all fields are set, it can be considered as a prefix although
  // it really is a fully qualified key.
  //
  if(Filled()) return OK;
  //
  // If the first field is not set this cannot be a prefix
  //
  if(!IsDefinedInSortOrder(0)) return NOTOK;
  
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
    if(IsDefinedInSortOrder(j))
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
// find first field that must be checked for skip
int 
WordKey::FirstSkipField() const
{
    int first_skip_field=-2;

    for(int i=0;i<nfields();i++)
    {
	if(first_skip_field==-2 && !IsDefinedInSortOrder(i)){first_skip_field=-1;}
	else
	if(first_skip_field==-2 && i==0 && !IsDefinedWordSuffix()){first_skip_field=-1;}
	else
	if(first_skip_field==-1 &&  IsDefinedInSortOrder(i)){first_skip_field=i;break;}
    }
    if(first_skip_field<0){first_skip_field=nfields();}
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
  const WordKeyInfo& info0 = *WordKey::info();
  //
  // If all fields are set, the whole key is the prefix.
  //
  if(Filled()) return OK;
  //
  // If the first field is not set there is no possible prefix
  //
//    cerr << "prefix only: input key:" << *this <<endl;
  if(!IsDefinedInSortOrder(0)) 
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
    if(IsDefinedInSortOrder(j)) 
    {
	if(found_unset) {SetInSortOrder(j,0);UndefinedInSortOrder(j);}
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
  const WordKeyInfo& info0 = *WordKey::info();

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
      SetInSortOrder(j,value);
  }

  return OK;
}

//
// Pack object into the <packed> string
//
int 
WordKey::Pack(String& packed) const
{
  const WordKeyInfo& info0 = *WordKey::info();

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
      WordKey::PackNumber(GetInSortOrder(info0.encode[i].sort_position), 
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
  const WordKeyInfo& info0 = *WordKey::info();

  
  for(int j = 0; j < info0.nfields; j++) 
  {
    if(!IsDefinedInSortOrder(j) && other.IsDefinedInSortOrder(j)) 
    {
      switch(info0.sort[j].type) 
      {
      case WORD_ISA_String: 
	  SetWord(other.GetWord());
	  break;
      default:
	  SetInSortOrder(j,other.GetInSortOrder(j)); 
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
  const WordKeyInfo& info0 = *WordKey::info();

  //
  // Walk the fields in sorting order. As soon as one of them
  // does not compare equal, return.
  //
  for(int j = 0; j < info0.nfields; j++) 
  {
      if(!IsDefinedInSortOrder(j)){buffer << "<UNDEF>";}
      else
      {
	  switch(info0.sort[j].type) 
	  {
	  case WORD_ISA_String:  buffer << GetWord();          break;
	  case WORD_ISA_NUMBER:  buffer << GetInSortOrder(j);  break;
  	  default:
  	      cerr << "WordKey::operator <<: invalid type " << info0.sort[j].type << " for field (in sort order) " << j << "\n";
  	      return NOTOK;
	  }
      }
      //
      // Output virtual word suffix field
      //
      if(j==0) {
	if(IsDefinedInSortOrder(j) && !IsDefinedWordSuffix()) {
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
  const WordKeyInfo& info0 = *WordKey::info();
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
      UndefinedInSortOrder(j);
    } else {
      unsigned int value = atoi(field->get());
      SetInSortOrder(j, value);
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
class BitStream;
void
WordKey::show_packed(const String& key,int type/*=0*/)
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
    for(j=1;j<nfields();j++)
    {
	int nbits=WordKey::info()->sort[j].bits;
	WordKeyNum max=(1<<(nbits));
	if(nbits==32){max=0xffffffff;}
	WordKeyNum val=HtRandom::rnd(0,max);
//  	if(nbits==1)printf("field:%d :val:%d max:%d********************************\n",j,val,max);
	SetInSortOrder(j,val);
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



void
WordKeyInfo::SetKeyDescriptionRandom(int maxbitsize/*=100*/,int maxnnfields/*=10*/)
{
    int bitsize=HtRandom::rnd(1,maxbitsize/8);
    bitsize*=8;// byte aligned (for word)
    int nfields0=HtRandom::rnd(2,bitsize > maxnnfields ? maxnnfields : bitsize);
    int i;
    char sdesc[10000];
    char sfield[10000];
    sdesc[0]=0;

    // build sortorder
    sprintf(sfield,"nfields: %d",nfields0);
    strcat(sdesc,sfield);

    int *sortv=HtRandom::randomize_v(NULL,nfields0-1);

    int bits;
    int totbits=0;
    // build fields
    for(i=0;i<nfields0-1;i++)
    {
	int maxf=(bitsize-totbits)-nfields0+i+2;
	if(maxf>32){maxf=32;}
	bits=HtRandom::rnd(1,maxf);
	if(i==nfields0-2)
	{
	    bits=maxf;
	    if((totbits+bits)%8)
	    {// argh really bad case :-(
		WordKeyInfo::SetKeyDescriptionRandom(maxbitsize,maxnnfields);
		return;
	    }
	}
	totbits+=bits;
	sprintf(sfield,"/Field%d %d %d",i,bits,sortv[i]+1);
	strcat(sdesc,sfield);
    }
    sprintf(sfield,"/Word 0 0");
    strcat(sdesc,sfield);

//      if(verbose)cout << "SetRandomKeyDesc:" << sdesc << endl;
    WordKeyInfo::SetKeyDescriptionFromString(sdesc);

}
