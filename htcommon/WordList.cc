//
// WordList.cc
//
// WordList: Interface to the word database. Previously, this wrote to 
//           a temporary text file. Now it writes directly to the 
//           word database. 
//           NOTE: Some code previously attempted to directly read from 
//           the word db. This will no longer work, so it's preferred to 
//           use the access methods here.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordList.cc,v 1.32 1999/09/24 10:28:56 loic Exp $
//

#include "WordList.h"
#include "WordReference.h"
#include "WordRecord.h"
#include "HtWordType.h"
#include "Configuration.h"
#include "htString.h"
#include "DB2_db.h"
#include "HtPack.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fstream.h>

extern Configuration	config;

//
// Possible values of the action argument of WordList::Walk
//
#define HTDIG_WORDLIST			0x0001
#define HTDIG_WORDLIST_PREFIX		0x0002
#define HTDIG_WORDLIST_WORD		0x0004
#define HTDIG_WORDLIST_COLLECTOR	0x0008
#define HTDIG_WORDLIST_WALKER		0x0010
// 
// Short hand
//
#define HTDIG_WORDLIST_COLLECT		(HTDIG_WORDLIST|HTDIG_WORDLIST_COLLECTOR)
#define HTDIG_WORDLIST_COLLECT_PREFIX	(HTDIG_WORDLIST_PREFIX|HTDIG_WORDLIST_COLLECTOR)
#define HTDIG_WORDLIST_COLLECT_WORD	(HTDIG_WORDLIST_WORD|HTDIG_WORDLIST_COLLECTOR)
#define HTDIG_WORDLIST_WALK		(HTDIG_WORDLIST|HTDIG_WORDLIST_WALKER)
#define HTDIG_WORDLIST_WALK_PREFIX	(HTDIG_WORDLIST_PREFIX|HTDIG_WORDLIST_WALKER)
#define HTDIG_WORDLIST_WALK_WORD	(HTDIG_WORDLIST_WORD|HTDIG_WORDLIST_WALKER)

//*****************************************************************************
// WordList::~WordList()
//
WordList::~WordList()
{
    delete words;
    Close();
}

//*****************************************************************************
//
WordList::WordList(const Configuration& config_arg) :
  config(config_arg)
{
    words = new List;

    // The database itself hasn't been opened yet
    isopen = 0;
    isread = 0;
}

//*****************************************************************************
//
void WordList::Replace(const WordReference& arg)
{
  WordReference	wordRef(arg);
  String 	word = wordRef.Word();
  
  // Why should we add empty words?
  if (word.length() == 0)
    return;

    // Let's clean it up--lowercase it, check for caps, and strip punctuation
  String orig = word;
  word.lowercase();
    
  if (word != orig)
    wordRef.Flags(wordRef.Flags() | FLAG_CAPITAL);

  HtStripPunctuation(word);

  if (!IsValid(word))
    return;

  static int	maximum_word_length = config.Value("maximum_word_length", 12);
  if (word.length() > maximum_word_length)
    word.chop(word.length() - maximum_word_length);

  wordRef.Word(word);

  //
  // New word.  Create a new reference for it and cache it in the object.
  //
  words->Add(new WordReference(wordRef));
}


//*****************************************************************************
// int WordList::valid_word(char *word)
//   Words are considered valid if they contain alpha characters and
//   no control characters.  Also, if the word is found in the list of
//   bad words, it will be marked invalid.
//
int WordList::IsValid(const char *word)
{
    int		control = 0;
    int		alpha = 0;
    static int	allow_numbers = config.Boolean("allow_numbers", 0);
    static int	minimum_word_length = config.Value("minimum_word_length", 3);

    if (badwords.Exists(word))
	return 0;

    if ((int)strlen(word) < minimum_word_length)
      return 0;

    while (word && *word)
    {
      if (HtIsStrictWordChar((unsigned char)*word) && !isdigit(*word))
	{
	    alpha = 1;
	    // break;  /* Can't stop here, there may still be control chars! */
	}
      else if (allow_numbers && isdigit(*word))
	{
	  alpha = 1;
	  // break;    /* Can't stop here, there may still be control chars! */
	}
      else if (iscntrl(*word))
	{
	    control = 1;
	    break;
	}
      word++;
    }

    return alpha && !control;
}


