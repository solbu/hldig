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
// $Id: WordKey.cc,v 1.3.2.8 1999/12/21 12:03:29 bosc Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <iostream.h>

#include "WordKey.h"
#include <ctype.h>
#include<iostream.h>
#include<fstream.h>

WordKeyInfo *word_key_info = NULL;

void 
WordKeyField::nprint(char c,int n)
{
    for(int i=0;i<n;i++)
    {
	if(!(i%4)){printf("%c",'a'+i/4);}
	else{printf("%c",c);}
    }
}

void 
WordKeyInfo::SetKeyDescriptionFromString(const String &desc)
{
    if(word_key_info){delete word_key_info;}
    word_key_info=new WordKeyInfo();
    word_key_info->SetDescriptionFromString(desc);
}
void 
WordKeyInfo::SetKeyDescriptionFromFile(const String &filename)
{
    if(word_key_info){delete word_key_info;}
    word_key_info=new WordKeyInfo();
    word_key_info->SetDescriptionFromFile(filename);
}

void 
WordKeyField::show()
{
    nprint(' ',bits_offset);
    printf("\"%s\" %3d %3d type:%2d lowbits:%2d lastbits:%2d\n",(char *)name,encoding_position,sort_position,type, lowbits, lastbits);
    nprint(' ',bits_offset);
    printf("|---bytesize:%2d bytes_offset:%2d bits:%2d direction:%2d\n", bytesize, bytes_offset, bits, direction);
    nprint(' ',bits_offset);
    printf("|---encoding_position:%2d sort_position:%2d bits_offset:%2d\n", encoding_position, sort_position, bits_offset);

}

WordKeyField::WordKeyField(WordKeyField *previous,char *nname,int nbits,int nencoding_position, int nsort_position )
{
    encoding_position=nencoding_position;
    sort_position=nsort_position;
    name = strdup(nname);

    type=(sort_position ? WORD_ISA_NUMBER : WORD_ISA_String);
    bits = nbits;
    bits_offset = (previous ?  previous->bits_offset + previous->bits  :  0 );

    if(bits_offset%8 && sort_position==0)
    {
	cerr << "WordKeyField::WordKeyField: begining of word is at:" << bits_offset
	     << " should be byte aligned" << endl;	
	bits_offset+=8-(bits_offset%8);
    }

    if(bits_offset<0 || bits_offset>WORDKEYFIELD_BITS_MAX*WORD_KEY_MAX_NFIELDS)
    {
	cerr << "WordKeyField::WordKeyField: bits_offset:" << bits_offset << " is not correct" << endl;
    }
    bytes_offset = bits_offset/8;
    bytesize = (bits_offset+bits-1)/8 - bytes_offset + 1;
    lastbits = (bits_offset+bits)%8;
    lowbits  =  bits_offset%8;
    direction=WORD_SORT_ASCENDING;
}
void 
WordKeyInfo::Initialize( String &line)
{
    StringList fields(line, "\t ");

    String *found;
    fields.Start_Get();
    int i=0;
    int nnfields=-1;
    while((found = (String*)fields.Get_Next()))
    {
	if(i==1)
	{
	    nnfields=atoi((char *)*found);
	}
	else
	{
	    if(i>1 || *found!=String("nfields:"))
	    {
		cerr << "WordKeyInfo::Initialize: syntax error at begining" << endl;
	    }
	}
	i++;
    }
    if(nnfields<2 || nnfields>=WORD_KEY_MAX_NFIELDS)
    {
	cerr << "WordKeyInfo::Initialize: invalid nfields:" << nnfields << endl;
    }
    Initialize(nnfields);
}
void 
WordKeyInfo::Initialize( int nnfields)
{
//      cerr << "WordKeyInfo::Initialize: nfields:" << nnfields << endl;
    nfields = nnfields;
    sort = new WordKeyField[nfields];
    encode = new WordKeyField[nfields];
    minimum_length = 0;
    previous=NULL;
    encoding_position=0;
}


