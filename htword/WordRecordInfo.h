//
// WordRecord.h
//
// NAME
// information on the record structure of the inverted index.
//
// SYNOPSIS
//
// Only called thru WordContext::Initialize()
//
// DESCRIPTION
// 
// The structure of a record is very limited. It can contain
// a single integer value or a string.
//
// CONFIGURATION
//
// wordlist_wordrecord_description {NONE|DATA|STR} (no default)
//   NONE: the record is empty
//   <br>
//   DATA: the record contains an integer (unsigned int)
//   <br>
//   STR: the record contains a string (String)
//
//
// END
//
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordRecordInfo.h,v 1.1.2.2 2000/09/14 03:13:28 ghutchis Exp $
//

#ifndef _WordRecordInfo_h_
#define _WordRecordInfo_h_

//
// Possible values of the type data field
//
#define WORD_RECORD_INVALID	0
#define WORD_RECORD_DATA	1
#define WORD_RECORD_STR		2
#define WORD_RECORD_NONE	3

#ifndef SWIG
//
// Meta information about WordRecord
//
class WordRecordInfo
{
 public:
  WordRecordInfo(const Configuration& config);

  int default_type;
};
#endif /* SWIG */

#endif /* _WordRecordInfo_h_ */
