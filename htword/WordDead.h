//
// WordDead.h
//
// NAME
// 
// list of documents that must be ignored and then deleted from the index.
//
// SYNOPSIS
// 
// Helper for the WordList class.
// 
// DESCRIPTION
// 
// WordDead is a list of WordKey entries describing deleted documents.
// All inverted index entries that match a WordKey entry of the WordDead
// list are treated as if they do not appear in the inverted index.
//
// 
//
// END
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordDead.h,v 1.1.2.1 2000/09/14 03:13:27 ghutchis Exp $
//

#ifndef _WordDead_h_
#define _WordDead_h_

#include <stdio.h>

#include "htString.h"
#include "WordDB.h"

class WordList;
class WordDeadCursor;

class WordDead 
{
 public:
  WordDead() { words = 0; db = 0; mask = 0; }
  ~WordDead();

  int Initialize(WordList* words);

  int Open();
  int Remove();
  int Close();
    
  int Mask(const WordKey& nmask) { *mask = nmask; return OK; }

  List* Words() const;

  int Normalize(WordKey& key) const;
  int Exists(const WordKey& key) const;
  int Put(const WordKey& key) const;

  WordDeadCursor* Cursor() const;
  int Next(WordDeadCursor* cursor, WordKey& key);

 private:
  WordList*			words;
  WordDB*	            	db;
  WordKey*			mask;
};
#endif /* _WordDead_h_ */
