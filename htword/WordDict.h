//
// WordDict.h
//
// NAME
// 
// manage and use an inverted index dictionary.
//
// SYNOPSIS
// 
// #include <mifluz.h>
// 
// WordList* words = ...;
// WordDict* dict = words->Dict();
// 
// DESCRIPTION
// 
// WordDict maps strings to unique identifiers and frequency in the 
// inverted index. Whenever a new word is found, the WordDict class 
// can be asked to assign it a serial number. When doing so, an entry
// is created in the dictionary with a frequency of zero. The application
// may then increment or decrement the frequency to reflect the inverted
// index content.
//
// The serial numbers range from 1 to 2^32 inclusive.
//
// A WordDict object is automatically created by the WordList object and
// should not be created directly by the application.
//
// END
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: WordDict.h,v 1.4 2004/05/28 13:15:26 lha Exp $
//

#ifndef _WordDict_h_
#define _WordDict_h_

#include <stdio.h>

#ifndef SWIG
#include "htString.h"
#include "WordDB.h"

class WordList;
class WordDictCursor;

#define WORD_DICT_SERIAL_INVALID  0

class WordDictRecord {
 public:
  inline WordDictRecord() { count = 0; id = WORD_DICT_SERIAL_INVALID; }

  inline int Unpack(const String& coded) {
    int offset = 0;
    coded.ber_shift(offset, count);
    coded.ber_shift(offset, id);
    return OK;
  }

  inline int Pack(String& coded) const {
    int offset = 0;
    coded.ber_push(offset, count);
    coded.ber_push(offset, id);
    return OK;
  }

  inline int Get(WordDB* db, const String& word) {
    String tmp_word = word;
    String coded(BER_MAX_BYTES * 2);
    int ret;
    if((ret = db->Get(0, tmp_word, coded, 0)) != 0) return ret;

    Unpack(coded);

    return ret;
  }
  
  inline int Put(WordDB* db, const String& word) {
    String coded(BER_MAX_BYTES * 2);
    Pack(coded);
    return db->Put(0, word, coded, 0);
  }

  inline int Del(WordDB* db, const String& word) {
    return db->Del(0, word);
  }

  inline unsigned int Count() { return count; }
  inline unsigned int Id() { return id; }

  unsigned int count;
  unsigned int id;
};
#endif /* SWIG */

class WordDict 
{
 public:
#ifndef SWIG
  //-
  // Private constructor. 
  //
  WordDict() { words = 0; db = 0; }
  ~WordDict();

  //-
  // Bind the object a WordList inverted index. Return OK on success,
  // NOTOK otherwise.
  //
  int Initialize(WordList* words);

  //-
  // Open the underlying Berkeley DB sub-database. The enclosing 
  // file is given by the <i>words</i> data member. Return OK on success,
  // NOTOK otherwise.
  //
  int Open();
  //-
  // Destroy the underlying Berkeley DB sub-database. Return OK on success,
  // NOTOK otherwise.
  //
  int Remove();
  //-
  // Close the underlying Berkeley DB sub-database. Return OK on success,
  // NOTOK otherwise.
  //
  int Close();
    
  //-
  // If the <b>word</b> argument exists in the dictionnary, return its
  // serial number in the <b>serial</b> argument. If it does not already
  // exists, assign it a serial number, create an entry with a frequency
  // of zero and return the new serial in the <b>serial</b> argument.
  // Return OK on success, NOTOK otherwise.
  //
  int Serial(const String& word, unsigned int& serial);
  //-
  // If the <b>word</b> argument exists in the dictionnary, return its
  // serial number in the <b>serial</b> argument. If it does not exists
  // set the <b>serial</b> argument to WORD_DICT_SERIAL_INVALID.
  // Return OK on success, NOTOK otherwise.
  //
  int SerialExists(const String& word, unsigned int& serial);
  //-
  // Short hand for Serial() followed by Ref().
  // Return OK on success, NOTOK otherwise.
  //
  int SerialRef(const String& word, unsigned int& serial);
  //-
  // Return the frequency of the <b>word</b> argument
  // in the <b>noccurrence</b> argument. 
  // Return OK on success, NOTOK otherwise.
  //
  int Noccurrence(const String& word, unsigned int& noccurrence) const;
#endif /* SWIG */

  //-
  // Short hand for words->GetContext()->GetType()->Normalize(word).
  // Return OK on success, NOTOK otherwise.
  // 
  int Normalize(String& word) const;

  //-
  // Short hand for Incr(word, 1)
  //
  int Ref(const String& word) { return Incr(word, 1); }
  //-
  // Add <b>incr</b> to the frequency of the <b>word</b>. 
  // Return OK on success, NOTOK otherwise.
  //
  int Incr(const String& word, unsigned int incr);
  //-
  // Short hand for Decr(word, 1)
  //
  int Unref(const String& word) { return Decr(word, 1); }
  //-
  // Subtract <b>decr</b> to the frequency of the <b>word</b>. If
  // the frequency becomes lower or equal to zero, remove the entry
  // from the dictionnary and lose the association between the word and its
  // serial number.
  // Return OK on success, NOTOK otherwise.
  //
  int Decr(const String& word, unsigned int decr);
  //-
  // Set the frequency of <b>word</b> with the value of the <b>noccurrence</b>
  // argument.
  //
  int Put(const String& word, unsigned int noccurrence);

  //-
  // Return true if <b>word</b> exists in the dictionnary, false otherwise.
  //
  int Exists(const String& word) const;

#ifndef SWIG
  //-
  // Return a pointer to the associated WordList object.
  //
  List* Words() const;

  //-
  // Return a cursor to sequentially walk the dictionnary using the 
  // <b>Next</b> method. 
  //
  WordDictCursor* Cursor() const;
  //-
  // Return the next entry in the dictionnary. The <b>cursor</b> argument
  // must have been created using the <i>Cursor</i> method. The word is
  // returned in the <b>word</b> argument and the record is returned in
  // the <b>record</b> argument. 
  // On success the function returns 0, at the end of the dictionnary it
  // returns DB_NOTFOUND. The <b>cursor</b> argument is deallocated when
  // the function hits the end of the dictionnary or an error occurs.
  // 
  int Next(WordDictCursor* cursor, String& word, WordDictRecord& record);

  //-
  // Return a cursor to sequentially walk the entries of the dictionnary
  // that start with the <b>prefix</b> argument, using the 
  // <b>NextPrefix</b> method. 
  //
  WordDictCursor* CursorPrefix(const String& prefix) const;
  //-
  // Return the next prefix from the dictionnary. The <b>cursor</b> argument
  // must have been created using the <i>CursorPrefix</i> method. The word is
  // returned in the <b>word</b> argument and the record is returned in
  // the <b>record</b> argument. The <b>word</b> is guaranteed to start with
  // the prefix specified to the <b>CursorPrefix</b> method.
  // On success the function returns 0, at the end of the dictionnary it
  // returns DB_NOTFOUND. The <b>cursor</b> argument is deallocated when
  // the function hits the end of the dictionnary or an error occurs.
  // 
  int NextPrefix(WordDictCursor* cursor, String& word, WordDictRecord& record);

  //-
  // Dump the complete dictionary in the file descriptor <b>f.</b> The
  // format of the dictionary is <i>word serial frequency</i>, one by
  // line. 
  //
  int Write(FILE* f);

 private:
  WordList*      words;
  WordDB*                db;
#endif /* SWIG */
};
#endif /* _WordDict_h_ */
