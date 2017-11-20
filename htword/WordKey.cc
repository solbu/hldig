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
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: WordKey.cc,v 1.9 2004/05/28 13:15:26 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <ctype.h>

#include "WordKey.h"

//
// Returns OK if fields set in 'object' and 'other' are all equal.
//
// Fields not set in either 'object' or 'other' are ignored 
// completely. If the prefix_length is > 0 the 'object' String
// fields are compared to the prefix_length bytes of the 'other'
// String fields only.
//
// This function is useful to compare existing keys with a search
// criterion that may be incomplete. For instance if we look for keys
// that contain words starting with a given prefix or keys that
// are located in a specific document, regardless of their location
// in the document.
//
int WordKey::Equal(const WordKey& other) const
{
  const WordKeyInfo& info = *WordKey::Info();
  //
  // Walk the fields in sorting order. As soon as one of them
  // does not compare equal, return.
  //
  for(int j = 0; j < info.nfields; j++) 
  {
    //
    // Only compare fields that are set in both key
    //
    if(!IsDefined(j) || !other.IsDefined(j)) continue;

    switch(info.sort[j].type) {
    case WORD_ISA_STRING:
      if(!IsDefinedWordSuffix()) {
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
// Compares full WordKey, unlike  Compare_WordOnly.
//
inline int 
WordKey::Compare(const char *a, int a_length, const char *b, int b_length)
{
  const WordKeyInfo& info = *WordKey::Info();

  if(a_length < info.num_length || b_length < info.num_length) {
      fprintf(stderr, "WordKey::Compare: key length %d or %d < info.num_length = %d\n", a_length, b_length, info.num_length);
      return NOTOK;
  }

  //
  // Walk the fields, as soon as one of them does not compare equal,
  // return.
  //

  //
  //  first field: string
  //
  const int p1_length = a_length - info.num_length;
  const int p2_length = b_length - info.num_length;
  {
      int len = p1_length > p2_length ? p2_length : p1_length;
      const unsigned char* p1 = (unsigned char *)a;
      const unsigned char* p2 = (unsigned char *)b;

      for (;len--; ++p1, ++p2) {
    if (*p1 != *p2)
        return (int)*p1 - (int)*p2;
      }
      if(p1_length != p2_length)
    return p1_length - p2_length;
  }
  //
  //  following fields: numerical
  //          But what *are* they?? -- lha
  //
  for(int j = 1; j < info.nfields; j++) 
  {
  WordKeyNum p1;
  int a_index = info.sort[j].bytes_offset + p1_length;
  WordKey::UnpackNumber((unsigned char *)&a[a_index],
            info.sort[j].bytesize,
            p1,
            info.sort[j].lowbits,
            info.sort[j].bits);
  
  WordKeyNum p2;
  int b_index = info.sort[j].bytes_offset + p2_length;
  WordKey::UnpackNumber((unsigned char *)&b[b_index],
            info.sort[j].bytesize,
            p2,
            info.sort[j].lowbits,
            info.sort[j].bits);
  if(p1 != p2)
      return p1 - p2;
  }

  //
  // If we reach this point, everything compared equal
  //
  return 0;
}
//
// Compare <a> and <b> in the Berkeley DB fashion. 
// <a> and <b> are packed keys.
// Only compares "word" part of WordKey, unlike  Compare.
//
inline int 
WordKey::Compare_WordOnly(const char *a, int a_length, const char *b, int b_length)
{
  const WordKeyInfo& info = *WordKey::Info();

  if(a_length < info.num_length || b_length < info.num_length) {
      fprintf(stderr, "WordKey::Compare: key length %d or %d < info.num_length = %d\n", a_length, b_length, info.num_length);
      return NOTOK;
  }

  //
  //  compare first field only: actual word
  //
  const int p1_length = a_length - info.num_length;
  const int p2_length = b_length - info.num_length;
  {
      int len = p1_length > p2_length ? p2_length : p1_length;
      const unsigned char* p1 = (unsigned char *)a;
      const unsigned char* p2 = (unsigned char *)b;

      for (;len--; ++p1, ++p2) {
    if (*p1 != *p2)
        return (int)*p1 - (int)*p2;
      }
      if(p1_length != p2_length)
    return p1_length - p2_length;
  }
  return 0;
}

//
// Compare <a> and <b> in the Berkeley DB fashion. 
// <a> and <b> are packed keys.
// Compares full WordKey, unlike  Compare_WordOnly.
//
int 
WordKey::Compare(const String& a, const String& b)
{
  return WordKey::Compare(a, a.length(), b, b.length());
}

//
// Compare <a> and <b> in the Berkeley DB fashion. 
// <a> and <b> are packed keys.
// Only compares "word" part of WordKey, unlike  Compare.
//
int 
WordKey::Compare_WordOnly(const String& a, const String& b)
{
  return WordKey::Compare_WordOnly(a, a.length(), b, b.length());
}

//
// C comparison function interface for Berkeley DB (bt_compare)
// Just call the static Compare function of WordKey. It is *critical*
// that this function is as fast as possible. See the Berkeley DB
// documentation for more information on the return values.
// Compares full WordKey, unlike  word_only_db_cmp.
//
int
word_db_cmp(const DBT *a, const DBT *b)
{
  return WordKey::Compare((char*)a->data, a->size, (char*)b->data, b->size);
}

//
// C comparison function interface for Berkeley DB (bt_compare)
// Just call the static Compare function of WordKey.
// See the Berkeley DB
// documentation for more information on the return values.
// Only compares text part of the WordKey, unlike  word_db_cmp.
//
int
word_only_db_cmp(const DBT *a, const DBT *b)
{
  return WordKey::Compare_WordOnly((char*)a->data, a->size, (char*)b->data, b->size);
}

//
// Compare current key defined fields with other key defined fields only,
// ignore fields that are not defined in key or other. Return 1 if different
// 0 if equal. If different, position is set to the field number that differ,
// lower is set to 1 if Get(position) is lower than other.Get(position) otherwise
// lower is set to 0.
//
int WordKey::Diff(const WordKey& other, int& position, int& lower)
{
  position = -1;

  if(IsDefined(0) && other.IsDefined(0)) {
    int ret = 0;
    if(other.IsDefinedWordSuffix())
      ret = GetWord().compare(other.GetWord());
    else
      ret = strncmp((char*)GetWord(), (const char*)other.GetWord(), other.GetWord().length());
    if(ret) {
      position = 0;
      lower = ret > 0;
    }
  }

  if(position < 0) {
    int nfields=WordKey::NFields();

    int i;
    for(i = 1; i < nfields; i++) {
      if(IsDefined(i) && other.IsDefined(i) &&
   Get(i) != other.Get(i)) {
  lower = Get(i) < other.Get(i);
  break;
      }
    }
    if(i < nfields)
      position = i;
  }

  return position >= 0;
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

//
// Implement ++ on a key.
//
// It behaves like arithmetic but follows these rules:
// . Increment starts at field <position>
// . If a field value overflows, increment field <position> - 1
// . Undefined fields are ignored and their value untouched
// . Incrementing the word field is done by appending \001
// . When a field is incremented all fields to the left are set to 0
// If position is not specified it is equivalent to NFields() - 1.
// It returns OK if successfull, NOTOK if position out of range or
// WORD_FOLLOWING_ATEND if the maximum possible value was reached.
//
// Examples assuming numerical fields are 8 bits wide:
// 
// 0                 1     2     3       OPERATION             RESULT
// ---------------------------------------------------------------------------------------
// foo     <DEF>     1     1     1 -> SetToFollowing(3) -> foo     <DEF>     1     1     2
// foo     <DEF>     1     1     1 -> SetToFollowing(2) -> foo     <DEF>     1     2     0
// foo     <DEF>     1     1   255 -> SetToFollowing(3) -> foo     <DEF>     1     2     0
// foo     <DEF>   255   255   255 -> SetToFollowing(3) -> foo\001 <DEF>     0     0     0
// foo     <DEF>   255     1     1 -> SetToFollowing(1) -> foo\001 <DEF>     0     0     0
// <UNDEF><UNDEF>  255     1     1 -> SetToFollowing(1) -> WORD_FOLLOWING_ATEND
// foo     <DEF>     1 <UNDEF> 255 -> SetToFollowing(3) -> foo     <DEF>     2 <UNDEF>   0
// foo     <DEF><UNDEF><UNDEF> 255 -> SetToFollowing(3) -> foo\001 <DEF><UNDEF><UNDEF>   0
//
//
int WordKey::SetToFollowing(int position /* = WORD_FOLLOWING_MAX */)
{
  if(position == WORD_FOLLOWING_MAX)
    position = NFields() - 1;
  
  if(position < 0 || position >= NFields()) {
    fprintf(stderr, "WordKey::SetToFollowing invalid position = %d\n", position);
    return NOTOK;
  }

  int i = position;
  while(i > 0) {
    if(IsDefined(i)) {
      if(Overflow(i, 1))
  Set(i, 0);
      else
  break;
    }
    i--;
  }

  if(i == 0) {
    if(IsDefined(i))
      GetWord() << '\001';
    else
      return WORD_FOLLOWING_ATEND;
  } else
    Get(i)++;

  for(i = position + 1; i < NFields(); i++)
    if(IsDefined(i)) Set(i,0);

  return OK;
}

//
// Return true if the key may be used as a prefix for search.
// In other words return true if the fields set in the key
// are all contiguous, starting from the first field in sort order.
//
int 
WordKey::Prefix() const
{
  const WordKeyInfo& info = *WordKey::Info();
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
  if(!IsDefinedWordSuffix()) { found_unset = 1; }
  //
  // Walk the fields in sorting order. 
  //
  for(int j = WORD_FIRSTFIELD; j < info.nfields; j++) 
  {
    //
    // Fields set, then fields unset then field set -> not a prefix
    //
    if(IsDefined(j)) 
    {
      if(found_unset) return NOTOK;
    }
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
// Unset all fields past the first unset field
// Return the number of fields in the prefix or 0 if
// first field is not set, ie no possible prefix.
//
int 
WordKey::PrefixOnly()
{
  const WordKeyInfo& info = *WordKey::Info();
  //
  // If all fields are set, the whole key is the prefix.
  //
  if(Filled()) return OK;
  //
  // If the first field is not set there is no possible prefix
  //
  if(!IsDefined(0)) 
  {
      return NOTOK;
  }
  
  int found_unset = 0;
  //
  // Walk the fields in sorting order. 
  //
  if(!IsDefinedWordSuffix()){found_unset=1;}

  for(int j = WORD_FIRSTFIELD; j < info.nfields; j++) 
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
  const WordKeyInfo& info = *WordKey::Info();
  if(length < info.num_length) {
    fprintf(stderr, "WordKey::Unpack: key record length < info.num_length\n");
    return NOTOK;
  }

  int string_length = length - info.num_length;
  SetWord(string, string_length);

  for(int j = WORD_FIRSTFIELD; j < info.nfields; j++) 
  {
      WordKeyNum value = 0; 
      int index = string_length + info.sort[j].bytes_offset;
      WordKey::UnpackNumber((unsigned char *)&string[index], 
          info.sort[j].bytesize, 
          value, 
          info.sort[j].lowbits, 
          info.sort[j].bits); 
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
  const WordKeyInfo& info = *WordKey::Info();

  char* string;
  int length = info.num_length;

  length += kword.length();

  if((string = (char*)malloc(length)) == 0) {
    fprintf(stderr, "WordKey::Pack: malloc returned 0\n");
    return NOTOK;
  }
  memset(string, '\0', length);

  memcpy(string, kword.get(), kword.length());
  for(int i = WORD_FIRSTFIELD; i < info.nfields; i++) {
    int index = kword.length() + info.sort[i].bytes_offset;
    WordKey::PackNumber(Get(i), 
      &string[index], 
      info.sort[i].bytesize, 
      info.sort[i].lowbits, 
      info.sort[i].lastbits); 
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
  const WordKeyInfo& info = *WordKey::Info();

  
  for(int j = 0; j < info.nfields; j++) {
    if(!IsDefined(j) && other.IsDefined(j)) {
      switch(info.sort[j].type) {
      case WORD_ISA_STRING: 
    SetWord(other.GetWord());
    if(!other.IsDefinedWordSuffix()) UndefinedWordSuffix();
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
  const WordKeyInfo& info = *WordKey::Info();

  //
  // Walk the fields in sorting order. As soon as one of them
  // does not compare equal, return.
  //
  for(int j = 0; j < info.nfields; j++) {
    if(!IsDefined(j)) {
      buffer << "<UNDEF>";
    } else {
      switch(info.sort[j].type) {
      case WORD_ISA_STRING:
  buffer << GetWord();
  break;
      case WORD_ISA_NUMBER:
  buffer << Get(j);
  break;
      default:
  fprintf(stderr, "WordKey::Get: invalid type %d for field %d\n", info.sort[j].type, j);
  return NOTOK;
      }
    }
    //
    // Output virtual word suffix field
    //
    if(j == 0) {
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

String
WordKey::Get() const
{
  String tmp;
  Get(tmp);
  return tmp;
}

//
// Set a key from an ascii representation
//
int
WordKey::Set(const String& buffer)
{
  StringList fields(buffer, "\t ");
  return SetList(fields);
}

//
// Set a key from list of fields
//
int
WordKey::SetList(StringList& fields)
{
  const WordKeyInfo& info = *WordKey::Info();
  int length = fields.Count();

  //
  // + 1 counts for the word suffix field
  //
  if(length < info.nfields + 1) {
    fprintf(stderr, "WordKey::Set: expected at least %d fields and found %d (ignored)\n", info.nfields + 1, length);
    return NOTOK;
  }
  if(length < 2) {
    fprintf(stderr, "WordKey::Set: expected at least two fields in line\n");
    return NOTOK;
  }

  Clear();

  fields.Start_Get();
  //
  // Handle word and its suffix
  //
  int i = 0;
  {
    //
    // Get the word
    //
    String* word = (String*)fields.Get_Next();
    if(word == 0) {
      fprintf(stderr, "WordKey::Set: failed to get word\n");
      return NOTOK;
    }
    if(word->nocase_compare("<undef>") == 0)
      UndefinedWord();
    else
      SetWord(*word);
    i++;

    //
    // Get the word suffix status
    //
    String* suffix = (String*)fields.Get_Next();
    if(suffix == 0) {
      fprintf(stderr, "WordKey::Set: failed to get word suffix %d\n", i);
      return NOTOK;
    }
    if(suffix->nocase_compare("<undef>") == 0)
      UndefinedWordSuffix();
    else
      SetDefinedWordSuffix();
  }

  //
  // Handle numerical fields
  //
  int j;
  for(j = WORD_FIRSTFIELD; i < info.nfields; i++, j++) {
    String* field = (String*)fields.Get_Next();

    if(field == 0) {
      fprintf(stderr, "WordKey::Set: failed to retrieve field %d\n", i);
      return NOTOK;
    }
    
    if(field->nocase_compare("<undef>") == 0) {
      Undefined(j);
    } else {
      WordKeyNum value = strtoul(field->get(), 0, 10);
      Set(j, value);
    }
  }

  return OK;
}

int WordKey::Write(FILE* f) const
{
  String tmp;
  Get(tmp);
  fprintf(f, "%s", (char*)tmp);
  return 0;
}

void WordKey::Print() const
{
  Write(stderr);
}

