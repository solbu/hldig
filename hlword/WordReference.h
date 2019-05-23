//
// WordReference.h
//
// NAME
// inverted index occurrence.
//
// SYNOPSIS
// 
// #include <WordReference.h>
//
// WordReference wordRef("word");
// WordReference wordRef();
// WordReference wordRef(WordKey("key <DEF> 1 2"), WordRecord());
//
// WordKey& key = wordRef.Key();
// WordKey& record = wordRef.Record();
// 
// wordRef.Clear();
//
// DESCRIPTION
//
// A <i>WordReference</i> object is an agregate of a <i>WordKey</i> object
// and a <i>WordRecord</i> object.
// 
// ASCII FORMAT
//
// The ASCII description is a string with fields separated by tabs or
// white space. It is made of the ASCII description of a
// <i>WordKey</i> object immediately followed by the ASCII
// description of a <i>WordRecord</i> object.  See the corresponding
// manual pages for more information.
//
// END
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: WordReference.h,v 1.7 2004/05/28 13:15:28 lha Exp $
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
class WordReference:public Object
{
public:
  //
  // Construction/Destruction
  //-
  // Constructor. Build an object with empty key and empty record.
  // 
  WordReference ()
  {
  }
#ifndef SWIG
  //-
  // Constructor. Build an object from disk representation of <b>key</b>
  // and <b>record</b>.
  // 
  WordReference (const String & key0, const String & record0)
  {
    Unpack (key0, record0);
  }
  //-
  // Constructor. Build an object with key word set to <b>word</b>
  // and otherwise empty and empty record.
  // 
  WordReference (const String & word)
  {
    Clear ();
    key.SetWord (word);
  }
#endif /* SWIG */
  ~WordReference ()
  {
  }

  //-
  // Reset to empty key and record
  //
  void Clear ()
  {
    key.Clear ();
    record.Clear ();
  }

  //
  // Accessors
  //-
  // Return the key object.
  //
  WordKey & Key ()
  {
    return key;
  }
#ifndef SWIG
  //-
  // Return the key object as const.
  //
  const WordKey & Key () const
  {
    return key;
  }
#endif                          /* SWIG */
  //-
  // Return the record object.
  //
  WordRecord & Record ()
  {
    return record;
  }
#ifndef SWIG
  //-
  // Return the record object as const.
  //
  const WordRecord & Record () const
  {
    return record;
  }
#endif                          /* SWIG */

  //
  // Conversion
  //
#ifdef SWIG
   %name (SetKey)
#endif                          /* SWIG */
    //-
    // Copy <b>arg</b> in the key part of the object.
    //
  void Key (const WordKey & arg)
  {
    key = arg;
  }
#ifndef SWIG
  //-
  // Set key structure from disk storage format as found in 
  // <b>packed</b> string.
  // Return OK if successfull, NOTOK otherwise.
  //
  int KeyUnpack (const String & packed)
  {
    return key.Unpack (packed);
  }
  //
  //-
  // Convert key object into disk storage format as found in 
  // return the resulting string.
  //
  String KeyPack () const
  {
    String tmp;
      key.Pack (tmp);
      return tmp;
  }
  //-
  // Convert key object into disk storage format as found in 
  // and place the result in <b>packed</b> string.
  // Return OK if successfull, NOTOK otherwise.
  //
  int KeyPack (String & packed) const
  {
    return key.Pack (packed);
  }
#endif                          /* SWIG */

#ifdef SWIG
   %name (SetRecord)
#endif                          /* SWIG */
    //-
    // Copy <b>arg</b> in the record part of the object.
    //
  void Record (const WordRecord & arg)
  {
    record = arg;
  }
#ifndef SWIG
  //-
  // Set record structure from disk storage format as found in 
  // <b>packed</b> string.
  // Return OK if successfull, NOTOK otherwise.
  //
  int RecordUnpack (const String & packed)
  {
    return record.Unpack (packed);
  }
  //-
  // Convert record object into disk storage format as found in 
  // return the resulting string.
  //
  String RecordPack () const
  {
    String tmp;
      record.Pack (tmp);
      return tmp;
  }
  //-
  // Convert record object into disk storage format as found in 
  // and place the result in <b>packed</b> string.
  // Return OK if successfull, NOTOK otherwise.
  //
  int RecordPack (String & packed) const
  {
    return record.Pack (packed);
  }

  //-
  // Short hand for KeyPack(<b>ckey</b>) RecordPack(<b>crecord</b>).
  //
  inline int Pack (String & ckey, String & crecord) const
  {
    if (key.Pack (ckey) == NOTOK)
      return NOTOK;
    if (record.Pack (crecord) == NOTOK)
      return NOTOK;
    return OK;
  }
  //-
  // Short hand for KeyUnpack(<b>ckey</b>) RecordUnpack(<b>crecord</b>).
  //
  int Unpack (const String & ckey, const String & crecord)
  {
    if (key.Unpack (ckey) == NOTOK)
      return NOTOK;
    if (record.Unpack (crecord) == NOTOK)
      return NOTOK;
    return OK;
  }
#endif /* SWIG */

  //
  // Transformations
  //
  //-
  // Merge key with other.Key() using the <i>WordKey::Merge</i> method:
  // key.Merge(other.Key()).
  // See the corresponding manual page for details. Copy other.record
  // into the record part of the object.
  //
  int Merge (const WordReference & other);
#ifndef SWIG
  //-
  // Copy <b>master</b> before merging with <b>master.</b>Merge(<b>slave</b>)
  // and return the copy. Prevents alteration of <b>master</b>.
  //
  static WordReference Merge (const WordReference & master,
                              const WordReference & slave)
  {
    WordReference tmp (master);
    tmp.Merge (slave);
    return tmp;
  }
#endif /* SWIG */

#ifndef SWIG
  int compare (const Object & to) const
  {
    String word (((const WordReference *) &to)->key.GetWord ());
    return key.GetWord ().nocase_compare (word);
  }
#endif /* SWIG */

#ifndef SWIG
  //
  // Set the whole structure from ASCII string description
  //
  //-
  // Set the whole structure from ASCII string in <b>bufferin</b>.
  // See <i>ASCII FORMAT</i> section.
  // Return OK if successfull, NOTOK otherwise.
  //
  int Set (const String & bufferin);
  int SetList (StringList & fields);
  //-
  // Convert the whole structure to an ASCII string description 
  // in <b>bufferout.</b>
  // See <i>ASCII FORMAT</i> section.
  // Return OK if successfull, NOTOK otherwise.
  //
  int Get (String & bufferout) const;
  //-
  // Convert the whole structure to an ASCII string description 
  // and return it.
  // See <i>ASCII FORMAT</i> section.
  // 
  String Get () const;
#endif /* SWIG */

  //
  // Debuging
  //
#ifndef SWIG
  //-
  // Print object in ASCII form on <b>f</b> (uses <i>Get</i> method).
  // See <i>ASCII FORMAT</i> section.
  //
  int Write (FILE * f) const;
#endif /* SWIG */
  //-
  // Print object in ASCII form on <b>stdout</b> (uses <i>Get</i> method).
  // See <i>ASCII FORMAT</i> section.
  //
  void Print () const;

protected:

#ifndef SWIG
  WordKey key;
  WordRecord record;
#endif /* SWIG */
};

#endif /* _WordReference_h */
