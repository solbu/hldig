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
// $Id: WordKey.cc,v 1.3.2.3 1999/12/09 11:31:26 bosc Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <iostream.h>

#include "WordKey.h"
#include <ctype.h>

//
// C comparison function interface for Berkeley DB (bt_compare)
// Just call the static Compare function of WordKey. It is *critical*
// that this function is as fast as possible. See the Berkeley DB
// documentation for more information on the return values.
//
int word_db_cmp(const DBT *a, const DBT *b)
{
  return WordKey::Compare((char*)a->data, a->size, (char*)b->data, b->size);
}

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
  const struct WordKeyInfo& info = word_key_info;
  //
  // Walk the fields in sorting order. As soon as one of them
  // does not compare equal, return.
  //
  for(int j = 0; j < info.nfields; j++) 
  {
    //
    // Only compare fields that are set in both key
    //
    if(!IsDefinedInSortOrder(j) || !other.IsDefinedInSortOrder(j)) continue;

    int k = info.sort[j].index;

    switch(info.sort[j].type) {
    case WORD_ISA_String:
      if(!IsDefinedWordSuffix()) {
//  	  cout << "COMPARING UNCOMPLETE WORDS IN WORDKEY" << endl;
	if(pool_String[k] != other.pool_String[k].sub(0, pool_String[k].length()))
	  return 0;
      } else {
	if(pool_String[k] != other.pool_String[k])
	  return 0;
      }
      break;
#define STATEMENT(type) \
    case WORD_ISA_##type: \
      if(pool_##type[k] != other.pool_##type[k]) return 0; \
      break;

#include"WordCaseIsAStatements.h"
    }
  }
  return 1;
}

//
// Compare <a> and <b> in the Berkeley DB fashion. 
// <a> and <b> are packed keys.
//
int WordKey::Compare(const String& a, const String& b)
{
  return WordKey::Compare(a, a.length(), b, b.length());
}

//
// Compare <a> and <b> in the Berkeley DB fashion. 
// <a> and <b> are packed keys.
//
int WordKey::Compare(const char *a, int a_length, const char *b, int b_length)
{
  const struct WordKeyInfo& info = word_key_info;

  if(a_length < info.minimum_length || b_length < info.minimum_length) {
    cerr << "WordKey::Compare: key length for a or b < info.minimum_length\n";
    return NOTOK;
  }

  //
  // Walk the fields in sorting order. As soon as one of them
  // does not compare equal, return.
  //
  for(int j = 0; j < info.nfields; j++) 
  {
    //
    // To sort in inverted order, just swap parameters.
    //
    if(info.sort[j].direction == WORD_SORT_DESCENDING) 
    {
      const char* tmp = a;
      int tmp_length = a_length;
      a = b; a_length = b_length; b = tmp; b_length = tmp_length;
    }
    switch(info.sort[j].type) {
    case WORD_ISA_String:
      {
	const char* p1 = a + info.sort[j].bytes_offset;
	int p1_length = a_length - info.sort[j].bytes_offset;
	const char* p2 = b + info.sort[j].bytes_offset;
	int p2_length = b_length - info.sort[j].bytes_offset;
	int len = p1_length > p2_length ? p2_length : p1_length;
	for (p1 = a + info.sort[j].bytes_offset, p2 = b + info.sort[j].bytes_offset; len--; ++p1, ++p2)
	  if (*p1 != *p2)
	    return ((int)*p1 - (int)*p2);
	if(p1_length != p2_length)
	  return p1_length - p2_length;
      }
      break;
#ifdef WORD_HAVE_TypeA
    case WORD_ISA_TypeA:
#endif /* WORD_HAVE_TypeA */
#ifdef WORD_HAVE_TypeB
    case WORD_ISA_TypeB:
#endif /* WORD_HAVE_TypeB */
#ifdef WORD_HAVE_TypeC
    case WORD_ISA_TypeC:
#endif /* WORD_HAVE_TypeC */
      {
	unsigned int p1;
	unsigned int p2;
	WordKey::UnpackNumber(&a[info.sort[j].bytes_offset],
			      info.sort[j].bytesize,
			      &p1,
			      info.sort[j].lowbits,
			      info.sort[j].bits);
	
	WordKey::UnpackNumber(&b[info.sort[j].bytes_offset],
			      info.sort[j].bytesize,
			      &p2,
			      info.sort[j].lowbits,
			      info.sort[j].bits);
	if(p1 != p2)
	  return p1 - p2;
      }
      break;
    default:
      cerr << "WordKey::Compare: invalid type " << info.sort[j].type << " for field (in sort order)" << j << "\n";
      break;
    }
    if(info.sort[j].direction == WORD_SORT_DESCENDING) {
      const char* tmp = a;
      int tmp_length = a_length;
      a = b; a_length = b_length; b = tmp; b_length = tmp_length;
    }
  }

  //
  // If we reach this point, everything compared equal
  //
  return 0;
}

