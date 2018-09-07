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
// at most two integer (int) values. 
//
// CONFIGURATION
//
// wordlist_wordrecord_description {NONE|DATA} (no default)
//   NONE: the record is empty
//   <br>
//   DATA: the record contains two integers (int)
//
//
// END
//
// WordRecord: Record for storing word information in the word database
//             Each word occurrence is stored as a separate key/record pair.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: WordRecordInfo.h,v 1.4 2004/05/28 13:15:28 lha Exp $
//

#ifndef _WordRecordInfo_h_
#define _WordRecordInfo_h_

//
// Possible values of the type data field
//
#define WORD_RECORD_INVALID  0
#define WORD_RECORD_DATA  1
#define WORD_RECORD_STATS  2
#define WORD_RECORD_NONE  3

#ifndef SWIG
//
// Meta information about WordRecord
//
// wordlist_wordrecord_description: DATA 
//   use WordRecordStorage::data for each word occurent
// wordlist_wordrecord_description: NONE 
//  or
// wordlist_wordrecord_description not specified
//   the data associated with each word occurrence is empty
//
class WordRecordInfo
{
public:
  WordRecordInfo (const Configuration & config);
  //
  // Unique instance handlers 
  //
  static void Initialize (const Configuration & config);
  static WordRecordInfo *Instance ()
  {
    if (instance)
      return instance;
    fprintf (stderr, "WordRecordInfo::Instance: no instance\n");
    return 0;
  }

  int default_type;

  //
  // Unique instance pointer
  //
  static WordRecordInfo *instance;
};
#endif /* SWIG */

#endif /* _WordRecordInfo_h_ */
