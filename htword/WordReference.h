//
// WordReference.h
//
// WordReference: Reference to a word. Store everything we need for internal use
//                Defined as a class to allow the comparison 
//                method (for sorting).
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordReference.h,v 1.3.2.1 1999/10/25 13:11:21 bosc Exp $
//
#ifndef _WordReference_h_
#define _WordReference_h_

#include "htString.h"
#include "WordRecord.h"
#include "WordKey.h"

class WordReference : public Object
{
 public:
  //
  // Construction/Destruction
  //
  WordReference()	{}
  WordReference(const String& key, const String& record) {
    Clear();
    Unpack(key, record);
  }
  WordReference(const String& word) {
    Clear();
    key.SetWord(word);
  }

  ~WordReference()	{}

  //
  // Accessors
  //
  WordKey&		Key() { return key; }
  const WordKey&	Key() const { return key; }
  WordRecord&		Record() { return record; }
  const WordRecord&	Record() const { return record; }

  //
  // Conversion
  //
  void			Key(const WordKey& arg) { key = arg; }
  void			KeyUnpack(const String& packed) { key.Unpack(packed); }
  String		KeyPack() const { String tmp; key.Pack(tmp); return tmp; }
  int			KeyPack(String& packed) const { return key.Pack(packed); }

  void			Record(const WordRecord& arg) { record = arg; }
  void			RecordUnpack(const String& packed) { record.Unpack(packed); }
  String		RecordPack() const { String tmp; record.Pack(tmp); return tmp; }
  int			RecordPack(String& packed) const { return record.Pack(packed); }

  inline int		Pack(String& ckey, String& crecord) const {
    if(key.Pack(ckey) == NOTOK) return NOTOK;
    if(record.Pack(crecord) == NOTOK) return NOTOK;
    return OK;
  }
  int			Unpack(const String& ckey, const String& crecord) {
    if(key.Unpack(ckey) == NOTOK) return NOTOK;
    if(record.Unpack(crecord) == NOTOK) return NOTOK;
    return OK;
  }

  //
  // Mutations
  //
#define WORD_FILLED	1
#define WORD_PARTIAL	2
  int			Merge(const WordReference& other);
  static WordReference	Merge(const WordReference& master, const WordReference& slave) {
    WordReference tmp(master);
    tmp.Merge(slave);
    return tmp;
  }

  void			Clear() { key.Clear(); record.Clear(); }
  int			compare(Object *to) { String word(((WordReference *) to)->key.GetWord()); return key.GetWord().nocase_compare(word); }

  //
  // Debuging
  //
  friend inline ostream	&operator << (ostream &o, const WordReference &wordRef) {
    return o << wordRef.key << wordRef.record;
  }
  friend inline istream &operator >> (istream &is,  WordReference &wordRef)
      {
	  return is >> wordRef.key >> wordRef.record;
      }      


 protected:

  WordKey		key;
  WordRecord		record;
};

#endif


