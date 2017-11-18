//
// WordMeta.h
//
// NAME
// 
// abstract class to manage and use an inverted index file.
//
// SYNOPSIS
// 
// #include <mifluz.h>
// 
// WordContext context;
//
// WordMeta* words = context->Meta();
// 
// delete words;
// 
// DESCRIPTION
// 
// WordMeta is the <i>mifluz</i> equivalent of a database handler. Each
// WordMeta object is bound to an inverted index file and implements the
// operations to create it, fill it with word occurrences and search 
// for an entry matching a given criterion.
// 
// WordMeta is an abstract class and cannot be instanciated. 
// The <b>Meta</b> method of the class WordContext will create 
// an instance using the appropriate derived class, either WordMetaOne
// or WordMetaMulti. Refer to the corresponding manual pages for
// more information on their specific semantic.
//
// 
//
// END
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: WordMeta.h,v 1.4 2004/05/28 13:15:28 lha Exp $
//

#ifndef _WordMeta_h_
#define _WordMeta_h_

#include <stdio.h>

#include "htString.h"
#include "WordDB.h"

class WordContext;
class WordLock;
class WordMetaImp;

//
// Serial number range [1-2^32]
//
#define WORD_META_SERIAL_INVALID  0

#define WORD_META_SERIAL_WORD  0
#define WORD_META_SERIAL_FILE  1

class WordMeta 
{
 public:
  WordMeta() { words = 0; db = 0; imp = 0; }
  ~WordMeta();

  int Initialize(WordList* words);

  int Open();
  int Close();
    
  int Serial(int what, unsigned int& serial);
  int GetSerial(int what, unsigned int& serial);
  int SetSerial(int what, unsigned int serial);

  int Lock(const String& resource, WordLock*& lock);
  int Unlock(const String& resource, WordLock*& lock);
  
 private:
  WordList    *words;
  WordDB          *db;
  WordMetaImp          *imp;
};
#endif /* _WordMeta_h_ */