//*****************************************************************************
// void WordList::Flush()
//   Dump the current list of words to the database.  After
//   the words have been dumped, the list will be destroyed to make
//   room for the words of the next document.
//   
void WordList::Flush()
{
  WordReference	*wordRef;

    // Provided for backwards compatibility
  if (!isopen)
    Open(config["word_db"], O_RDWR);

  words->Start_Get();
  while ((wordRef = (WordReference *) words->Get_Next()))
    {
      if (wordRef->Word().length() == 0) {
	cerr << "WordList::Flush: unexpected empty word\n";
	continue;
      }

      Add(*wordRef);
    }	
    
  // Cleanup
  words->Destroy();
}

//*****************************************************************************
// void WordList::MarkGone()
//   The current document has disappeared or been modified. 
//   We do not need to store these words.
//
void WordList::MarkGone()
{
  words->Destroy();
}


//*****************************************************************************
// void WordList::BadWordFile(char *filename)
//   Read in a list of words which are not to be included in the word
//   file.  This is mostly to exclude words that are too common.
//
void WordList::BadWordFile(const String& filename)
{
    FILE	*fl = fopen(filename, "r");
    char	buffer[1000];
    char	*word;
    String      new_word;
    static int	minimum_word_length = config.Value("minimum_word_length", 3);
    static int	maximum_word_length = config.Value("maximum_word_length", 12);

    // Read in the badwords file (it's just a text file)
    while (fl && fgets(buffer, sizeof(buffer), fl))
    {
	word = strtok(buffer, "\r\n \t");
	if (word && *word)
	  {
	    // We need to clean it up before we add it
	    // Just in case someone enters an odd one
	    if ((int)strlen(word) > maximum_word_length)
		word[maximum_word_length] = '\0';
	    new_word = word;
	    new_word.lowercase();
	    HtStripPunctuation(new_word);
	    if (new_word.length() >= minimum_word_length)
	      badwords.Add(new_word, 0);
	  }
    }

    if (fl)
	fclose(fl);
}


//*****************************************************************************
//
int WordList::Open(const String& filename, int mode)
{
  Close();
  
  dbf = 0;

  dbf = Database::getDatabaseInstance(DB_BTREE);

  if(!dbf) return NOTOK;

  dbf->SetCompare(word_db_cmp);

  isread = mode & O_RDONLY;

  int ret;

  if(isread)
    ret = dbf->OpenRead(filename);
  else
    ret = dbf->OpenReadWrite(filename, 0777);
  
  if(ret != OK)
    return NOTOK;

  isopen = 1;

  return OK;
}


//*****************************************************************************
// int WordList::Close()
//
int WordList::Close()
{
  if(isopen) {
    dbf->Close();
    delete dbf;
    dbf = 0;
    isopen = 0;
    isread = 0;
  }
  return OK;
}


//*****************************************************************************
//
int WordList::Add(const WordReference& wordRef)
{
  if (wordRef.Word().length() == 0)
    return NOTOK;

  int ret;

  String	key;
  String	record;

  if((ret = wordRef.Pack(key, record)) == OK) {
    dbf->Put(key, record);
  }

  return ret;
}


//*****************************************************************************
// List *WordList::operator [] (const WordReference& wordRef)
//
List *WordList::operator [] (const WordReference& wordRef)
{
  return Collect(wordRef, HTDIG_WORDLIST_COLLECT_WORD);
}


//*****************************************************************************
// List *WordList::Prefix (const WordReference& prefix)
//
List *WordList::Prefix (const WordReference& prefix)
{
  return Collect(prefix, HTDIG_WORDLIST_COLLECT_PREFIX);
}

//*****************************************************************************
// List *WordList::WordRefs()
//
List *WordList::WordRefs()
{
  return Collect(WordReference(), HTDIG_WORDLIST_COLLECT);
}

//
// Callback data dedicated to Dump and dump_word communication
//
class DumpWordData : public Object
{
public:
  DumpWordData(FILE* fl_arg) { fl = fl_arg; }

  FILE* fl;
};

