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
//
#if RELEASE
static char RCSid[] = "$Id: WordList.cc,v 1.28 1999/09/10 13:24:15 loic Exp $";
#endif

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
// WordList::WordList()
//
WordList::WordList()
{
    words = new List;

    // The database itself hasn't been opened yet
    isopen = 0;
    isread = 0;

}


//*****************************************************************************
// WordList::~WordList()
//
WordList::~WordList()
{
    delete words;
    Close();
}


//*****************************************************************************
// void WordList::Word
// (String word, unsigned int location, unsigned int anchor_number,
//  unsigned long int flags)
//
void WordList::Word (String word, unsigned int location, 
		     unsigned int anchor_number,
		     unsigned int flags)
{
    // Why should we add empty words?
    if (word.length() == 0)
      return;

    // Let's clean it up--lowercase it, check for caps, and strip punctuation
    String 	orig = word;
    word.lowercase();
    
    if (word != orig)
      flags |= FLAG_CAPITAL;

    HtStripPunctuation(word);

    if (!valid_word(word))
	return;

    static int	maximum_word_length = config.Value("maximum_word_length", 12);
    if (word.length() > maximum_word_length)
	word.chop(word.length() - maximum_word_length);
    //
    // New word.  Create a new reference for it
    //
    WordReference	*wordRef = new WordReference;
    wordRef->Location = location;
    wordRef->DocumentID = docID;
    wordRef->Flags = flags;
    wordRef->Anchor = anchor_number;
    wordRef->Word = word;
    words->Add(wordRef);
}


