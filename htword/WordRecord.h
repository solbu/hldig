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
// $Id: WordRecord.h,v 1.6.2.5 2000/01/03 10:04:48 bosc Exp $
//

#ifndef _WordRecord_h_
#define _WordRecord_h_

#ifndef SWIG
#include "HtPack.h"
#include "StringList.h"
#include "WordContext.h"
#include "Configuration.h"
#endif /* SWIG */

//
// Possible values of the type data field
//
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
#endif /* SWIG */

class WordRecordStat {
 public:
  unsigned int		noccurence;
  unsigned int		ndoc;
};

class WordRecordStorage {
 public:
  unsigned int		data;
  WordRecordStat	stats;
};

class WordRecordInfo
{
 public:
    int default_type;
    static WordRecordInfo *Get(){return WordContext::record_info;}
    static void Initialize(const Configuration& config);
};
class WordRecord
{
 public:
  inline int DefaultType(){return WordRecordInfo::Get()->default_type;}
  WordRecord() { memset((char*)&info, '\0', sizeof(info)); type =  DefaultType(); 
                 //    cout << "record default  type:" << (int)type << endl;
               }
  void	Clear() { memset((char*)&info, '\0', sizeof(info)); type = DefaultType(); }

  int Pack(String& packed) const {
#ifndef SWIG
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
#endif /* SWIG */
  }

  int Unpack(const String& packed) {
#ifndef SWIG
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
#endif /* SWIG */
  }

  //
  // Set the whole structure from ascii string description
  //
  int Set(const String& buffer);
#ifndef SWIG
  int Set(StringList& fields);
#endif /* SWIG */
  //
  // Convert the whole structure to an ascii string description
  //
  int Get(String& buffer) const;

#ifndef SWIG
  friend ostream	&operator << (ostream &o, const WordRecord &record);
#endif /* SWIG */
  void Print() const;
  
  unsigned char			type;

  WordRecordStorage		info;
};

#endif
