//
// WordStat.h
//
// WordStat: Kind of record that holds statistics about each distinct word
//            in the database.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordStat.h,v 1.1 1999/10/05 16:03:31 loic Exp $
//
#ifndef _WordStat_h_
#define _WordStat_h_

#include "WordReference.h"

class WordStat : public WordReference
{
 public:
  //
  // Construction/Destruction
  //
  WordStat()	{ record.type = WORD_RECORD_STATS; }
  WordStat(const String& key, const String& record) : WordReference(key, record) {
    WordStat::record.type = WORD_RECORD_STATS;
  }
  WordStat(const String& word) {
    Clear();
    key.SetWord(String("\001") + word);
    record.type = WORD_RECORD_STATS;
  }
  
  ~WordStat()	{}

  //
  // Accessors
  //
  unsigned int Noccurence() const { return record.info.stats.noccurence; }
  unsigned int &Noccurence() { return record.info.stats.noccurence; }

  //
  // Return upper boundary key of reference count records
  //
  static inline const WordReference& Last() {
    if(!word_stat_last)
      word_stat_last = new WordReference("\002");
    return *word_stat_last;
  }

 protected:

  static WordReference*		word_stat_last;
};

#endif


