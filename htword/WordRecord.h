//
// WordRecord.h
//
// WordRecord: Record for storing word information in the word database
//             Each word occurence is stored as a separate key/record pair.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordRecord.h,v 1.7 2000/02/19 05:29:08 ghutchis Exp $
//

#ifndef _WordRecord_h_
#define _WordRecord_h_

#ifndef SWIG
#include "HtPack.h"
#include "StringList.h"
#include "Configuration.h"
#endif /* SWIG */

//
// Possible values of the type data field
//
#define WORD_RECORD_INVALID	0
#define WORD_RECORD_DATA	1
#define WORD_RECORD_STATS	2
#define WORD_RECORD_NONE	3

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

//
// Meta information about WordRecord
//
// wordlist_wordrecord_description: DATA 
//   use WordRecordStorage::data for each word occurent
// wordlist_wordrecord_description: NONE 
//  or
// wordlist_wordrecord_description not specified
//   the data associated with each word occurence is empty
//
class WordRecordInfo
{
 public:
    WordRecordInfo(const Configuration& config);
    //
    // Unique instance handlers 
    //
    static void Initialize(const Configuration& config);
    static WordRecordInfo* Instance() {
      if(instance) return instance;
      fprintf(stderr, "WordRecordInfo::Instance: no instance\n");
      return 0;
    }

    int default_type;

    //
    // Unique instance pointer
    //
    static WordRecordInfo* instance;
};
#endif /* SWIG */

//
// Statistical information on a word
//
class WordRecordStat {
 public:
  unsigned int		noccurence;
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
  // Convinience functions to access key structure information (see WordKeyInfo.h)
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
      cerr << "WordRecord::Pack: unknown type " << type << "\n";
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
	cerr << "WordRecord::Unpack: decoding mismatch\n";
	return NOTOK;
      }
      memcpy((char*)&info.data, (char*)decompressed, sizeof(info.data));
      break;

    case WORD_RECORD_STATS:
      decompressed = htUnpack(WORD_RECORD_STATS_FORMAT, packed);
      if(decompressed.length() != sizeof(info.stats)) {
	cerr << "WordRecord::Unpack: decoding mismatch\n";
	return NOTOK;
      }
      memcpy((char*)&info.stats, (char*)decompressed, sizeof(info.stats));
      break;

    case WORD_RECORD_NONE:
      break;

    default:
      cerr << "WordRecord::Pack: unknown type " << (int)type << "\n";
      return NOTOK;
      break;
    }

    return OK;
  }
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

#ifndef SWIG
  friend ostream	&operator << (ostream &o, const WordRecord &record);
#endif /* SWIG */
  void Print() const;
  
  unsigned char			type;
  WordRecordStorage		info;
};

#endif
