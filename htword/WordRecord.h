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
// $Id: WordRecord.h,v 1.3 1999/10/01 15:30:10 loic Exp $
//

#ifndef _WordRecord_h_
#define _WordRecord_h_

#include "HtPack.h"

//
// Possible values of the type data field
//
#define WORD_RECORD_DATA	1
#define WORD_RECORD_STATS	2


/* And this is how we will compress this structure, for disk
   storage.  See HtPack.h  (If there's a portable method by
   which this format string does not have to be specified at
   all, it should be preferred.  For now, at least it is kept
   here, together with the actual struct declaration.)

   Since none of the values are non-zero, we want to use
   unsigned chars and unsigned short ints when possible. */

#define WORD_RECORD_DATA_FORMAT "u"
#define WORD_RECORD_STATS_FORMAT "2u"

class WordRecord
{
 public:
  WordRecord() { info.data = 0; type = WORD_RECORD_DATA; }
  void	Clear() { info.data = 0; type = WORD_RECORD_DATA; }

  int Pack(String& packed) const {
    switch(type) {

    case WORD_RECORD_DATA:
      packed = htPack(WORD_RECORD_DATA_FORMAT, (char *)&info.data);
      break;

    case WORD_RECORD_STATS:
      packed = htPack(WORD_RECORD_STATS_FORMAT, (char *)&info.stats);
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
      memcpy((char*)&info.data, (const char*)decompressed, sizeof(info.data));
      break;

    case WORD_RECORD_STATS:
      decompressed = htUnpack(WORD_RECORD_STATS_FORMAT, packed);
      if(decompressed.length() != sizeof(info.stats)) {
	cerr << "WordRecord::Unpack: decoding mismatch\n";
	return NOTOK;
      }
      memcpy((char*)&info.stats, (const char*)decompressed, sizeof(info.stats));
      break;

    default:
      cerr << "WordRecord::Pack: unknown type " << type << "\n";
      return NOTOK;
      break;
    }

    return OK;
  }

  friend inline ostream	&operator << (ostream &o, const WordRecord &record);
  
  unsigned char			type;

  union {
    unsigned int		data;
    struct {
      unsigned int		noccurence;
      unsigned int		ndoc;
    } stats;
  } info;
};

inline ostream &operator << (ostream &o, const WordRecord &record)
{
  switch(record.type) {

  case WORD_RECORD_DATA:
    o << record.info.data;
    break;

  case WORD_RECORD_STATS:
    o << record.info.stats.noccurence << "\t";
    o << record.info.stats.ndoc;
    break;

  default:
    cerr << "WordRecord::ostream <<: unknown type " << record.type << "\n";
    break;
  }

  return o;
}

#endif