//*****************************************************************************
// int WordList::valid_word(char *word)
//   Words are considered valid if they contain alpha characters and
//   no control characters.  Also, if the word is found in the list of
//   bad words, it will be marked invalid.
//
int WordList::valid_word(char *word)
{
    int		control = 0;
    int		alpha = 0;
    static int	allow_numbers = config.Boolean("allow_numbers", 0);
    static int	minimum_word_length = config.Value("minimum_word_length", 3);

    if (badwords.Exists(word))
	return 0;

    if (strlen(word) < minimum_word_length)
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
      Open(config["word_db"]);

    words->Start_Get();
    while ((wordRef = (WordReference *) words->Get_Next()))
      {
	if (wordRef->Word.length() == 0)
	  continue;

	Add(wordRef);
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
void WordList::BadWordFile(char *filename)
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
	    if (strlen(word) > maximum_word_length)
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
// int WordList::Open(char *filename)
//
int WordList::Open(char *filename)
{
    dbf = 0;

    dbf = Database::getDatabaseInstance(DB_BTREE);

    if (dbf->OpenReadWrite(filename, 0664) != OK)
        return NOTOK;
    else
      {
	isopen = 1;
        return OK;
      }
}


//*****************************************************************************
// int WordList::Read(char *filename)
//
int WordList::Read(char *filename)
{
    dbf = 0;

    dbf = Database::getDatabaseInstance(DB_BTREE);

    if (dbf->OpenRead(filename) != OK)
      return NOTOK;
    else
      {
	isopen = 1;
	isread = 1;
	return OK;
      }
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
// int WordList::Add(WordReference *wordRef)
//
int WordList::Add(WordReference *wordRef)
{
  if (!wordRef || wordRef->Word.length() == 0)
    return NOTOK;

  String	key;
  String	compressedData;
  WordRecord	*wordRec = new WordRecord;

  // Construct the key, which is word conjoined by the DocID
  // \001 is used to join them simply because it's never a word char
  key = wordRef->Word;
  key << "\001" << wordRef->DocumentID;
  
  // Now split out the WordRecord to store
  wordRec->id = wordRef->DocumentID;
  wordRec->flags = wordRef->Flags;
  wordRec->anchor = wordRef->Anchor;
  wordRec->location = wordRef->Location;
  
  // We need to compress the WordRecord and convert it into a binary form
  compressedData = htPack(WORD_RECORD_COMPRESSED_FORMAT,
			  (char *) wordRec);
  
  dbf->Put(wordRef->Word, compressedData.get(), compressedData.length());

  delete wordRec;

  return OK;
}


//*****************************************************************************
// List *WordList::operator [] (String word)
//
List *WordList::operator [] (String word)
{
  return Collect(word, HTDIG_WORDLIST_COLLECT_WORD);
}


//*****************************************************************************
// List *WordList::Prefix (String prefix)
//
List *WordList::Prefix (String prefix)
{
  return Collect(prefix, HTDIG_WORDLIST_COLLECT_PREFIX);
}

//*****************************************************************************
// List *WordList::WordRefs()
//
List *WordList::WordRefs()
{
  return Collect("", HTDIG_WORDLIST_COLLECT);
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
static int dump_word(WordList *words, WordReference *word, Object &data)
{
  DumpWordData &info = (DumpWordData &)data;

  word->Dump(info.fl);
}

//*****************************************************************************
// int WordList::Dump(char* filename)
//
// Write an ascii version of the word database in <filename>
//
int WordList::Dump(char* filename)
{
  FILE		*fl;
  int		ret;

  if (!isopen) {
    cerr << "WordList::Dump: database must be opened first\n";
    return NOTOK;
  }

  if((fl = fopen(filename, "w")) == 0) {
    perror(form("WordList::Dump: opening %s for writing", filename));
    return NOTOK;
  }

  WordReference::DumpHeader(fl);
  DumpWordData data(fl);
  (void)Walk(0, HTDIG_WORDLIST_WALK, dump_word, data);
  
  fclose(fl);
}

//*****************************************************************************
//
List *WordList::Collect (String word, int action)
{
  Object o;
  return Walk(word, action, (wordlist_walk_callback_t)0, o);
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
List *WordList::Walk (String word, int action, wordlist_walk_callback_t callback, Object &callback_data)
{
    List        *list = 0;
    char        *key;
    String	wordKey;
    String	data;
    String      decompressed;
    WordRecord  *wr = new WordRecord;
    int		prefixLength = word.length();

    if(action & HTDIG_WORDLIST_COLLECTOR) {
      list = new List;
    }

    if(action & HTDIG_WORDLIST) {
      dbf->Start_Get();
    } else {
      dbf->Start_Seq(word.get());
    }
    while ((key = dbf->Get_Next(data)))
      {

        // Break off the \001 and the docID
        wordKey = strtok(key, "\001");
	//
	// Stop loop if we reach a record whose key does not
	// match requirements.
	//
	if(action & HTDIG_WORDLIST_PREFIX) {
	  if(word != wordKey.sub(0, prefixLength))
            break;
	} else if(action & HTDIG_WORDLIST_WORD) {
	  if(word != wordKey) {
	    break;
	  }
	}

        if (data.length())
          {
	    char	*unpack = data.get();
	    decompressed = htUnpack(WORD_RECORD_COMPRESSED_FORMAT,
			  unpack);

            if (decompressed.length() != sizeof (WordRecord))
	      {
		cout << "Decoding mismatch" << endl;
		continue;
	      }

            memcpy((char *) wr, decompressed.get(), sizeof(WordRecord));

            WordReference *wordRef = new WordReference;
            wordRef->Word = wordKey;
            wordRef->DocumentID = wr->id;
            wordRef->Flags = wr->flags;
            wordRef->Anchor = wr->anchor;
            wordRef->Location = wr->location;

	    if(action & HTDIG_WORDLIST_COLLECTOR) {
	      list->Add(wordRef);
	    } else if(action & HTDIG_WORDLIST_WALKER) {
	      if(callback(this, wordRef, callback_data) == NOTOK) {
		break;
	      }
	    } else {
	      // Useless to continue since we're not doing anything
	      break;
	    }
          }
      }

    return list;
}

//*****************************************************************************
// int WordList::Exists(WordReference wordRef)
//
int WordList::Exists(WordReference wordRef)
{
    String	key;

    // Construct the key, which is word conjoined by the DocID
    // \001 is used to join them simply because it's never a word char
    key = wordRef.Word;
    key << "\001" << wordRef.DocumentID;

    return dbf->Exists(key);
}

//*****************************************************************************
// int WordList::Exists(String word)
//
int WordList::Exists(String word)
{
    // Attempt to retrieve the word, but see if we actually get anything back
    dbf->Start_Seq(word.get());
    return (dbf->Get_Next() != 0);
}


//*****************************************************************************
// int WordList::Delete(WordReference wordRef)
//
int WordList::Delete(WordReference wordRef)
{
    String      key;

    // Construct the key, which is word conjoined by the DocID
    // \001 is used to join them simply because it's never a word char
    key = wordRef.Word;
    key << "\001" << wordRef.DocumentID;

    return dbf->Delete(key);
}


//*****************************************************************************
// List *WordList::Words()
//
// Rewrite this with Walk + callback
//
List *WordList::Words()
{
    List	*list = new List;
    char	*key;
    String	word;
    String	lastWord = 0;

    dbf->Start_Get();
    while ((key = dbf->Get_Next()))
      {
	// Break off the \001 and the docID
	word = strtok(key, "\001");

	if ( (lastWord == 0) || (word != lastWord) )
	  {
            list->Add(new String(word));
	    lastWord = word;
	  }
      }
    return list;
}

