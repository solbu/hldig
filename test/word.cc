//
// word.cc
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: word.cc,v 1.2 1999/09/24 14:30:13 loic Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <unistd.h>
#include <stdlib.h>
#include <iostream.h>
#include <stdio.h>

// If we have this, we probably want it.
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include "defaults.h"
#include "WordKey.h"
#include "WordList.h"
#include "HtWordType.h"

typedef struct {
  char* word_desc;
  char* config;
  int key;
  int list;
} params_t;

static void usage();
static void doword(params_t* params);
static void dolist(params_t* params);
static void dokey(params_t* params);
static void pack_show(const WordReference& wordRef);

static int verbose = 0;

//*****************************************************************************
// int main(int ac, char **av)
//
int main(int ac, char **av)
{
  int			c;
  extern char		*optarg;
  params_t		params;

  params.word_desc = strdup("???");
  params.config = strdup("???");
  params.key = 0;
  params.list = 0;

  while ((c = getopt(ac, av, "vklw:c:")) != -1)
    {
      switch (c)
	{
	case 'v':
	  verbose++;
	  break;
	case 'w':
	  free(params.word_desc);
	  params.word_desc = strdup(optarg);
	  break;
	case 'c':
	  free(params.config);
	  params.config = strdup(optarg);
	  break;
	case 'k':
	  params.key = 1;
	  break;
	case 'l':
	  params.list = 1;
	  break;
	case '?':
	  usage();
	  break;
	}
    }

  doword(&params);

  free(params.word_desc);
  free(params.config);

  return 0;
}

static void doword(params_t* params)
{
  if(params->key) {
    cerr << "Test WordKey class\n";
    dokey(params);
  }
  if(params->list) {
    cerr << "Test WordList class\n";
    config.Defaults(&defaults[0]);
    config.Read(params->config);
    // Ctype-like functions for what constitutes a word.
    HtWordType::Initialize(config);
    unlink(config["word_db"]);
    dolist(params);
  }
}

static void dolist(params_t* params)
{
  static char* word_list[] = {
    "The",
    "quick",
    "brown",
    "fox",
    "jumps",
    "over",
    "the",
    "lazy",
    "dog",
    0
  };

  //
  // Most simple case. Insert a few words and
  // search them, using exact match.
  //
  {
    WordList words(config);
    words.Open(config["word_db"], O_RDWR);

    WordReference wordRef;
    wordRef.DocID(3);
    wordRef.Flags(FLAG_TEXT);
    unsigned int location = 0;
    unsigned int anchor = 0;
    if(verbose) fprintf(stderr, "Inserting\n");

    for(char** p = word_list; *p; p++) {
      wordRef.Word(*p);
      wordRef.Location(location);
      wordRef.Anchor(anchor);
      if(verbose > 1) pack_show(wordRef);
      words.Replace(wordRef);
      location += strlen(*p);
      anchor++;
    }
    words.Flush();
    words.Close();

    wordRef.Location(location = 0);
    wordRef.Anchor(anchor = 0);

    if(verbose) fprintf(stderr, "Searching\n");

    words.Open(config["word_db"], O_RDONLY);
    for(char** p = word_list; *p; p++) {
      wordRef.Word(*p);
      wordRef.Location(location);
      wordRef.Anchor(anchor);

      if(verbose) fprintf(stderr, "%s ... ", *p);
      if(verbose > 1) pack_show(wordRef);
      List *result = words[wordRef];
      result->Start_Get();
      int count = 0;
      WordReference* found;
      while((found = (WordReference*)result->Get_Next())) {
	if(wordRef.Word() != found->Word()) {
	  fprintf(stderr, "dolist: simple: expected %s, got %s\n", (const char*)wordRef.Word(), (const char*)found->Word());
	}
	count++;
      }
      if(count != 1) {
	fprintf(stderr, "dolist: simple: searching %s, got %d matches instead of 1\n", (const char*)wordRef.Word(), count);
      }
      if(verbose) fprintf(stderr, "done\n");

      delete result;

      location += strlen(*p);
      anchor++;
    }
  }
  //
  // Search all occurences of 'the'
  //
  {
    WordList words(config);
    words.Open(config["word_db"], O_RDWR);

    WordReference wordRef;
    wordRef.Word("the");

    List *result = words[wordRef];
    result->Start_Get();
    int count = 0;
    WordReference* found;
    while((found = (WordReference*)result->Get_Next())) {
	if(wordRef.Word() != found->Word()) {
	  fprintf(stderr, "dolist: simple: expected %s, got %s\n", (const char*)wordRef.Word(), (const char*)found->Word());
	}
	if(verbose) found->Dump(stderr);
	count++;
    }
    if(count != 2) {
      fprintf(stderr, "dolist: searching occurences of '%s', got %d matches instead of 2\n", (const char*)wordRef.Word(), count);
    }

    delete result;
  }
}

#define WORD_BIT_MASK(b) ((b) == 32 ? 0xffffffff : (( 1 << (b)) - 1))

