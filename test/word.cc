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
// $Id: word.cc,v 1.14.2.10 2000/01/06 13:58:29 bosc Exp $
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

#include "WordKey.h"
#include "WordList.h"
#include "WordType.h"
#include "Configuration.h"

static ConfigDefaults defaults[] = {
  { "wordlist_wordkey_description", "nfields: 4/Location 16 3/Flags 8 2/DocID 32 1/Word 0 0", 0 },
  { "wordlist_wordrecord_description","DATA", 0 },
  { "word_db", "var/htdig/db.words.db", 0 },
  { "wordlist_extend", "true", 0 },
  { "minimum_word_length", "1", 0},
  { "wordlist_cache_size", "1000000"}, 
  { 0 }
};
static ConfigDefaults compress_defaults[] = {
  { "wordlist_wordkey_description", "nfields: 4/Location 16 3/Flags 8 2/DocID 32 1/Word 0 0", 0 },
  { "word_db", "var/htdig/db.words.db", 0 },
  { "wordlist_extend", "true", 0 },
  { "minimum_word_length", "1", 0},
  { "wordlist_compress", "true", 0},
  { "wordlist_compress_debug", "2", 0},
  { "wordlist_cache_size", "1000000"}, 
  { 0 }
};

static Configuration	config;

typedef struct 
{
    char* word_desc;
    int key;
    int list;
    int skip;
    int bitstream;
    int compress;
} params_t;

static void usage();
static void doword(params_t* params);
static void dolist(params_t* params);
static void dokey(params_t* params);
static void doskip(params_t* params);
static void dobitstream(params_t* params);
static void pack_show(const WordReference& wordRef);

static int verbose = 0;

// *****************************************************************************
// int main(int ac, char **av)
//

int main(int ac, char **av)
{
  int			c;
  extern char		*optarg;
  params_t		params;

  params.word_desc = strdup("???");
  params.key = 0;
  params.list = 0;
  params.skip = 0;
  params.bitstream = 0;
  params.compress = 0;

  while ((c = getopt(ac, av, "vklbszw:")) != -1)
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
	case 'k':
	  params.key = 1;
	  break;
	case 'l':
	  params.list = 1;
	  break;
	case 's':
	  params.skip = 1;
	  break;
	case 'b':
	  params.bitstream = 1;
	  break;
	case 'z':
	  params.compress = 1;
	  break;
	case '?':
	  usage();
	  break;
	}
    }

  doword(&params);

  free(params.word_desc);

  return 0;
}

