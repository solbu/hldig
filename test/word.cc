//
// word.cc
//
// word: Implement tests for the word database related classes.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: word.cc,v 1.14 1999/10/08 12:59:58 loic Exp $
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
#include "WordType.h"

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
    if(verbose) cerr << "Test WordKey class with " << params->word_desc << "\n";
    dokey(params);
  }
  if(params->list) {
    if(verbose) cerr << "Test WordList class\n";
    config.Defaults(&defaults[0]);
    config.Read(params->config);
    // Ctype-like functions for what constitutes a word.
    WordType::Initialize(config);
    unlink(config["word_db"]);
    dolist(params);
  }
}

static void dolist(params_t*)
{
  static char* word_list[] = {
    "The",	// DocID = 1
    "quick",	// DocID = 2
    "brown",	// DocID = 3
    "fox",	// DocID = 4
    "jumps",	// DocID = 5
    "over",	// DocID = 6
    "the",	// DocID = 7
    "lazy",	// DocID = 8
    "dog",	// DocID = 9
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
    wordRef.Key().SetFlags(67);
    unsigned int location = 0;
    unsigned int anchor = 0;
    unsigned int docid = 1;
    if(verbose) fprintf(stderr, "Inserting\n");

    for(char** p = word_list; *p; p++) {
      wordRef.Key().SetWord(*p);
      wordRef.Key().SetDocID(docid);
      wordRef.Key().SetLocation(location);
      wordRef.Record().info.data = anchor;
      if(verbose > 2) pack_show(wordRef);
      if(verbose > 1) cerr << wordRef << "\n";
      words.Insert(wordRef);
      location += strlen(*p);
      anchor++;
      docid++;
    }
    words.Close();

    location = anchor = 0;
    docid = 1;

    if(verbose) fprintf(stderr, "Searching\n");

    words.Open(config["word_db"], O_RDONLY);
    for(char** p = word_list; *p; p++) {
      wordRef.Key().SetWord(*p);
      wordRef.Key().SetLocation(location);
      wordRef.Record().info.data = anchor;
      wordRef.Key().SetDocID(docid);

      location += strlen(*p);
      anchor++;
      docid++;

      //
      // Skip first word because we don't want to deal with upper/lower case at present.
      //
      if(p == word_list) continue;

      if(verbose) fprintf(stderr, "%s ... ", *p);
      if(verbose > 2) pack_show(wordRef);
      if(verbose > 1) cerr << wordRef << "\n";
      List *result = words[wordRef];
      result->Start_Get();
      int count = 0;
      WordReference* found;
      while((found = (WordReference*)result->Get_Next())) {
	if(wordRef.Key().GetWord() != found->Key().GetWord()) {
	  fprintf(stderr, "dolist: simple: expected %s, got %s\n", (char*)wordRef.Key().GetWord(), (char*)found->Key().GetWord());
	  exit(1);
	}
	count++;
      }
      if(count != 1) {
	fprintf(stderr, "dolist: simple: searching %s, got %d matches instead of 1\n", (char*)wordRef.Key().GetWord(), count);
	exit(1);
      }
      if(verbose) fprintf(stderr, "done\n");

      delete result;

    }
  }
  //
  // Print all records as sorted within Berkeley DB with number
  // of occurences.
  //
  if(verbose) {
    WordList words(config);
    words.Open(config["word_db"], O_RDWR);

    List *result = words.Words();
    if(result == 0) {
      fprintf(stderr, "dolist: getting all words failed\n");
      exit(1);
    }
    result->Start_Get();
    int count = 0;
    String* found;
    while((found = (String*)result->Get_Next())) {
      unsigned int noccurence;
      WordKey key;
      key.SetWord(*found);
      words.Noccurence(key, noccurence);
      cerr << *found << " (" << noccurence << ")\n";
      count++;
    }
    if(count != 8) {
      fprintf(stderr, "dolist: getting all words, got %d matches instead of 8\n", count);
      exit(1);
    }

    delete result;
  }
  //
  // Search all occurences of 'the'
  //
  {
    WordList words(config);
    words.Open(config["word_db"], O_RDWR);

    WordReference wordRef;
    wordRef.Key().SetWord("the");

    unsigned int noccurence;
    if(words.Noccurence(wordRef.Key(), noccurence) != OK) {
      fprintf(stderr, "dolist: get ref count of 'the' failed\n");
      exit(1);
    } else if(noccurence != 2) {
      fprintf(stderr, "dolist: get ref count of 'the' failed, got %d instead of 2\n", noccurence);
      exit(1);
    }
    List *result = words[wordRef];
    result->Start_Get();
    int count = 0;
    WordReference* found;
    while((found = (WordReference*)result->Get_Next())) {
	if(wordRef.Key().GetWord() != found->Key().GetWord()) {
	  fprintf(stderr, "dolist: simple: expected %s, got %s\n", (char*)wordRef.Key().GetWord(), (char*)found->Key().GetWord());
	  exit(1);
	}
	if(verbose) cerr << *found << "\n";
	count++;
    }
    if(count != 2) {
      fprintf(stderr, "dolist: searching occurences of '%s', got %d matches instead of 2\n", (char*)wordRef.Key().GetWord(), count);
      exit(1);
    }

    delete result;
  }
  //
  // Delete all occurences of 'the'
  //
  {
    WordList words(config);
    words.Open(config["word_db"], O_RDWR);

    WordReference wordRef("the");

    int count;
    if((count = words.WalkDelete(wordRef)) != 2) {
      fprintf(stderr, "dolist: delete occurences of 'the', got %d deletion instead of 2\n", count);
      exit(1);
    }

    List* result = words[wordRef];
    if(result->Count() != 0) {
      fprintf(stderr, "dolist: unexpectedly found 'the' \n");
      exit(1);
    }
    delete result;

    unsigned int noccurence;
    if(words.Noccurence(wordRef.Key(), noccurence) != OK) {
      fprintf(stderr, "dolist: get ref count of 'thy' failed\n");
      exit(1);
    } else if(noccurence != 0) {
      fprintf(stderr, "dolist: get ref count of 'thy' failed, got %d instead of 0\n", noccurence);
      exit(1);
    }
  }
  //
  // Delete all words in document 5 (only one word : jumps)
  //
  {
    WordList words(config);
    words.Open(config["word_db"], O_RDWR);

    WordReference wordRef;
    wordRef.Key().SetDocID(5);
    int count;
    if((count = words.WalkDelete(wordRef)) != 1) {
      fprintf(stderr, "dolist: delete occurences in DocID 5, %d deletion instead of 1\n", count);
      exit(1);
    }

    wordRef.Clear();
    wordRef.Key().SetWord("jumps");
    List* result = words[wordRef];
    if(result->Count() != 0) {
      fprintf(stderr, "dolist: unexpectedly found 'jumps' \n");
      exit(1);
    }
    delete result;

    unsigned int noccurence;
    if(words.Noccurence(wordRef.Key(), noccurence) != OK) {
      fprintf(stderr, "dolist: get ref count of 'jumps' failed\n");
      exit(1);
    } else if(noccurence != 0) {
      fprintf(stderr, "dolist: get ref count of 'jumps' failed, got %d instead of 0\n", noccurence);
      exit(1);
    }
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
    if(verbose > 1) fprintf(stderr, "%s\t=\t", info.fields[i].name);
    switch(info.fields[i].type) {
    case WORD_ISA_String:
      {
	word.SetWord("Test string");
	if(verbose > 1) fprintf(stderr, "%s", word.GetWord().get());
      }
      break;

#define STATEMENT(type) \
    case WORD_ISA_##type: \
      { \
	word.Set##type(0x12579ade & WORD_BIT_MASK(info.fields[i].bits), info.fields[i].index); \
	if(verbose > 1) fprintf(stderr, "0x%0x", word.Get##type(info.fields[i].index)); \
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
    if(verbose > 1) fprintf(stderr, "\n");
  }

  //
  // Pack the word
  //
  String packed;

  word.Pack(packed);

  if(verbose > 1) {
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

  if(verbose > 1) {
    for(int i = 0; i < info.nfields; i++) {
      fprintf(stderr, "%s\t=\t", info.fields[i].name);
      switch(info.fields[i].type) {
      case WORD_ISA_String:
	fprintf(stderr, "%s", (char*)other_word.GetWord());
	break;
#define STATEMENT(type) \
      case WORD_ISA_##type: \
        { \
	  fprintf(stderr, "0x%0x", other_word.Get##type(info.fields[i].index)); \
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
      fprintf(stderr, "\n");
    }
  }

  //
  // The two (word and other_word) must compare equal, using
  // comparison of the packed strings.
  //
  if(!word.PackEqual(other_word)) {
    fprintf(stderr, "dokey: %s not equal (object compare)\n", params->word_desc);
    exit(1);
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
    exit(1);
  }

  //
  // Add one char to the word, they must not compare equal and
  // the difference must be minus one.
  //
  other_word.GetWord().append("a");
  other_word.Pack(other_packed);
  {
    int ret;
    if((ret = WordKey::Compare(packed, other_packed)) != -1) {
      fprintf(stderr, "dokey: %s different length, expected -1 got %d\n", params->word_desc, ret);
      exit(1);
    }
  }
  other_word.GetWord().set("Test string");

  //
  // Change T to S
  // the difference must be one.
  //
  {
    String& tmp = other_word.GetWord();
    tmp[tmp.indexOf('T')] = 'S';
  }
  other_word.Pack(other_packed);
  {
    int ret;
    if((ret = WordKey::Compare(packed, other_packed)) != 1) {
      fprintf(stderr, "dokey: %s different letter (S instead of T), expected 1 got %d\n", params->word_desc, ret);
      exit(1);
    }
  }
  other_word.GetWord().set("Test string");
  
  //
  // Substract one to the first numeric field
  // The difference must be one.
  //
  other_word.SetTypeA(other_word.GetTypeA(0) - 1, 0);
  other_word.Pack(other_packed);
  {
    int ret;
    if((ret = WordKey::Compare(packed, other_packed)) != 1) {
      fprintf(stderr, "dokey: %s different numeric field, expected 1 got %d\n", params->word_desc, ret);
      exit(1);
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