//
// See WordKey.h
// Tested: Pack, Unpack, Compare (both forms), accessors, meta information
//
static void dokey(params_t* params)
{
  WordKey word;
  const struct WordKeyInfo& info = word_key_info;

  //
  // Feed the structure with a pattern
  //
  for(int i = 0; i < info.nfields; i++) {
    if(verbose) fprintf(stderr, "%s\t=\t", info.fields[i].name);
    switch(info.fields[i].type) {
    case WORD_ISA_pool_String:
      {
	word.Set("Test string", info.fields[i].index);
	String tmp;
	if(verbose) fprintf(stderr, "%s", word.Get(tmp, info.fields[i].index).get());
      }
      break;
    case WORD_ISA_pool_unsigned_int:
    case WORD_ISA_pool_unsigned_short:
    case WORD_ISA_pool_unsigned_char:
      {
	word.Set(0x12579ade & WORD_BIT_MASK(info.fields[i].bits), info.fields[i].index);
	if(verbose) fprintf(stderr, "0x%0x", word.Get(0, info.fields[i].index));
      }
      break;
    }
    if(verbose) fprintf(stderr, "\n");
  }

  //
  // Pack the word
  //
  String packed;

  word.Pack(packed);

  if(verbose) {
    for(int i = 0; i < packed.length(); i++) {
      fprintf(stderr, "0x%02x(%c) ", packed[i] & 0xff, packed[i]);
    }
    fprintf(stderr, "\n");
  }
  
  //
  // Unpack in another object
  //
  WordKey other_word;
  other_word.Unpack(packed);

  for(int i = 0; i < info.nfields; i++) {
    if(verbose) fprintf(stderr, "%s\t=\t", info.fields[i].name);
    switch(info.fields[i].type) {
    case WORD_ISA_pool_String:
      {
	String tmp;
	if(verbose) fprintf(stderr, "%s", other_word.Get(tmp, info.fields[i].index).get());
      }
      break;
    case WORD_ISA_pool_unsigned_int:
    case WORD_ISA_pool_unsigned_short:
    case WORD_ISA_pool_unsigned_char:
      {
	if(verbose) fprintf(stderr, "0x%0x", other_word.Get(0, info.fields[i].index));
      }
      break;
    }
    if(verbose) fprintf(stderr, "\n");
  }

  //
  // The two (word and other_word) must compare equal, using
  // comparison of the packed strings.
  //
  if(!word.PackEqual(other_word)) {
    fprintf(stderr, "dokey: %s not equal (object compare)\n", params->word_desc);
  }

  //
  // Pack the other_word
  //
  String other_packed;

  other_word.Pack(other_packed);

  
  //
  // The two (word and other_word) must compare equal
  // using the alternate comparison (fast) interface.
  //
  if(WordKey::Compare(packed, other_packed) != 0) {
    fprintf(stderr, "dokey: %s not equal (fast compare)\n", params->word_desc);
  }

  //
  // Add one char to the word, they must not compare equal and
  // the difference must be minus one.
  //
  {
    String tmp = other_word.Get(String(""), info.nfields - 1);
    tmp.append("a");
    other_word.Set(tmp, info.nfields - 1);
  }
  other_word.Pack(other_packed);
  {
    int ret;
    if((ret = WordKey::Compare(packed, other_packed)) != -1) {
      fprintf(stderr, "dokey: %s different length, expected -1 got %d\n", params->word_desc, ret);
    }
  }
  {
    String tmp("Test string");
    other_word.Set(tmp, info.nfields - 1);
  }

  //
  // Change T to S
  // the difference must be one.
  //
  {
    String tmp = other_word.Get(String(""), info.nfields - 1);
    tmp[tmp.indexOf('T')] = 'S';
    other_word.Set(tmp, info.nfields - 1);
  }
  other_word.Pack(other_packed);
  {
    int ret;
    if((ret = WordKey::Compare(packed, other_packed)) != 1) {
      fprintf(stderr, "dokey: %s different letter (S instead of T), expected 1 got %d\n", params->word_desc, ret);
    }
  }
  {
    String tmp("Test string");
    other_word.Set(tmp, info.nfields - 1);
  }
  
  //
  // Substract one to the first numeric field
  // The difference must be one.
  //
  other_word.Set(other_word.Get((unsigned int)0, 0) - 1, 0);
  other_word.Pack(other_packed);
  {
    int ret;
    if((ret = WordKey::Compare(packed, other_packed)) != 1) {
      fprintf(stderr, "dokey: %s different numeric field, expected 1 got %d\n", params->word_desc, ret);
    }
  }
}

static void pack_show(const WordReference& wordRef)
{
  String key;
  String record;

  wordRef.Pack(key, record);

  fprintf(stderr, "key = ");
  for(int i = 0; i < key.length(); i++) {
    fprintf(stderr, "0x%02x(%c) ", key[i] & 0xff, key[i]);
  }
  fprintf(stderr, " record = ");
  for(int i = 0; i < record.length(); i++) {
    fprintf(stderr, "0x%02x(%c) ", record[i] & 0xff, record[i]);
  }
  fprintf(stderr, "\n");
}

//*****************************************************************************
// void usage()
//   Display program usage information
//
static void usage()
{
    cout << "usage: word [options]\n";
    cout << "Options:\n";
    cout << "\t-v\t\tIncreases the verbosity\n";
    cout << "\t-w file\tname of the word description file used to generate sources\n";
    cout << "\t-c file\tspecify the config file to load\n";
    cout << "\t-k\t\tTest WordKey\n";
    cout << "\t-l\t\tTest WordList\n";
    exit(0);
}

