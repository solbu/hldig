//
// words.cc
//
// words: Remove words from documents that have been purged by the docs code.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: words.cc,v 1.22.2.9 2000/05/06 20:46:40 loic Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "htmerge.h"
#include "HtPack.h"

#include <errno.h>

//
// Callback data dedicated to Dump and dump_word communication
//
class DeleteWordData : public Object
{
public:
  DeleteWordData(const Dictionary& discard_arg) : discard(discard_arg) { deleted = remains = 0; }

  const Dictionary& discard;
  int deleted;
  int remains;
};

//*****************************************************************************
//
//
static int delete_word(WordList *words, WordDBCursor& cursor, const WordReference *word_arg, Object &data)
{
  const HtWordReference *word = (const HtWordReference *)word_arg;
  DeleteWordData& d = (DeleteWordData&)data;
  static String docIDStr;

  docIDStr = 0;
  docIDStr << word->DocID();

  if(d.discard.Exists(docIDStr)) {
    if(words->Delete(cursor) != 0) {
      cerr << "htmerge: deletion of " << (char*)word->Get() << " failed " << strerror(errno) << "\n";
      return NOTOK;
    }
    if (verbose)
      {
	cout << "htmerge: Discarding ";
	if(verbose > 2)
	  cout << (char*)word->Get();
	else 
	  cout << word->Word();
	cout << "\n";
	cout.flush();
      }
    d.deleted++;
  } else {
    d.remains++;
  }

  return OK;
}

//*****************************************************************************
// void mergeWords()
//
void mergeWords()
{
  HtWordList		words(config);
  DeleteWordData	data(discard_list); 

  words.Open(config["word_db"], O_RDWR);
  WordCursor* search = words.Cursor(delete_word, &data);
  search->Walk();
  delete search;
  words.Close();

  if (verbose)
    cout << "\n";
  if (stats)
    {
      cout << "htmerge: Total word count: " 
	   << data.remains << endl;
      cout << "htmerge: Total deleted words: " << data.deleted << endl;
    }

}


