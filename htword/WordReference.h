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
// $Id: WordReference.h,v 1.3.2.8 2000/01/20 15:55:22 loic Exp $
//
#ifndef _WordReference_h_
#define _WordReference_h_

#ifndef SWIG
#include "htString.h"
#include "WordRecord.h"
#include "WordKey.h"
#endif /* SWIG */

//
// Describe the WordKey/WordRecord pair
//
class WordReference : public Object
{
 public:
  //
  // Construction/Destruction
  //
  WordReference()	{}
#ifndef SWIG
  WordReference(const String& key0, const String& record0) {
    Unpack(key0, record0);
  }
  WordReference(const String& word) {
    Clear();
    key.SetWord(word);
  }
#endif /* SWIG */
  ~WordReference()	{}

  //
  // Reset to empty key and record
  //
  void			Clear() { key.Clear(); record.Clear(); }

  //
  // Accessors
  //
  WordKey&		Key() { return key; }
#ifndef SWIG
  const WordKey&	Key() const { return key; }
#endif /* SWIG */
  WordRecord&		Record() { return record; }
#ifndef SWIG
  const WordRecord&	Record() const { return record; }
#endif /* SWIG */

  //
  // Conversion
  //
#ifdef SWIG
%name(SetKey)
#endif /* SWIG */
  void			Key(const WordKey& arg) { key = arg; }
#ifndef SWIG
  void			KeyUnpack(const String& packed) { key.Unpack(packed); }
  String		KeyPack() const { String tmp; key.Pack(tmp); return tmp; }
  int			KeyPack(String& packed) const { return key.Pack(packed); }
#endif /* SWIG */

#ifdef SWIG
%name(SetRecord)
#endif /* SWIG */
  void			Record(const WordRecord& arg) { record = arg; }
#ifndef SWIG
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
#endif /* SWIG */

  //
  // Transformations
  //
  int			Merge(const WordReference& other);
#ifndef SWIG
  static WordReference	Merge(const WordReference& master, const WordReference& slave) {
    WordReference tmp(master);
    tmp.Merge(slave);
    return tmp;
  }
#endif /* SWIG */

#ifndef SWIG
  int			compare(Object *to) { String word(((WordReference *) to)->key.GetWord()); return key.GetWord().nocase_compare(word); }
#endif /* SWIG */

#ifndef SWIG
  //
  // Set the whole structure from ascii string description
  //
  int Set(const String& bufferin);
  int Set(StringList& fields);
  //
  // Convert the whole structure to an ascii string description
  //
  int Get(String& bufferout) const;
#endif /* SWIG */

  //
  // Debuging
  //
#ifndef SWIG
  friend ostream	&operator << (ostream &o, const WordReference &wordRef);
#endif /* SWIG */
  void Print() const;

 protected:

#ifndef SWIG
  WordKey		key;
  WordRecord		record;
#endif /* SWIG */
};

#endif /* _WordReference_h */

