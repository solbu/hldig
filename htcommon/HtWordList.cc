//
// HtWordList.cc
//
// HtWordList: Specialized WordList class that can hold a list 
//	       of words waiting to be inserted in the database.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtWordList.cc,v 1.2 1999/10/01 15:19:28 loic Exp $
//

#include "HtWordList.h"
#include "HtWordReference.h"
#include "WordRecord.h"
#include "WordType.h"
#include "Configuration.h"
#include "htString.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <iostream.h>
#include <fstream.h>
#include <errno.h>

//*****************************************************************************
// HtWordList::~HtWordList()
//
HtWordList::~HtWordList()
{
    delete words;
}

//*****************************************************************************
//
HtWordList::HtWordList(const Configuration& config_arg) :
  WordList(config_arg)
{
    words = new List;
}

//*****************************************************************************
//
void HtWordList::Replace(const WordReference& arg)
{
  //
  // New word.  Create a new reference for it and cache it in the object.
  //
  words->Add(new WordReference(arg));
}

//*****************************************************************************
// void HtWordList::Flush()
//   Dump the current list of words to the database.  After
//   the words have been dumped, the list will be destroyed to make
//   room for the words of the next document.
//   
void HtWordList::Flush()
{
  HtWordReference	*wordRef;

    // Provided for backwards compatibility
  if (!isopen)
    Open(config["word_db"], O_RDWR);

  words->Start_Get();
  while ((wordRef = (HtWordReference *) words->Get_Next()))
    {
      if (wordRef->Word().length() == 0) {
	cerr << "HtWordList::Flush: unexpected empty word\n";
	continue;
      }

      Override(*wordRef);
    }	
    
  // Cleanup
  words->Destroy();
}

//*****************************************************************************
// void HtWordList::MarkGone()
//   The current document has disappeared or been modified. 
//   We do not need to store these words.
//
void HtWordList::MarkGone()
{
  words->Destroy();
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
//
// Write the ascii representation of a word occurence. Helper
// of WordList::Dump
//
static int dump_word(WordList *, WordCursor &, const WordReference *word, Object &data)
{
  const HtWordReference *word_tmp = (const HtWordReference *)word;

  DumpWordData &info = (DumpWordData &)data;

  word_tmp->Dump(info.fl);

  return OK;
}

//*****************************************************************************
// int HtWordList::Dump(char* filename)
//
// Write an ascii version of the word database in <filename>
//
int HtWordList::Dump(const String& filename)
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

  HtWordReference::DumpHeader(fl);
  DumpWordData data(fl);
  (void)Walk(WordReference(), HTDIG_WORDLIST_WALK, dump_word, data);
  
  fclose(fl);

  return OK;
}