void 
WordKeyInfo::AddFieldInEncodingOrder(String &name,int bits, int sort_position)
{
    int i;
    for(i=0;i<encoding_position;i++)
    {
	if(encode[i].sort_position==sort_position)
	{
	    cerr << "WordKeyInfo::AddFieldInEncodingOrder: syntax error in key: " << endl;
	    cerr << "WordKeyInfo::AddFieldInEncodingOrder: found sort position twice " << endl;
	}
    }
	

    WordKeyField tmp( previous, name, bits, encoding_position, sort_position );
    sort[sort_position]       = tmp;
    encode[encoding_position] = tmp;
//    printf ("srt:");sort[sort_position].show();
    previous = sort + sort_position;

    encoding_position++;
    if(sort_position == 0)
    {
	// this should be the last encoded field...
	minimum_length = sort[sort_position].bytes_offset;
	// verifiy some things
	int fail=0;
	if(encoding_position!=nfields)
	{
	    cerr << "WordKeyInfo::AddFieldInEncodingOrder: didnt find the right nuimber of fields" << endl;
	    cerr << "found:" << encoding_position << " expected:" << nfields << endl;
	}
  	for(i=0;i<nfields;i++)
  	{
  	    if(sort[i].sort_position!=i){fail=1;}
  	    if(encode[sort[i].encoding_position].sort_position!=i){fail=2;}

  	    if(sort[encode[i].sort_position].encoding_position!=i){fail=3;}
  	    if(encode[i].encoding_position!=i){fail=4;}
  	}
	if(fail)
	{
	    cerr << "WordKeyInfo::AddFieldInEncodingOrder: bad syntax:" << fail << endl;
	    for(i=0;i<nfields;i++)
	    {
		cerr << "field in encoding order:" << i << endl;
		encode[i].show();
	    }
	    for(i=0;i<nfields;i++)
	    {
		cerr << "field in sort order:" << i << endl;
		sort[i].show();
	    }
	}
    }
}

void 
WordKeyInfo::AddFieldInEncodingOrder(const String &line)
{
    StringList fields(line, "\t ");
	
    fields.Start_Get();

    String *name=(String*)fields.Get_Next();
    if(!name){cerr << "WordKeyInfo::AddFieldInEncodingOrder: bad syntax:" << line << endl;}

    String *sbits=(String*)fields.Get_Next();
    if(!sbits){cerr << "WordKeyInfo::AddFieldInEncodingOrder: bad syntax:" << line << endl;}
    int bits=atoi((char *)*sbits);
    if(bits<0 || bits>WORDKEYFIELD_BITS_MAX){cerr << "WordKeyInfo::AddFieldInEncodingOrder: strange value:" << line << endl;}

    String *ssortpos=(String*)fields.Get_Next();
    if(!ssortpos){cerr << "WordKeyInfo::AddFieldInEncodingOrder: bad syntax:" << line << endl;}
    int sortpos=atoi((char *)*ssortpos);
    if(sortpos<0 || sortpos>nfields){cerr << "WordKeyInfo::AddFieldInEncodingOrder: strange value:" << line << endl;}

    AddFieldInEncodingOrder(*name, bits, sortpos);
}

