//
// WordContext.h
//
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordContext.h,v 1.1.2.3 2000/01/10 16:19:13 loic Exp $
//

#ifndef _WordContext_h_
#define _WordContext_h_

// this is an atempt to group the global parameters that were
// running around htword. 
// it's temporary until we find a better solution

#include "Configuration.h"

class WordType;
class WordKeyInfo;
class WordRecordInfo;

class WordContext
{
 public:
#ifndef SWIG
  friend WordType;
  friend WordKeyInfo;
  friend WordRecordInfo;

  static inline WordType   *Get_word_type_default() { return word_type_default; }
#endif /* SWIG */
  static void               Initialize(const Configuration &config);
#ifndef SWIG
  static int                CheckInitialized() { return WordContext::intialized_ok; }

 private:
  static WordType       *word_type_default;
  static WordKeyInfo    *key_info;
  static WordRecordInfo *record_info;
  static int intialized_ok;
#endif /* SWIG */
};

#endif // _WordContext_h_
