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
// $Id: WordRecord.h,v 1.6.2.2 1999/12/09 11:31:27 bosc Exp $
//

#ifndef _WordRecord_h_
#define _WordRecord_h_

#ifndef SWIG
#include "HtPack.h"
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

class WordRecordInfo {
 public:
  unsigned int		data;
  WordRecordStat	stats;
};

class WordRecord
{
    int DefaultType()
	{
#ifdef WORD_RECORD_STATS_DEFAULT
	    return(WORD_RECORD_STATS);
#elif WORD_RECORD_NONE_DEFAULT
	    return(WORD_RECORD_NONE);
#else
	    return(WORD_RECORD_DATA);
#endif
	}
 public:
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

#ifndef SWIG
  friend inline ostream	&operator << (ostream &o, const WordRecord &record);
  friend inline istream &operator >> (istream &is, WordRecord &record);
#endif /* SWIG */
  void Print() const;
  
  unsigned char			type;

  WordRecordInfo		info;
};

#ifndef SWIG
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

  case WORD_RECORD_NONE:
    break;

  default:
    cerr << "WordRecord::ostream <<: unknown type " << record.type << "\n";
    break;
  }

  return o;
}

inline istream &operator >> (istream &is, WordRecord &record)
{
    switch(record.type) 
    {
	
    case WORD_RECORD_DATA:
	is >> record.info.data;
    break;

  case WORD_RECORD_STATS:
    cerr << "WordRecord::istream >>: STATS read unsupported!?? "  << "\n";
    break;

  case WORD_RECORD_NONE:
    break;

  default:
    cerr << "WordRecord::ostream <<: unknown type " << record.type << "\n";
    break;
  }

  return is;
}
#endif /* SWIG */

#endif