void
WordKeyInfo::SetDescriptionFromString(const String &desc)
{
//      cerr << "WordKeyInfo::SetKeyDescriptionFromString:\"" << desc << "\""<< endl;
    StringList lines(desc, "/");
    String *found;
    int initok=0;
    lines.Start_Get();
    while((found = (String*)lines.Get_Next()))
    {
	if(!initok){Initialize(*found);initok=1;}
	else
	{
	    if( encoding_position >= nfields )
	    {cerr << "WordKeyInfo::SetKeyDescriptionFromString: unexpected line " << *found << endl;}
	    AddFieldInEncodingOrder(*found);
	}
    }
}
void
WordKeyInfo::SetDescriptionFromFile(const String &filename)
{
    cerr << "WordKeyInfo::SetKeyDescriptionFromFile:" << filename << endl;
    nfields=-1;
    ifstream in((const char *)filename);
#define WORD_BUFFER_SIZE	1024
    char buffer[WORD_BUFFER_SIZE];
    String line;
    int line_number = 0;

    while(!in.eof())
    {
	line_number++;

	in.get(buffer, WORD_BUFFER_SIZE);
	line.append(buffer);
	//
	// Get the terminator. I love iostream :-(
	//
	if(!in.eof()) 
	{
	    char c;
	    in.get(c);
	    if(c == '\n') 
		line.append(c);
	    in.putback(c);
	}
	//
	// Join big lines
	//
	if(line.last() != '\n' && line.last() != '\r' && !in.eof())
	    continue;
	//
	// Eat the terminator
	//
	if(!in.eof()) in.get();
	//
	// Strip line terminators from line
	//
	line.chop("\r\n");
	//
	// If line ends with a \ continue
	//
	if(line.last() == '\\') {
	    line.chop(1);
	    if(!in.eof())
		continue;
	}
      
	if(!line.empty()) 
	{
	    if(!in.good())
	    {
		cerr << "WordKeyInfo::SetKeyDescription: line " << line_number << " : " << line << endl
		     << " input from file failed (A)" << endl;
		break;
	    }

	    if(line[0] != '#')
	    {

		if( nfields < 0 ){Initialize(line);}
		else
		{
		    AddFieldInEncodingOrder(line);
		}

		if( encoding_position >= nfields )
		{cerr << "WordKeyInfo::SetKeyDescription: unexpected line " << line_number << " : " << line << endl;}
	    }
      
	    if(in.eof()){break;}
	    if(!in.good())
	    {
		cerr << "WordKeyInfo::SetKeyDescription: line " << line_number << " : " << line << endl
		     << " input from stream failed (B)" << endl;
		break;
	    }
	}

	line.trunc();
    }
}

void 
WordKeyInfo::show()
{
    printf("-----------------------------------------\n");
    printf("nfields:%3d minimum_length:%3d\n",nfields,minimum_length);
    int i,j;
    for(i=0;i<nfields;i++)
    {
	for(j=0;j<nfields;j++){if(sort[j].encoding_position==i)break;}
	if(j==nfields)
	{
	    cerr << "WordKeyInfo::show field not found !!!!!!!!!!!!!! " <<endl;
	    j=0;
	}
	sort[j].show();
    }
    char str[WORDKEYFIELD_BITS_MAX*WORD_KEY_MAX_NFIELDS];
    for(i=0;i<WORDKEYFIELD_BITS_MAX*WORD_KEY_MAX_NFIELDS;i++)
    {str[i]='_';}
    int tmx=0;
    for(j=0;j<nfields;j++)
    {
	for(i=0;i<sort[j].bits;i++)
	{
	    char c=(j%10)+'0';
	    int pos=sort[j].bits_offset+i;
	    if(str[pos]!='_'){c='X';}
	    str[pos]=c;
	    if(tmx<pos){tmx=pos;}
	}
    }
    str[tmx+1]=0;
    printf("%s (bits)\n",str);
    printf("^0      ^1      ^2      ^3      ^4      ^5      ^6      ^7\n");
    printf("0123456701234567012345670123456701234567012345670123456701234567\n");
}

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
  const struct WordKeyInfo& info = *word_key_info;
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

    switch(info.sort[j].type) {
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
  const struct WordKeyInfo& info = *word_key_info;

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
      const unsigned char* tmp = (unsigned char *)a;
      int tmp_length = a_length;
      a = b; a_length = b_length; b = (char *)tmp; b_length = tmp_length;
    }
    switch(info.sort[j].type) {
    case WORD_ISA_String:
      {
	const unsigned char* p1 = (unsigned char *)a + info.sort[j].bytes_offset;
	int p1_length = a_length - info.sort[j].bytes_offset;
	const unsigned char* p2 = (unsigned char *)b + info.sort[j].bytes_offset;
	int p2_length = b_length - info.sort[j].bytes_offset;
	int len = p1_length > p2_length ? p2_length : p1_length;
	for (p1 = (unsigned char *)a + info.sort[j].bytes_offset, p2 = (unsigned char *)b + info.sort[j].bytes_offset; len--; ++p1, ++p2)
	  if (*p1 != *p2)
	    return ((int)*p1 - (int)*p2);
	if(p1_length != p2_length)
	  return p1_length - p2_length;
      }
      break;
    case WORD_ISA_NUMBER:
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
      const unsigned char* tmp = (unsigned char *)a;
      int tmp_length = a_length;
      a = b; a_length = b_length; b = (char *)tmp; b_length = tmp_length;
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
    const struct WordKeyInfo& info = *word_key_info;
    if(position<0){position=nfields();}
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
    for(i=position;i<nfields();i++){if(IsDefinedInSortOrder(i)){SetInSortOrder(i,0);}}

    return(OK);
}



