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
// WordRecord record();
// if(record.DefaultType() == WORD_RECORD_DATA) {
//   record.info.data = ...
// }
//
// DESCRIPTION
// 
// The record can only contain one integer, if the default record
// type (see CONFIGURATION in <i>WordKeyInfo</i>) is set to <i>DATA.</i>
// If the default type is set to <i>NONE</i> the record does not contain
// any usable information.
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
// $Id: WordRecord.h,v 1.6.2.11 2000/10/10 03:15:44 ghutchis Exp $
//

#ifndef _WordRecord_h_
#define _WordRecord_h_

#ifndef SWIG
#include "HtPack.h"
#include "StringList.h"
#include "Configuration.h"
#include "WordRecordInfo.h"
#endif /* SWIG */

/* And this is how we will compress this structure, for disk
   storage.  See HtPack.h  (If there's a portable method by
   which this format string does not have to be specified at
   all, it should be preferred.  For now, at least it is kept
   here, together with the actual struct declaration.)

   Since none of the values are non-zero, we want to use
   unsigned chars and unsigned short ints when possible. */

#ifndef SWIG
#define WORD_RECORD_DATA_FORMAT "u"
#define WORD_RECORD_STATS_FORMAT "u2"
#endif /* SWIG */

//
// Statistical information on a word
//
class WordRecordStat {
 public:
  unsigned int		noccurrence;
  unsigned int		ndoc;
};

//
// The data members of WordRecord. Should really be a union but
// is quite difficult to handle properly for scripting language
// interfaces.
//
class WordRecordStorage {
 public:
  //
  // Arbitrary data
  //
  unsigned int		data;
  //
  // Statistical data used by WordStat
  //
  WordRecordStat	stats;
};

//
// Describe the data associated with a key (WordKey)
//
// If type is:
//    WORD_RECORD_DATA	info.data is valid
//    WORD_RECORD_STATS	info.stats is valid
//    WORD_RECORD_NONE	nothing valid
//
class WordRecord
{
 public:
  WordRecord() { Clear(); }

  void	Clear() { memset((char*)&info, '\0', sizeof(info)); type = DefaultType(); }

#ifndef SWIG
  //
  // Convenience functions to access key structure information (see WordKeyInfo.h)
  //
  static inline const WordRecordInfo* Info()   { return WordRecordInfo::Instance(); }
#endif /* SWIG */
  static inline int                   DefaultType() { return Info()->default_type; }

#ifndef SWIG
  int Pack(String& packed) const {
    switch(type) {

    case WORD_RECORD_DATA:
      packed = htPack(WORD_RECORD_DATA_FORMAT, (char *)&info.data);
      break;

    case WORD_RECORD_STATS:
      packed = htPack(WORD_RECORD_STATS_FORMAT, (char *)&info.stats);
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

  int Unpack(const String& packed) {
    String decompressed;

    switch(type) {

    case WORD_RECORD_DATA:
      decompressed = htUnpack(WORD_RECORD_DATA_FORMAT, packed);
      if(decompressed.length() != sizeof(info.data)) {
	fprintf(stderr, "WordRecord::Unpack: decoding mismatch\n");
	return NOTOK;
      }
      memcpy((char*)&info.data, (char*)decompressed, sizeof(info.data));
      break;

    case WORD_RECORD_STATS:
      decompressed = htUnpack(WORD_RECORD_STATS_FORMAT, packed);
      if(decompressed.length() != sizeof(info.stats)) {
	fprintf(stderr, "WordRecord::Unpack: decoding mismatch\n");
	return NOTOK;
      }
      memcpy((char*)&info.stats, (char*)decompressed, sizeof(info.stats));
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
  //
  // Set the whole structure from ASCII string description
  //
  int Set(const String& bufferin);
  int SetList(StringList& fields);
  //
  // Convert the whole structure to an ASCII string description
  //
  int Get(String& bufferout) const;
  String Get() const;
#endif /* SWIG */

#ifndef SWIG
  //
  // Print object in ASCII form on FILE (uses Get)
  //
  int Write(FILE* f) const;
#endif /* SWIG */
  void Print() const;
  
  unsigned char			type;
  WordRecordStorage		info;
};

#endif /* _WordRecord_h_ */

