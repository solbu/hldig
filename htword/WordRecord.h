//
// WordRecord.h
//
// NAME
// inverted index record.
//
// SYNOPSIS
//
// #include <WordRecord.h>
// 
// WordContext* context;
// WordRecord* record = context->Record();
// if(record->DefaultType() == WORD_RECORD_DATA) {
//   record->info.data = 120;
// } else if(record->DefaultType() == WORD_RECORD_STR) {
//   record->info.str = "foobar";
// }
// delete record;
//
// DESCRIPTION
// 
// The record can contain an integer, if the default record
// type (see CONFIGURATION in <i>WordKeyInfo</i>) is set to <i>DATA</i>
// or a string if set to <i>STR.</i>
// If the type is set to <i>NONE</i> the record does not contain
// any usable information.
//
// Although constructors may be used, the prefered way to create a 
// WordRecord object is by using the <b>WordContext::Record</b> method.
//
// ASCII FORMAT
//
// If default type is <i>DATA</i> it is the decimal representation of
// an integer. If default type is <i>NONE</i> it is the empty string.
//
// END
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordRecord.h,v 1.6.2.10 2000/09/14 03:13:28 ghutchis Exp $
//

#ifndef _WordRecord_h_
#define _WordRecord_h_

#ifndef SWIG
#include "StringList.h"
#include "WordContext.h"
#endif /* SWIG */

//
// The data members of WordRecord. Should really be a union but
// is quite difficult to handle properly for scripting language
// interfaces.
//
class WordRecordStorage {
 public:
  //
  // Only a number. Could be stored in str but using this
  // allows the compression to perform better.
  //
  unsigned int  data;
  //
  // User data
  //
  String	str;
};

//
// Describe the data associated with a key (WordKey)
//
// If type is:
//    WORD_RECORD_DATA	info.data is valid
//    WORD_RECORD_STR	info.str is valid
//    WORD_RECORD_NONE	nothing valid
//
class WordRecord
{
 public:
  //-
  // Constructor. Build an empty record.
  // The <b>ncontext</b> argument must be a pointer to a valid
  // WordContext object.
  //
  inline WordRecord(WordContext* ncontext) {
    context = ncontext;
    Clear();
  }

  //-
  // Reset to empty and set the type to the default specified
  // in the configuration.
  //
  inline void Clear() {
    memset((char*)&info, '\0', sizeof(info));
    type = DefaultType();
  }

  //-
  // Return the default type WORD_RECORD_{DATA,STR,NONE}
  //
  inline int DefaultType() {
    return context->GetRecordInfo().default_type;
  }

#ifndef SWIG
  //-
  // Convert the object to a representation for disk storage written
  // in the <b>packed</b> string.
  // Return OK on success, NOTOK otherwise.
  //
  inline int Pack(String& packed) const {
    packed.trunc();
    switch(type) {

    case WORD_RECORD_DATA:
      {
	packed << (char)type;
	int offset = 1;
	packed.ber_push(offset, info.data);
      }
      break;

    case WORD_RECORD_STR:
      packed << (char)type;
      packed << info.str;
      break;

    case WORD_RECORD_NONE:
      packed.trunc();
      break;

    default:
      fprintf(stderr, "WordRecord::Pack: unknown type %d\n", type);
      return NOTOK;
      break;
    }
    return OK;
  }

  //-
  //
  // Alias for Unpack(String(string, length))
  //
  inline int Unpack(const char* string, int length) {
    return Unpack(String(string, length));
  }

  //-
  // Read the object from a representation for disk storage contained
  // in the <b>packed</b> argument.
  // Return OK on success, NOTOK otherwise.
  //
  inline int Unpack(const String& packed) {
    String decompressed;

    if(packed.length() == 0)
      type = WORD_RECORD_NONE;
    else
      type = packed[0];
    
    switch(type) {

    case WORD_RECORD_DATA:
      {
	int offset = 1;
	packed.ber_shift(offset, info.data);
      }
      break;

    case WORD_RECORD_STR:
      info.str = packed.sub(1);
      break;

    case WORD_RECORD_NONE:
      break;

    default:
      fprintf(stderr, "WordRecord::Pack: unknown type %d\n", (int)type);
      return NOTOK;
      break;
    }

    return OK;
  }
#endif /* SWIG */

#ifndef SWIG
  //-
  // Set the whole structure from ASCII string description stored
  // in the <b>bufferin</b> argument.
  // Return OK on success, NOTOK otherwise.
  //
  int Set(const String& bufferin);
  int SetList(StringList& fields);
  //-
  // Convert the whole structure to an ASCII string description
  // and return it in the <b>bufferout</b> argument.
  // Return OK on success, NOTOK otherwise.
  //
  int Get(String& bufferout) const;
  //-
  // Convert the whole structure to an ASCII string description
  // and return it.
  //
  String Get() const;
  //-
  // Return a pointer to the WordContext object used to create
  // this instance.
  //
  inline WordContext* GetContext() { return context; }
  //-
  // Return a pointer to the WordContext object used to create
  // this instance as a const.
  //
  inline const WordContext* GetContext() const { return context; }

  //-
  // Print object in ASCII form on descriptor <b>f</b> using the
  // Get method.
  //
  int Write(FILE* f) const;
#endif /* SWIG */
  void Print() const;
  
  unsigned char			type;
  WordRecordStorage		info;
#ifndef SWIG
  WordContext*			context;
#endif /* SWIG */
};

#endif /* _WordRecord_h_ */

