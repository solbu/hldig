// WordKeyInfo.cc
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
//
#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <errno.h>

#include "WordKeyInfo.h"
#include "StringList.h"

#define WORDKEYFIELD_BITS_MAX 64

//
// WordKeyField implementation
//
int WordKeyField::SetNum(WordKeyField *previous, char *nname, int nbits)
{
  type = WORD_ISA_NUMBER;
  name.set(nname, strlen(nname));

  bits = nbits;
  bits_offset = (previous ?  previous->bits_offset + previous->bits  :  0 );

  if(bits_offset < 0 ||
     bits_offset > WORDKEYFIELD_BITS_MAX*WORD_KEY_MAX_NFIELDS) {
    fprintf(stderr, "WordKeyField::WordKeyField: bits_offset: %d out of bounds\n", bits_offset);
    return EINVAL;
  }
  bytes_offset = bits_offset / 8;
  bytesize = ((bits_offset + bits - 1) / 8) - bytes_offset + 1;
  lastbits = (bits_offset + bits) % 8;
  lowbits = bits_offset % 8;

  return 0;
}

int WordKeyField::SetString()
{
  name.set("Word");
  type = WORD_ISA_STRING;
  return 0;
}

//
// Tabulate for printing
//
static void nprint(char c, int n)
{
  for(int i = 0; i < n; i++) {
    if(!(i % 4)) {
      printf("%c", 'a' + i / 4);
    } else {
      printf("%c", c);
    }
  }
}

//
// Print object on standard output
//
void 
WordKeyField::Show()
{
  if(!name.nocase_compare("Word")) {
    printf("Word type: %2d\n", type);
  } else {
    nprint(' ',bits_offset);
    printf("\"%s\" type:%2d lowbits:%2d lastbits:%2d\n",
	   (char *)name,
	   type,
	   lowbits,
	   lastbits);
    nprint(' ',bits_offset);
    printf("|---bytesize:%2d bytes_offset:%2d bits:%2d bits_offset:%2d\n", bytesize, bytes_offset, bits, bits_offset);
  }
}

//
// WordKeyInfo implementation
//

WordKeyInfo* WordKeyInfo::instance = 0;

WordKeyInfo::WordKeyInfo(const Configuration& config)
{
  sort = NULL;
  nfields = -1;
  num_length = 0;

  const String &keydesc = config["wordlist_wordkey_description"];

  if(!keydesc.empty()) {
    Set(keydesc);
  } else {
    fprintf(stderr, "WordKeyInfo::WordKeyInfo: didn't find key description in config\n");
  }
}

void 
WordKeyInfo::Initialize(const Configuration &config_arg)
{
  if(instance != 0)
    delete instance;
  instance = new WordKeyInfo(config_arg);
}

void 
WordKeyInfo::InitializeFromString(const String &desc)
{
  Configuration config;
  config.Add("wordlist_wordkey_description", desc);
  Initialize(config);
}

int
WordKeyInfo::Alloc(int nnfields)
{
  nfields = nnfields;
  if(!(sort = new WordKeyField[nfields])) {
    fprintf(stderr, "WordKeyInfo::Alloc: cannot allocate\n");
    return ENOMEM;
  }
  num_length = 0;
  return 0;
}

int
WordKeyInfo::Set(const String &desc)
{
  int ret = 0;
  StringList fields(desc, "/");

  if(fields.Count() > WORD_KEY_MAX_NFIELDS) {
    fprintf(stderr, "WordKeyInfo::Set: too many fields in %s, max is %d\n", (const char*)desc, WORD_KEY_MAX_NFIELDS);
    return EINVAL;
  }

  if(fields.Count() <= 0) {
    fprintf(stderr, "WordKeyInfo::Set: no fields\n");
    return EINVAL;
  }

  if((ret = Alloc(fields.Count())))
    return ret;

  WordKeyField* previous = 0;
  int i;
  for(i = 0; i < fields.Count(); i++) {
    char* field = fields[i];
    WordKeyField& key_field = sort[i];
    if(!mystrcasecmp(field, "word")) {
      //
      // String field
      //
      if(i != 0) {
	fprintf(stderr, "WordKeyInfo::Set: Word field must show in first position %s\n", (const char*)desc);
	return EINVAL;
      }
      key_field.SetString();
    } else {
      //
      // Numerical field
      //
      StringList pair(field, "\t ");
	
      if(pair.Count() != 2) {
	fprintf(stderr, "WordKeyInfo::AddField: there must be exactly two strings separated by a white space (space or tab) in a field description (%s in key description %s)\n", field, (const char*)desc);
	return EINVAL;
      }

      int bits = atoi(pair[1]);
      char* name = pair[0];
      key_field.SetNum(previous, name, bits);
      previous = &key_field;
    }
  }

  //
  // Total length in bytes of the numerical fields
  //
  num_length = sort[i - 1].bytes_offset + sort[i - 1].bytesize;
  
  return ret;
}

void 
WordKeyInfo::Show()
{
    fprintf(stderr, "-----------------------------------------\n");
    fprintf(stderr, "nfields:%3d num_length:%3d\n", nfields, num_length);
    int i;
    for(i = 0; i < nfields; i++)
	sort[i].Show();

    char str[WORDKEYFIELD_BITS_MAX*WORD_KEY_MAX_NFIELDS];
    memset(str, '_', WORDKEYFIELD_BITS_MAX*WORD_KEY_MAX_NFIELDS);

    int last = 0;
    int j;
    for(j = 0; j < nfields; j++) {
	for(i = 0; i < sort[j].bits; i++) {
	    char c = (j % 10) + '0';
	    int pos = sort[j].bits_offset + i;
	    if(str[pos] != '_') {
	      fprintf(stderr, "WordKeyInfo::Show: overlaping bits (field %d), bit %d\n", j, i);
	      c='X';
	    }
	    str[pos] = c;
	    if(last < pos) last = pos;
	}
    }
    str[last + 1] = '\0';
    fprintf(stderr, "%s (bits)\n",str);
    fprintf(stderr, "^0      ^1      ^2      ^3      ^4      ^5      ^6      ^7\n");
    fprintf(stderr, "0123456701234567012345670123456701234567012345670123456701234567\n");
}