//
// Compare object and <other> using comparison of their packed form
//
int WordKey::PackEqual(const WordKey& other) const
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
    const struct WordKeyInfo& info = word_key_info;
    int nfields=word_key_info.nfields;
    if(position<0){position=nfields;}
    int i;

    if(position==0){cerr << "SetToFollowingInSortOrder with position=0" << endl;return NOTOK;}
    
    if(position>1)
    {// increment if numerical field
	int bits=info.sort[position].bits;
	int f=GetInSortOrder(position-1);
	// check which direction we're going
	if(info.sort[position].direction == WORD_SORT_ASCENDING)
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
    for(i=position;i<nfields;i++){if(IsDefinedInSortOrder(i)){SetInSortOrder(i,0);}}

    return(OK);
}



//
// Return true if the key may be used as a prefix for search.
// In other words return true if the fields set in the key
// are all contiguous, starting from the first field in sort order.
//
int WordKey::Prefix() const
{
  const struct WordKeyInfo& info = word_key_info;
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
  for(int j = 1; j < info.nfields; j++) 
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

    for(int i=0;i<word_key_info.nfields;i++)
    {
	if(first_skip_field==-2 && !IsDefinedInSortOrder(i)){first_skip_field=-1;}
	else
	if(first_skip_field==-2 && i==0 && !IsDefinedWordSuffix()){first_skip_field=-1;}
	else
	if(first_skip_field==-1 &&  IsDefinedInSortOrder(i)){first_skip_field=i;break;}
    }
    if(first_skip_field<0){first_skip_field=word_key_info.nfields;}
    return(first_skip_field);
}

