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
// $Id: WordReference.h,v 1.3.2.2 1999/12/09 11:31:27 bosc Exp $
//
#ifndef _WordReference_h_
#define _WordReference_h_

#ifndef SWIG
#include "htString.h"
#include "WordRecord.h"
#include "WordKey.h"
#endif /* SWIG */

class WordReference : public Object
{
 public:
  //
  // Construction/Destruction
  //
  WordReference()	{}
#ifndef SWIG
  WordReference(const String& key, const String& record) {
    Clear();
    Unpack(key, record);
  }
  WordReference(const String& word) {
    Clear();
    key.SetWord(word);
  }
#endif /* SWIG */

  ~WordReference()	{}

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
  void			KeyUnpack(const String& packed) { key.Unpack(packed); }
  String		KeyPack() const { String tmp; key.Pack(tmp); return tmp; }
#ifndef SWIG
  int			KeyPack(String& packed) const { return key.Pack(packed); }
#endif /* SWIG */

#ifdef SWIG
%name(SetRecord)
#endif /* SWIG */
  void			Record(const WordRecord& arg) { record = arg; }
  void			RecordUnpack(const String& packed) { record.Unpack(packed); }
  String		RecordPack() const { String tmp; record.Pack(tmp); return tmp; }
#ifndef SWIG
  int			RecordPack(String& packed) const { return record.Pack(packed); }
#endif /* SWIG */

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
  int			Merge(const WordReference& other);
#ifndef SWIG
  static WordReference	Merge(const WordReference& master, const WordReference& slave) {
    WordReference tmp(master);
    tmp.Merge(slave);
    return tmp;
  }
#endif /* SWIG */

  void			Clear() { key.Clear(); record.Clear(); }
#ifndef SWIG
  int			compare(Object *to) { String word(((WordReference *) to)->key.GetWord()); return key.GetWord().nocase_compare(word); }
#endif /* SWIG */

  //
  // Debuging
  //
#ifndef SWIG
  friend inline ostream	&operator << (ostream &o, const WordReference &wordRef) {
    return o << wordRef.key << wordRef.record;
  }
  friend inline istream &operator >> (istream &is,  WordReference &wordRef)
      {
	  return is >> wordRef.key >> wordRef.record;
      }      
#endif /* SWIG */
  void Print() const;

 protected:

#ifndef SWIG
  WordKey		key;
  WordRecord		record;
#endif /* SWIG */
};

#endif