//*****************************************************************************
// static int dump_word(WordList* words, WordReference* word)
//
// Write the ascii representation of a word occurence. Helper
// of WordList::Dump
//
static int dump_word(WordList *words, const WordReference *word, Object &data)
{
  DumpWordData &info = (DumpWordData &)data;

  word->Dump(info.fl);

  return OK;
}

//*****************************************************************************
// int WordList::Dump(char* filename)
//
// Write an ascii version of the word database in <filename>
//
int WordList::Dump(const String& filename)
{
  FILE		*fl;

  if (!isopen) {
    cerr << "WordList::Dump: database must be opened first\n";
    return NOTOK;
  }

  if((fl = fopen(filename, "w")) == 0) {
    perror(form("WordList::Dump: opening %s for writing", (const char*)filename));
    return NOTOK;
  }

  WordReference::DumpHeader(fl);
  DumpWordData data(fl);
  (void)Walk(WordReference(), HTDIG_WORDLIST_WALK, dump_word, data);
  
  fclose(fl);

  return OK;
}

//*****************************************************************************
//
List *WordList::Collect (const WordReference& wordRef, int action)
{
  Object o;
  return Walk(wordRef, action, (wordlist_walk_callback_t)0, o);
}

//*****************************************************************************
//
// Walk and collect data from the word database.
//
// If action bit HTDIG_WORDLIST is set all the words are retrieved.
// If action bit HTDIG_WORDLIST_WORD is set all the occurences of
//    the <word> argument (exact match) are retrieved.
// If action bit HTDIG_WORDLIST_PREFIX is set all the occurences of
//    the words starting with <word> are retrieved.
//
// If action bit HTDIG_WORDLIST_COLLECTOR is set WordReferences are
//    stored in a list and the list is returned.
// If action bit HTDIG_WORDLIST_WALKER is set the <callback> function
//    is called for each WordReference found. No list is built and the
//    function returns a null pointer.
//
List *WordList::Walk (const WordReference& wordRef, int action, wordlist_walk_callback_t callback, Object &callback_data)
{
    List        	*list = 0;
    int			prefixLength = wordRef.Word().length();

    if(action & HTDIG_WORDLIST_COLLECTOR) {
      list = new List;
    }

    if(action & HTDIG_WORDLIST) {
      dbf->Start_Get();
    } else {
      dbf->Start_Seq(wordRef.KeyPack());
    }

    String data;
    String key;
    while (dbf->Get_Next(data, key))
      {
	WordReference found(key, data);
	//
	// Stop loop if we reach a record whose key does not
	// match requirements.
	//
	if(action & HTDIG_WORDLIST_PREFIX) {
	  if(!wordRef.Key().Equal(found.Key(), prefixLength))
            break;
	} else if(action & HTDIG_WORDLIST_WORD) {
	  if(!wordRef.Key().Equal(found.Key(), 0))
	    break;
	}

	if(action & HTDIG_WORDLIST_COLLECTOR) {
	  list->Add(new WordReference(found));
	} else if(action & HTDIG_WORDLIST_WALKER) {
	  int ret = callback(this, &found, callback_data);
	  if(ret == NOTOK) break;
	} else {
	  // Useless to continue since we're not doing anything
	  break;
	}
      }

    return list;
}

//*****************************************************************************
//
int WordList::Exists(const WordReference& wordRef)
{
    return dbf->Exists(wordRef.KeyPack());
}

//*****************************************************************************
// int WordList::Delete(const WordReference& wordRef)
//
int WordList::Delete(const WordReference& wordRef)
{
    return dbf->Delete(wordRef.KeyPack());
}


//*****************************************************************************
// List *WordList::Words()
//
//
List *WordList::Words()
{
    List		*list = new List;
    String		key;
    String		record;
    String		word;
    WordReference	lastWord;

    dbf->Start_Get();
    while (dbf->Get_Next(record, key))
      {
	WordReference	wordRef(key, record);

	if ( (lastWord.Word() == String("")) || (wordRef.Word() != lastWord.Word()) )
	  {
            list->Add(new String(wordRef.Word()));
	    lastWord = wordRef;
	  }
      }
    return list;
}