//
// Unset all fields past the first unset field
// Return the number of fields in the prefix or 0 if
// first field is not set, ie no possible prefix.
//
int WordKey::PrefixOnly()
{
  const struct WordKeyInfo& info = word_key_info;
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

  for(int j = 1; j < info.nfields; j++) 
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
int WordKey::Unpack(const String& data)
{
  const char* string = data;
  int length = data.length();

  const struct WordKeyInfo& info = word_key_info;

  if(length < info.minimum_length) {
    cerr << "WordKey::Unpack: key record length < info.minimum_length\n";
    return NOTOK;
  }

  for(int i = 0; i < info.nfields; i++) {

    switch(info.fields[i].type) {

    case WORD_ISA_String:
      pool_String[info.fields[i].index].set(&string[info.fields[i].bytes_offset], length - info.minimum_length);
      SetDefined(i);
      SetDefinedWordSuffix();
      break;

#define STATEMENT(type) \
    case WORD_ISA_##type: \
      { \
	unsigned int value = 0; \
	WordKey::UnpackNumber(&string[info.fields[i].bytes_offset], \
				 info.fields[i].bytesize, \
				 &value, \
				 info.fields[i].lowbits, \
				 info.fields[i].bits); \
	pool_##type[info.fields[i].index] = (##type)value; \
	SetDefined(i); \
      } \
      break;

#include"WordCaseIsAStatements.h"
    }
  }

  return OK;
}

//
// Pack object into the <packed> string
//
int WordKey::Pack(String& packed) const
{
  const struct WordKeyInfo& info = word_key_info;

  char* string;
  int length = info.minimum_length;

  length += pool_String[0].length();

  if((string = (char*)malloc(length)) == 0) {
    cerr << "WordKey::Pack: malloc returned 0\n";
    return NOTOK;
  }
  memset(string, '\0', length);

  for(int i = 0; i < info.nfields; i++) {

    switch(info.fields[i].type) {
    case WORD_ISA_String:
      memcpy(&string[info.fields[i].bytes_offset], pool_String[info.fields[i].index].get(), pool_String[info.fields[i].index].length());
      break;

#define STATEMENT(type) \
    case WORD_ISA_##type: \
      WordKey::PackNumber((unsigned int)pool_##type[info.fields[i].index], \
			  &string[info.fields[i].bytes_offset], \
			  info.fields[i].bytesize, \
			  info.fields[i].lowbits, \
			  info.fields[i].lastbits); \
      break;

#include"WordCaseIsAStatements.h"
    }
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
  const struct WordKeyInfo& info = word_key_info;

  for(int i = 0; i < info.nfields; i++) {
    if(!IsDefined(i) && other.IsDefined(i)) {
      switch(info.fields[i].type) {

#define STATEMENT(type) \
      case WORD_ISA_##type: \
	Set##type(other.Get##type(i), i); \
	break;

#include"WordCaseIsAStatements.h"

      }
    }
  }

  return OK;
}

ostream &operator << (ostream &o, const WordKey &key)
{
  const struct WordKeyInfo& info = word_key_info;

  //
  // Walk the fields in sorting order. As soon as one of them
  // does not compare equal, return.
  //
//    if(!key.IsDefinedWordSuffix()){cerr << "word suffix undefined for word:" << key.GetWord()<<endl;}
  for(int j = 0; j < info.nfields; j++) 
  {
      int index = info.sort[j].index;
      if(!key.IsDefinedInSortOrder(j)){o << "<UNDEF>";}
      else
      {
	  switch(info.sort[j].type) 
	  {
	                 case WORD_ISA_String:  o << key.pool_String[index];  break;
#define STATEMENT(type)  case WORD_ISA_##type:  o << key.pool_##type[index];  break;
#include"WordCaseIsAStatements.h"
	  default:
	      cerr << "WordKey::operator <<: invalid type " << info.sort[j].type << " for field (in sort order) " << j << "\n";
	      break;
	  }
      }
      if(j==0 && key.IsDefinedInSortOrder(j) && !key.IsDefinedWordSuffix())
      {
	  o << "<UNDEF>";
      }
      o << "\t";
  }
  return o;
}


static int
read_fields(istream &is,int maxn,char *res,int maxres)
{
    int i=0;
    for(int j=0;j<maxn;j++)
    {
	char c;
	for(;;)// skip spaces
	{
	    c=is.get();
	    if(is.eof()){return j;}
	    if(!isspace(c)){is.putback(c);break;}
	}
	if(j!=0 && i<maxres-1){res[i++]=' ';}
	for(;;i++)// get string into tmp
	{
	    c=is.get();
	    if(is.eof()){res[i]='\0';return j+1;}
	    if(isspace(c)){is.putback(c);break;}
	    if(i>=maxres-1)
	    {cerr << "WordKey operator>>: field :" <<j <<" too long!" <<endl;return j;}
	    res[i]=c;
	}
	res[i]='\0';
    }
//      cout << "read_fields: result:" << res << endl;
    return maxn;
}


int
WordKey::Set(const char *s)
{
    const int max_tmp=1024;
    char tmp[max_tmp];
    Clear();
    int k=0;
    int status=0;
    int j;
    for(j=0;j<word_key_info.nfields;j++)
    {
	// get a string (but check if not too long)
	//  	replaces: is >> tmp; !!
	char c;
	for(;;)// skip spaces
	{
	    c=s[k++];
	    if(c=='\0'){status=1;break;}
	    if(!isspace(c)){k--;break;}
	}
	if(status==1){break;}// premature end
	int i;
	for(i=0;;i++)// get string into tmp
	{
	    c=s[k++];
	    if(c=='\0'){status=2;break;}
	    if(isspace(c)){k--;break;}
	    if(i>=max_tmp-1)
	    {cerr << "WordKey::Set(char *): field :" <<j <<" too long!" <<endl;break;}
	    tmp[i]=c;
	}
	tmp[i]='\0';
	///

	// now parse this field's string
	if(!strcmp(tmp,"<UNDEF>")){UndefinedInSortOrder(j);}
	else
	{
	    if(j==0)
	    {// this is the WORD field
//  		cout << "WordKey::Set(char *): --- word field :|" << tmp << "|" << endl;
		// check for undefined suffix
		if(strlen(tmp)>=strlen("<UNDEF>") && 
		   !strcmp(tmp+strlen(tmp)-strlen("<UNDEF>"),"<UNDEF>"))
		{		    
		    // strip <UNDEF> suffix from word
		    tmp[strlen(tmp)-strlen("<UNDEF>")]='\0';
		    SetWord(tmp);
		    UndefinedWordSuffix();
		}
		else
		{SetWord(tmp);}// normal Word

	    }
	    else
	    {
		char *chk;
		int v=strtol(tmp,&chk,0);
		if(*tmp=='\0' || *chk!='\0')
		{// argh this is not a valid numerical field
		    status=3;
		    break;
		}
		SetInSortOrder(j,v);
	    }
	}

	if(status==2)
	{
	    if(j<word_key_info.nfields -1)
	    {// premature end
		break;
	    }
	    else{status=0;}// ok, this was the end anyways
	}
    }
    if(status>2)
    {
	cerr << "WordKey::Set(" << s << ") : bad string format: for field: " << j <<  endl;
	return NOTOK;
    }
    return OK;
}


istream &
operator >> (istream &is,  WordKey &key)
{
    const int max_tmp=1024;
    char tmp[max_tmp];
    int nr=read_fields(is,word_key_info.nfields,tmp,max_tmp);
    if(nr != word_key_info.nfields)
    {
	// nr==0 might be an end of file while reading WordList...
	if(nr!=0){cerr << "WordKey::operator >> read failed!:not enough fields" << endl;}
    }
    else
    if(key.Set(tmp) == NOTOK){cerr << "WordKey::operator >> read failed!" << endl;}
    return is;
}

void WordKey::Print() const
{
  cout << *this;
}
