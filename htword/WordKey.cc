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
// $Id: WordKey.cc,v 1.3 1999/10/05 16:03:31 loic Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <iostream.h>

#include "WordKey.h"


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
int WordKey::Equal(const WordKey& other, int prefix_length) const
{
  const struct WordKeyInfo& info = word_key_info;
  //
  // Walk the fields in sorting order. As soon as one of them
  // does not compare equal, return.
  //
  for(int j = 0; j < info.nfields; j++) {
    int i = info.sort[j].field_number;
    //
    // Only compare fields that are set in both key
    //
    if(!IsSet(i) || !other.IsSet(i)) continue;

    int k = info.fields[i].index;

    switch(info.fields[i].type) {
    case WORD_ISA_String:
      if(prefix_length > 0) {
	if(pool_String[k] != other.pool_String[k].sub(0, prefix_length))
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

#ifdef WORD_HAVE_TypeA
STATEMENT(TypeA)
#endif /* WORD_HAVE_TypeA */
#ifdef WORD_HAVE_TypeB
STATEMENT(TypeB)
#endif /* WORD_HAVE_TypeB */
#ifdef WORD_HAVE_TypeC
STATEMENT(TypeC)
#endif /* WORD_HAVE_TypeC */

#undef STATEMENT
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
  for(int j = 0; j < info.nfields; j++) {
    int i = info.sort[j].field_number;
    //
    // To sort in inverted order, just swap parameters.
    //
    if(info.sort[j].direction == WORD_SORT_DESCENDING) {
      const char* tmp = a;
      int tmp_length = a_length;
      a = b; a_length = b_length; b = tmp; b_length = tmp_length;
    }
    switch(info.fields[i].type) {
    case WORD_ISA_String:
      {
	const char* p1 = a + info.fields[i].bytes_offset;
	int p1_length = a_length - info.fields[i].bytes_offset;
	const char* p2 = b + info.fields[i].bytes_offset;
	int p2_length = b_length - info.fields[i].bytes_offset;
	int len = p1_length > p2_length ? p2_length : p1_length;
	for (p1 = a + info.fields[i].bytes_offset, p2 = b + info.fields[i].bytes_offset; len--; ++p1, ++p2)
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
	WordKey::UnpackNumber(&a[info.fields[i].bytes_offset],
			      info.fields[i].bytesize,
			      &p1,
			      info.fields[i].lowbits,
			      info.fields[i].bits);
	
	WordKey::UnpackNumber(&b[info.fields[i].bytes_offset],
			      info.fields[i].bytesize,
			      &p2,
			      info.fields[i].lowbits,
			      info.fields[i].bits);
	if(p1 != p2)
	  return p1 - p2;
      }
      break;
    default:
      cerr << "WordKey::Compare: invalid type " << info.fields[i].type << " for field " << i << "\n";
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
  if(!IsSet(info.sort[0].field_number)) return NOTOK;
  
  int found_unset = 0;
  //
  // Walk the fields in sorting order. 
  //
  for(int j = 1; j < info.nfields; j++) {
    int i = info.sort[j].field_number;

    //
    // Fields set, then fields unset then field set -> not a prefix
    //
    if(IsSet(i))
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
  if(Filled()) return info.nfields;
  //
  // If the first field is not set there is no possible prefix
  //
  if(!IsSet(info.sort[0].field_number)) return 0;
  
  int field_count = 0;
  int found_unset = 0;
  //
  // Walk the fields in sorting order. 
  //
  for(int j = 0; j < info.nfields; j++) {
    int i = info.sort[j].field_number;

    //
    // Unset all fields after the first unset field
    //
    if(IsSet(i)) {
      if(found_unset)
	Unset(i);
      else
	field_count++;
    } else
      found_unset++;
  }

  return field_count;
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
      Set(i);
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
	Set(i); \
      } \
      break;

#ifdef WORD_HAVE_TypeA
STATEMENT(TypeA)
#endif /* WORD_HAVE_TypeA */
#ifdef WORD_HAVE_TypeB
STATEMENT(TypeB)
#endif /* WORD_HAVE_TypeB */
#ifdef WORD_HAVE_TypeC
STATEMENT(TypeC)
#endif /* WORD_HAVE_TypeC */

#undef STATEMENT
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

#ifdef WORD_HAVE_TypeA
STATEMENT(TypeA)
#endif /* WORD_HAVE_TypeA */
#ifdef WORD_HAVE_TypeB
STATEMENT(TypeB)
#endif /* WORD_HAVE_TypeB */
#ifdef WORD_HAVE_TypeC
STATEMENT(TypeC)
#endif /* WORD_HAVE_TypeC */

#undef STATEMENT
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
    if(!IsSet(i) && other.IsSet(i)) {
      switch(info.fields[i].type) {

#define STATEMENT(type) \
      case WORD_ISA_##type: \
	Set##type(other.Get##type(i), i); \
	break;

#ifdef WORD_HAVE_TypeA
STATEMENT(TypeA)
#endif /* WORD_HAVE_TypeA */
#ifdef WORD_HAVE_TypeB
STATEMENT(TypeB)
#endif /* WORD_HAVE_TypeB */
#ifdef WORD_HAVE_TypeC
STATEMENT(TypeC)
#endif /* WORD_HAVE_TypeC */

#undef STATEMENT

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
  for(int j = 0; j < info.nfields; j++) {
    int i = info.sort[j].field_number;
    int index = info.fields[i].index;

    switch(info.fields[i].type) {
    case WORD_ISA_String:
      o << key.pool_String[index] << "\t";
      break;
#define STATEMENT(type) \
    case WORD_ISA_##type: \
      o << key.pool_##type[index] << "\t"; \
      break;

#ifdef WORD_HAVE_TypeA
STATEMENT(TypeA)
#endif /* WORD_HAVE_TypeA */
#ifdef WORD_HAVE_TypeB
STATEMENT(TypeB)
#endif /* WORD_HAVE_TypeB */
#ifdef WORD_HAVE_TypeC
STATEMENT(TypeC)
#endif /* WORD_HAVE_TypeC */

#undef STATEMENT
    default:
      cerr << "WordKey::operator <<: invalid type " << info.fields[i].type << " for field " << i << "\n";
      break;
    }
  }

  return o;
}

