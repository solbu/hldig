//
// WordList.cc
//
//: Interface to the word database. Previously, this wrote to a temporary text
//  file. Now it writes directly to the word database. 
//  NOTE: Some code previously attempted to directly read from the word db.
//  This will no longer work, so it's preferred to use the access methods here
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
//
#if RELEASE
static char RCSid[] = "$Id: WordList.cc,v 1.23 1999/08/31 07:25:01 ghutchis Exp $";
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

    if (isopen)
      Close();
}


//*****************************************************************************
// void WordList::Word
// (String word, unsigned int location, unsigned int anchor_number,
//  unsigned long int flags)
//
void WordList::Word (String word, unsigned int location, 
		     unsigned int anchor_number,
		     unsigned long int flags)
{
    // Why should we add empty words?
    if (word.length() == 0)
      return;

    // Let's start to clean it up
    String 	orig = word;
    word.lowercase();
    
    if (word != orig)
      flags |= FLAG_CAPITAL;

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
    WordRecord		*wordRec = new WordRecord;
    Database		*dbf = Database::getDatabaseInstance(DB_BTREE);
    char		*wordfile = config["word_db"];
    String		compressedData;
    String		key;


    if (dbf->OpenReadWrite(wordfile, 0664) == NOTOK)
    {
      // Need a useful error here, yet we're basically dead.
    }

    words->Start_Get();
    while ((wordRef = (WordReference *) words->Get_Next()))
    {
      if (wordRef->Word.length() == 0)
	continue;

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
    }

    // Cleanup
    delete wordRec;
    words->Destroy();
    dbf->Close();
    delete dbf;
}


//*****************************************************************************
// void WordList::MarkScanned()
//   The current document hasn't changed.  No need to redo the words.
//
void WordList::MarkScanned()
{
    words->Destroy();
}


//*****************************************************************************
// void WordList::MarkGone()
//   The current document has disappeared.  The merger can take out
//   all references to the current doc.
//
void WordList::MarkGone()
{
    words->Destroy();
}


//*****************************************************************************
// void WordList::MarkModified()
//   The current document has disappeared.  The merger can take out
//   all references to the current doc.
//
void WordList::MarkModified()
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
    dbf->Close();
    delete dbf;
    dbf = 0;
    isopen = 0;
    isread = 0;
    return OK;
}


//*****************************************************************************
// int WordList::Add(WordReference *wordRef)
//
int WordList::Add(WordReference *wordRef)
{
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
    List        *list = new List;
    char        *key;
    String	wordKey;
    String      data;
    String      decompressed;
    WordRecord  *wr = new WordRecord;

    dbf->Start_Seq(word.get());
    while ((key = dbf->Get_Next(data)))
      {

        // Break off the \001 and the docID
        wordKey = strtok(key, "\001");
        if (word != wordKey)
            break;

        if (data.length())
          {
	    decompressed = htUnpack(WORD_RECORD_COMPRESSED_FORMAT,
			  data.get());

            if (decompressed.length() != sizeof (WordRecord))
	      {
		cout << "Decoding mismatch" << endl;
		continue;
	      }

            memcpy((char *) wr, decompressed.get(), sizeof(WordRecord));

            WordReference *wordRef = new WordReference;
            wordRef->Word = word;
            wordRef->DocumentID = wr->id;
            wordRef->Flags = wr->flags;
            wordRef->Anchor = wr->anchor;
            wordRef->Location = wr->location;

            list->Add(wordRef);
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


//*****************************************************************************
// List *WordList::WordRefs()
//
List *WordList::WordRefs()
{
    List        *list = new List;
    char        *key;
    char	*word;
    String	data;
    String	decompressed;
    WordRecord  *wr = new WordRecord;

    dbf->Start_Get();
    while ((key = dbf->Get_Next(data)))
      {
        // Break off the \001 and the docID
        word = strtok(key, "\001");

        if (data.length())
          {
	    decompressed = htUnpack(WORD_RECORD_COMPRESSED_FORMAT,
			  data.get());

            if (decompressed.length() != sizeof (WordRecord))
	      {
		cout << "Decoding mismatch" << endl;
		continue;
	      }

            memcpy((char *) wr, decompressed.get(), sizeof(WordRecord));

            WordReference *wordRef = new WordReference;
            wordRef->Word = word;
            wordRef->DocumentID = wr->id;
            wordRef->Flags = wr->flags;
            wordRef->Anchor = wr->anchor;
            wordRef->Location = wr->location;

	    list->Add(wordRef);

	  }
      }

    return list;
}
