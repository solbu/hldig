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
// $Id: WordContext.h,v 1.1.2.5 2000/01/14 14:41:05 loic Exp $
//
#ifndef _WordContext_h_
#define _WordContext_h_

#ifndef SWIG
#include "Configuration.h"
#endif /* SWIG */

//
// Short hand for calling Initialize for all classes
// Word* that have a single instance (WordType, WordKeyInfo, WordRecordInfo).
//
class WordContext
{
 public:
  static void               Initialize(const Configuration &config);
#ifndef SWIG
  //
  // Build configuration from MIFLUZ_CONFIG or ~/.mifluz
  // Build a Configuration and feed it with config_defaults, if provided.
  // Call Initialize(config) with built configuration.
  // Return configuration built if either file found or config_defaults
  // provided, 0 otherwise.
  //
  static Configuration     *Initialize(const ConfigDefaults* config_defaults = 0);
#endif /* SWIG */
};

#endif // _WordContext_h_
