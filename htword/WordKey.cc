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
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordKey.cc,v 1.3.2.19 2000/09/14 03:13:27 ghutchis Exp $
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <ctype.h>

#include "clib.h"
#include "WordKey.h"
#include "ber.h"
#include "HtMaxMin.h"

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
  const WordKeyInfo& info = context->GetKeyInfo();
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

    if(Get(j) != other.Get(j)) return 0;
  }
  return 1;
}

//
// Compare <a> and <b> in the Berkeley DB fashion. 
// <a> and <b> are packed keys.
//
int 
WordKey::Compare(WordContext* context, const unsigned char *a, int a_length, const unsigned char *b, int b_length)
{
  const WordKeyInfo& info = context->GetKeyInfo();
  int bytes;
  ber_t a_value;
  ber_t b_value;

  for(int j = 0; j < info.nfields; j++) {
    if((bytes = ber_buf2value(a, a_length, a_value)) < 1) {
      fprintf(stderr, "WordKey::Compare: failed to retrieve field %d for a\n", j);
      abort();
    }
    a += bytes;
    a_length -= bytes;
    if((bytes = ber_buf2value(b, b_length, b_value)) < 1) {
      fprintf(stderr, "WordKey::Compare: failed to retrieve field %d for b\n", j);
      abort();
    }
    b += bytes;
    b_length -= bytes;

    if(a_value != b_value)
      return a_value - b_value;
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
WordKey::Compare(WordContext* context, const String& a, const String& b)
{
  return WordKey::Compare(context, (const unsigned char*)a, a.length(), (const unsigned char*)b, b.length());
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
  WordContext* context = (WordContext*)a->app_private;
  return WordKey::Compare(context, (const unsigned char*)a->data, a->size, (const unsigned char*)b->data, b->size);
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

  int nfields=WordKey::NFields();

  int i;
  for(i = 0; i < nfields; i++) {
    if(IsDefined(i) && other.IsDefined(i) &&
       Get(i) != other.Get(i)) {
      lower = Get(i) < other.Get(i);
      break;
    }
  }
  if(i < nfields)
    position = i;

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
  while(i >= 0) {
    if(IsDefined(i)) {
      if(Overflow(i, 1))
	Set(i, 0);
      else
	break;
    }
    i--;
  }

  if(i < 0) {
    fprintf(stderr, "WordKey::SetToFollowing cannot increment\n");
    return NOTOK;
  }
  
  Get(i)++;

  for(i = position + 1; i < NFields(); i++)
    if(IsDefined(i)) Set(i,0);

  return OK;
}

int 
WordKey::Prefix() const
{
  const WordKeyInfo& info = context->GetKeyInfo();
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

  for(int j = 0; j < info.nfields; j++) {
    //
    // Fields set, then fields unset then field set -> not a prefix
    //
    if(IsDefined(j)) {
      if(found_unset) return NOTOK;
    } else {
      //
      // Found unset fields and this is fine as long as we do
      // not find a field set later on.
      //
      found_unset++;
    }
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
  const WordKeyInfo& info = context->GetKeyInfo();
  //
  // If all fields are set, the whole key is the prefix.
  //
  if(Filled()) return OK;
  //
  // If the first field is not set there is no possible prefix
  //
  if(!IsDefined(0))
      return NOTOK;
  
  int found_unset = 0;
  //
  // Walk the fields in sorting order. 
  //

  for(int j = 0; j < info.nfields; j++) {
    //
    // Unset all fields after the first unset field
    //
    if(IsDefined(j)) {
      if(found_unset) {
	Set(j,0);
	Undefined(j);
      }
    } else {
      found_unset = 1;
    }
  }

  return OK;
}

//
// Unpack from data and fill fields of object
// 
int 
WordKey::Unpack(const char* string, int length)
{
  const WordKeyInfo& info = context->GetKeyInfo();

  const unsigned char* p = (const unsigned char*)string;
  int p_length = length;
  ber_t value;

  for(int j = 0; j < info.nfields; j++) {
    int bytes = ber_buf2value(p, p_length, value);
    if(bytes < 1) {
      fprintf(stderr, "WordKey::Unpack: ber_buf2value failed at %d\n", j);
      return NOTOK;
    }
    p_length -= bytes;
    if(p_length < 0) {
      fprintf(stderr, "WordKey::Unpack: ber_buf2value overflow at %d\n", j);
      return NOTOK;
    }
    p += bytes;
    Set(j, value);
  }

  return OK;
}

//
// Pack object into the <packed> string
//
int 
WordKey::Pack(String& packed) const
{
  const WordKeyInfo& info = context->GetKeyInfo();

  unsigned char* string;
  // 
  // + 1 : storage for the string length 
  //
  int length = BER_MAX_BYTES * info.nfields;

  if((string = (unsigned char*)malloc(length)) == 0) {
    fprintf(stderr, "WordKey::Pack: malloc returned 0\n");
    return NOTOK;
  }

  unsigned char* p = string;
  int p_length = length;

  for(int i = 0; i < info.nfields; i++) {
    int bytes = ber_value2buf(p, p_length, Get(i));
    if(bytes < 1) {
      fprintf(stderr, "WordKey::Pack: ber_value2buf failed at %d\n", i);
      return NOTOK;
    }
    p_length -= bytes;
    if(p_length < 0) {
      fprintf(stderr, "WordKey::Pack: ber_value2buf overflow at %d\n", i);
      return NOTOK;
    }
    p += bytes;
  }

  packed.set((const char*)string, p - string);

  free(string);

  return OK;
}

//
// Copy all fields set in <other> to object, only if 
// the field is not already set in <other>
//
int WordKey::Merge(const WordKey& other)
{
  const WordKeyInfo& info = context->GetKeyInfo();
  
  for(int j = 0; j < info.nfields; j++) {
    if(!IsDefined(j) && other.IsDefined(j)) {
      Set(j,other.Get(j)); 
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
  const WordKeyInfo& info = context->GetKeyInfo();

  //
  // Walk the fields in sorting order. As soon as one of them
  // does not compare equal, return.
  //
  for(int j = 0; j < info.nfields; j++) {
    if(!IsDefined(j)) {
      buffer << "<UNDEF>";
    } else {
      buffer << Get(j);
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
  const WordKeyInfo& info = context->GetKeyInfo();
  int length = fields.Count();

  if(length < info.nfields) {
    fprintf(stderr, "WordKey::SetList: expected at least %d fields and found %d (ignored)\n", info.nfields, length);
    return NOTOK;
  }
  if(length < 1) {
    fprintf(stderr, "WordKey::SetList: expected at least one field in line\n");
    return NOTOK;
  }

  Clear();

  //
  // Handle numerical fields
  //
  int i;
  for(i = 0; i < info.nfields; i++) {
    String* field = (String*)fields.Get_First();

    if(field == 0) {
      fprintf(stderr, "WordKey::Set: failed to retrieve field %d\n", i);
      return NOTOK;
    }
    
    if(field->nocase_compare("<undef>") == 0) {
      Undefined(i);
    } else {
      WordKeyNum value = strtoul(field->get(), 0, 10);
      Set(i, value);
    }
    fields.Remove(0);
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