static void doword(params_t* params)
{
  if(params->key) {
    if(verbose) cerr << "Test WordKey class with " << params->word_desc << "\n";
    dokey(params);
  }
  if(params->list || params->skip) 
  {
      if(params->compress) 
      {
	  config.Defaults(compress_defaults);
      }
      else{config.Defaults(defaults);}
    // Ctype-like functions for what constitutes a word.
    WordList::Initialize(config);
    unlink(config["word_db"]);
  }


  if(params->list)
  {
    if(verbose) cerr << "Test WordList class\n";
    dolist(params);
  }
  if(params->skip) 
  {
      if(verbose) cerr << "Test SkipUselessSequentialWalking in WordList class "  << "\n";
      doskip(params);
  }

  if(params->bitstream) 
  {
      if(verbose) cerr << "Test BitStream class "  << "\n";
      dobitstream(params);
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
	
    // setup a new wordlist
    WordList words(config);
    if(verbose)WordKeyInfo::Get()->show();	
    words.Open(config["word_db"], O_RDWR);


    // create entries from word_list
    WordReference wordRef;
    wordRef.Key().SetInSortOrder(2, 67);
    unsigned int location = 0;
    unsigned int anchor = 0;
    unsigned int docid = 1;
    if(verbose) fprintf(stderr, "Inserting\n");

    for(char** p = word_list; *p; p++) 
    {
	if(verbose > 4) cerr << "inserting word:" << *p << "\n";
	wordRef.Key().SetWord(*p);
  	wordRef.Key().SetInSortOrder(1, docid);
  	wordRef.Key().SetInSortOrder(3, location);
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

    // reopen wordlist
    words.Open(config["word_db"], O_RDONLY);
    //  check if each word (from word_list) is there
    for(char** p = word_list; *p; p++) 
    {
      // recreate wordref from each word
      wordRef.Key().SetWord(*p);
      wordRef.Key().SetInSortOrder(3, location);
      wordRef.Record().info.data = anchor;
      wordRef.Key().SetInSortOrder(1, docid);

      location += strlen(*p);
      anchor++;
      docid++;

      //
      // Skip first word because we don't want to deal with upper/lower case at present.
      //
      if(p == word_list) continue;

      // check if wordref is in wordlist 
      if(verbose) fprintf(stderr, "searching for %s ... ", *p);
      if(verbose > 2) pack_show(wordRef);
      if(verbose > 1) cerr << wordRef << "\n";
      // find matches in wordlist
      List *result = words[wordRef];
      result->Start_Get();
      int count = 0;
      WordReference* found;
      // loop through found matches
      while((found = (WordReference*)result->Get_Next())) 
      {
	if(wordRef.Key().GetWord() != found->Key().GetWord()) 
	{
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
    if(verbose){cerr << "**** Delete test:" << words  <<endl;cerr << "**** Delete test:" << endl;}
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
    wordRef.Key().SetInSortOrder(1, 5);
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
#include"HtRandom.h"

static void 
dokey(params_t* params)
{
    int ikey,j,ikeydesc;
    char *test_keys[]=
    {
	"nfields: 4/DocID 5 1/Flags 8 2/Location 19  3/Word 0 0",
	"nfields: 4/DocID 3 1/Location  2 3/Flags 11 2/Word 0 0",
	"nfields: 4/DocID 3 1/Location  5 3/Flags 8  2/Word 0 0",
	"nfields: 4/DocID 3 1/Location  7 3/Flags 14 2/Word 0 0",
	"nfields: 6/DocID 3 1/Location  7 3/Flags 9  2/Foo1 13 4/Foo2 16 5/Word 0 0",
    };

    for(ikeydesc=0;ikeydesc<1000;ikeydesc++)
    {
        // check predefined keys structures first
	if( ikeydesc < (int)(sizeof(test_keys)/sizeof(*test_keys)))
	{
	    WordKeyInfo::SetKeyDescriptionFromString(test_keys[ikeydesc]);
	}
	else
	{
	// check random predefined keys structures afterwards
	    WordKeyInfo::SetKeyDescriptionRandom();
	}

  	if(verbose)WordKeyInfo::Get()->show();	
	for(ikey=0;ikey<20;ikey++)
	{
	    WordKey word;
	    word.SetRandom();
  	    if(verbose>1)cout << "WORD :" << word << endl;

	    String packed;
	    word.Pack(packed);
//  	    WordKey::show_packed(packed,1);

	    WordKey other_word;
	    other_word.Unpack(packed);

  	    if(verbose>1)cout << "OTHER_WORD:" << other_word << endl;
	    int failed =0 ;
	    for(j=1;j<word.nfields();j++)
	    {
		if(word.GetInSortOrder(j)!=other_word.GetInSortOrder(j))
		{failed=1;}
	    }
	    if(word.GetWord()!=other_word.GetWord() || !word.IsDefinedInSortOrder(0))
	    {failed=1;}

	    if(failed)
	    {
		printf("DOKEY failed, original and packed/unpacked not equal\n");
		WordKeyInfo::Get()->show();	
		cout << "WORD :" << word << endl;
		WordKey::show_packed(packed,1);
		cout << "OTHER_WORD:" << other_word << endl;
		exit(1);
	    }

	    //
	    // Compare in packed form
	    //
	    if(!word.PackEqual(other_word)) 
	    {
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

	    word.SetWord("Test string");
	    word.SetInSortOrder(1,1);
	    other_word.SetWord("Test string");
	    word.Pack(packed);
	    //
	    // Add one char to the word, they must not compare equal and
	    // the difference must be minus one.
	    //
	    other_word.GetWord().append("a");
	    other_word.Pack(other_packed);
	    {
		int ret;
		if((ret = WordKey::Compare(packed, other_packed)) != -1) 
		{
		    cerr << word << endl << other_word << endl;
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
		if((ret = WordKey::Compare(packed, other_packed)) != 1) 
		{
		    cerr << word << endl << other_word << endl;
		    fprintf(stderr, "dokey: %s different letter (S instead of T), expected 1 got %d\n", params->word_desc, ret);
		    exit(1);
		}
	    }
	    other_word.GetWord().set("Test string");
  
	    //
	    // Substract one to the first numeric field
	    // The difference must be one.
	    //
	    other_word.SetInSortOrder(1,word.GetInSortOrder(1) - 1);
	    other_word.Pack(other_packed);
	    {
		int ret;
		if((ret = WordKey::Compare(packed, other_packed)) != 1) 
		{
		    cerr << word << endl << other_word << endl;
		    fprintf(stderr, "dokey: %s different numeric field, expected 1 got %d\n", params->word_desc, ret);
		    exit(1);
		}
	    }

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
// void doskip()
//   Test SkipUselessSequentialWalking in WordList class
//
#include<ctype.h>
#include<fstream.h>
int 
get_int_array(char *s,int **plist,int &n)
{
    int i=0;
    for(n=0;s[i];n++)
    {	
	for(;s[i] && !isalnum(s[i]);i++);
	if(!s[i]){break;}
	for(;s[i] && isalnum(s[i]);i++);
    }
    if(!n){*plist=NULL;return(NOTOK);}
    int *list=new int[n];
    *plist=list;
    int j;
    i=0;
    for(j=0;s[i];j++)
    {	
	for(;s[i] && !isalnum(s[i]);i++);
	if(!s[i]){break;}
	list[j]=atoi(s+i);
//  	cout << "adding number:" << s+i << "=" << atoi(s+i) << endl;
	for(;s[i] && isalnum(s[i]);i++);
    }
//      cout << "get_int_array: input:" << s << ":" << endl;
//      cout << "result: n:" << n << ":";
//      for(i=0;i<n;i++){cout << (*plist)[i] << " " ;}
//      cout << endl;
    return(OK);
}
class SkipTestEntry
{
public:
    char *searchkey;
    char *goodorder;
    void GetSearchKey(WordKey &searchKey)
    {
	searchKey.Set((String)searchkey);
	if(verbose) cout << "GetSearchKey: string:" << searchkey << " got:" << searchKey << endl;
    }
    int Check(WordList &WList)
    {
	WordKey empty;
	WordReference srchwrd;
	GetSearchKey(srchwrd.Key());
	Object o;
	if(verbose) cout << "checking SkipUselessSequentialWalking on:" << srchwrd << endl;
	if(verbose) cout << "walking all:" << endl;
//	WList.verbose=5;
	List *all=WList.Search(WordSearchDescription(empty));
	if(verbose) cout << "walking search: searching for:" << srchwrd <<endl;

	WordSearchDescription search(srchwrd.Key());
	search.traceRes = new List;
	WList.Walk(search);
	List *wresw = search.collectRes;
	List *wres  = search.traceRes;
	wresw->Start_Get();
	wres->Start_Get();
	WordReference *found;
	WordReference *correct;
	int i;
	int ngoodorder;
	int *goodorder_a;
	get_int_array(goodorder,&goodorder_a,ngoodorder);
	for(i=0;(found = (WordReference*)wres->Get_Next());i++) 
	{
  	    if(i>=ngoodorder){cerr << "SkipUselessSequentialWalking test failed! i>=ngoodorder" << endl;exit(1);}
	    if(verbose) cout << "Check actual  " << i              << "'th  walked:" << *found   << endl;
	    correct = (WordReference*)all->Nth(goodorder_a[i]);
	    if(verbose) cout << "Check correct " << goodorder_a[i] << "           :" << *correct << endl;    
	    if(!correct->Key().Equal(found->Key()))
	    {cout << "SkipUselessSequentialWalking test failed! at position:" << i << endl;exit(1);}
	}
	if(i<ngoodorder){cerr << "SkipUselessSequentialWalking test failed! n<ngoodorder" << endl;exit(1);}
	
	delete [] goodorder_a;
	delete wresw;
	delete wres;
	return OK;
    }
};

SkipTestEntry SkipTestEntries[]=
{
     {
 	"et  <DEF>      <UNDEF>       0       10      ",
 	"3 4 5 9 10 12 13 14"
     },
     {
 	"et  <UNDEF>    20            0       <UNDEF> ",
 	"3 4 5 6 7 8 9 14 15 16 17",
     },
};

static void 
doskip(params_t*)
{
    if(verbose > 0) cerr << "doing SkipUselessSequentialWalking test" << endl;
    // read db into WList from file: skiptest_db.txt
    if(verbose){cout<< "WList config:minimum_word_length:" << config.Value("minimum_word_length")<< endl;}
    WordList WList(config);
    WList.Open(config["word_db"], O_RDWR);
    ifstream ins("skiptest_db.txt");
    if(!ins.good())
    {
	cerr << "SkipUselessSequentialWalking test failed : read db file:skiptest_db.txt failed" << endl;
	exit(1);
    }
    ins >> WList;
    if(verbose){cout << "WList read:" << endl << WList << endl;}
    // now check walk order for a few search terms
    int i;
    if(verbose) cout << "number of entries:"<< sizeof(SkipTestEntries)/sizeof(SkipTestEntry) << endl;
    for(i=0;i<(int)(sizeof(SkipTestEntries)/sizeof(SkipTestEntry));i++)
    {
	if(SkipTestEntries[i].Check(WList) == NOTOK)
	{
	    cerr << "SkipUselessSequentialWalking test failed on SkipTestEntry number:" << i <<endl;
	    exit(1);
	}
    }

}



#include"WordBitCompress.h"

void bitstream_int();
void bitstream_bit();

static void 
dobitstream(params_t*)
{
    bitstream_int();
    bitstream_bit();
}
void
bitstream_int()
{ 
    BitStream bs;
    bs.set_use_tags();
    const int nnums=5000;
    unsigned int nums[nnums];
    unsigned int szs[nnums];
    int i,sz;
    char tag[1000];
    printf("inserting (uint):\n");
    for(i=0;i<nnums;i++)
    {
//  	printf("-----------%5d ------------\n",i);
	sz=rand()%33;
	szs[i]=sz;
	if(sz==32){nums[i]=(rand())&(0xffffffff);}
	else{nums[i]=(rand())&(pow2(sz)-1);}
	sprintf(tag,"tag(i:%d,sz:%d,val:%x)",i,sz,nums[i]);
	bs.put_uint(nums[i],sz,tag);
    }

    printf("checking (uint):\n");
//      bs.show();
    bs.rewind();
    for(i=0;i<nnums;i++)
    {
//    	printf("-----------%5d ------------\n",i);
	unsigned int v;
	sz=szs[i];
	sprintf(tag,"tag(i:%d,sz:%d,val:%x)",i,sz,nums[i]);
	v=bs.get_uint(sz,tag);
	if(v!=nums[i])
	{
	    cerr << "BitStream failed for pos:" << i << " size:" << sz << " found:" << v << " tag:" << tag << endl;
	    exit(1);
	}
    }
}

void
bitstream_bit()
{ 
    BitStream bs;
    bs.set_use_tags();
    const int nnums=1000;
    unsigned int nums[nnums];
    int i;
    char tag[1000];
    printf("inserting (bit):\n");
    for(i=0;i<nnums;i++)
    {
//    	printf("-----------%5d ------------\n",i);
	nums[i]=(rand()/100)%2;
	sprintf(tag,"tag(i:%d,val:%x)",i,nums[i]);
//  	printf("tag:%s\n",tag);
	bs.put(nums[i],tag);
    }

    printf("checking (bit):\n");
//       bs.show();
    bs.rewind();
    for(i=0;i<nnums;i++)
    {
	unsigned int v;
	sprintf(tag,"tag(i:%d,val:%x)",i,nums[i]);
	v=bs.get(tag);
	if((v==0)!=(nums[i]==0))
	{
	    cerr << "BitStream failed for pos:" << i << " v:" << v << " should be:" << nums[i] << " tag:" << tag << endl;
	    exit(1);
	}
    }
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
    cout << "\t-k\t\tTest WordKey\n";
    cout << "\t-l\t\tTest WordList\n";
    cout << "\t-z\t\tActivate compression test\n";
    cout << "\t-s\t\tTest Skip\n";
    exit(0);
}