//
// Return true if the key may be used as a prefix for search.
// In other words return true if the fields set in the key
// are all contiguous, starting from the first field in sort order.
//
int WordKey::Prefix() const
{
  const struct WordKeyInfo& info = *word_key_info;
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
int WordKey::PrefixOnly()
{
  const struct WordKeyInfo& info = *word_key_info;
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

  const struct WordKeyInfo& info = *word_key_info;

  if(length < info.minimum_length) {
    cerr << "WordKey::Unpack: key record length < info.minimum_length\n";
    return NOTOK;
  }

  SetWord(String(&string[info.sort[0].bytes_offset], length - info.minimum_length));

  for(int j = 1; j < info.nfields; j++) 
  {
      WordKeyNum value = 0; 
      WordKey::UnpackNumber(&string[info.sort[j].bytes_offset], 
			    info.sort[j].bytesize, 
			    &value, 
			    info.sort[j].lowbits, 
			    info.sort[j].bits); 
      SetInSortOrder(j,value);
  }

  return OK;
}

//
// Pack object into the <packed> string
//
int WordKey::Pack(String& packed) const
{
  const struct WordKeyInfo& info = *word_key_info;

  char* string;
  int length = info.minimum_length;

  length += kword.length();

  if((string = (char*)malloc(length)) == 0) {
    cerr << "WordKey::Pack: malloc returned 0\n";
    return NOTOK;
  }
  memset(string, '\0', length);

  memcpy(&string[info.sort[0].bytes_offset], kword.get(), kword.length());
  for(int i = 0; i < info.nfields-1; i++) 
  {
      WordKey::PackNumber(GetInSortOrder(info.encode[i].sort_position), 
			  &string[info.encode[i].bytes_offset], 
			  info.encode[i].bytesize, 
			  info.encode[i].lowbits, 
			  info.encode[i].lastbits); 
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
  const struct WordKeyInfo& info = *word_key_info;

  
  for(int j = 0; j < info.nfields; j++) 
  {
    if(!IsDefinedInSortOrder(j) && other.IsDefinedInSortOrder(j)) 
    {
      switch(info.sort[j].type) 
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
  const struct WordKeyInfo& info = *word_key_info;

  //
  // Walk the fields in sorting order. As soon as one of them
  // does not compare equal, return.
  //
  for(int j = 0; j < info.nfields; j++) 
  {
      if(!IsDefinedInSortOrder(j)){buffer << "<UNDEF>";}
      else
      {
	  switch(info.sort[j].type) 
	  {
	  case WORD_ISA_String:  buffer << GetWord();          break;
	  case WORD_ISA_NUMBER:  buffer << GetInSortOrder(j);  break;
  	  default:
  	      cerr << "WordKey::operator <<: invalid type " << info.sort[j].type << " for field (in sort order) " << j << "\n";
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
  const struct WordKeyInfo& info = *word_key_info;
  int length = fields.Count();

  if(length < info.nfields + 1) {
    cerr << "WordKey::Set: expected at least " << info.nfields << " fields and found " << length << " (ignored) " << endl;
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
  for(j = 1; i < info.nfields; i++, j++) {
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


