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
// $Id: WordContext.h,v 1.1.2.4 2000/01/13 14:47:09 loic Exp $
//
#ifndef _WordContext_h_
#define _WordContext_h_

#include "Configuration.h"

//
// Short hand for calling Initialize for all classes
// Word* that have a single instance (WordType, WordKeyInfo, WordRecordInfo).
//
class WordContext
{
 public:
  static void               Initialize(const Configuration &config);
};

#endif // _WordContext_h_
