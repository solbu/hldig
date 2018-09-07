//
// WordContext.h
//
// NAME
// 
// read configuration description and setup mifluz context.
//
// SYNOPSIS
// 
// #include <mifluz.h>
// 
// Configuration* config = WordContext::Initialize();
// ...
// WordContext::Finish();
// 
// DESCRIPTION
// 
// The WordContext::Initialize() method initialize the global context
// for the mifluz library. All other classes depend on it. It must
// therefore be called before any other <i>mifluz</i> classes are used. 
// 
// CONFIGURATION
// 
// wordlist_monitor {true|false} (default false)
//   If true create a <i>WordMonitor</i> instance to gather statistics and 
//   build reports.
//
// 
// ENVIRONMENT
//
// <b>MIFLUZ_CONFIG</b> file name of configuration file read by
// WordContext(3). Defaults to <b>~/.mifluz.</b>
//
// END
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: WordContext.h,v 1.5 2004/05/28 13:15:26 lha Exp $
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
  //-
  // Create environment. Must be called before any other class are used.
  //
  // When calling <b>Initialize</b> a second time, one must ensure
  // that all WordList and WordCursor objects have been
  // destroyed. WordList and WordCursor internal state depends on the
  // current WordContext that will be lost by a second call. 
  // <br>
  // For those interested by the internals, the <b>Initialize</b> function
  // maintains a Berkeley DB environment (DB_ENV) in the following way:
  //
  // First invocation:
  // <pre>
  // Initialize -> new DB_ENV (thru WordDBInfo)
  // </pre>
  //
  // Second invocation:
  // <pre>
  // Initialize -> delete DB_ENV -> new DB_ENV (thru WordDBInfo)
  // </pre>
  //
  static void Initialize (Configuration & config);
#ifndef SWIG
  //-
  // Build a <i>Configuration</i> object from the file pointed to by the 
  // MIFLUZ_CONFIG environment variable or ~/.mifluz.
  // The <b>config_defaults</b> argument, if provided, is passed to
  // the <i>Configuration</i> object using the <b>Defaults</b> method.
  // The <b>Initialize(const Configuration &)</b> method is then called
  // with the <i>Configuration</i> object.
  //
  // Refer to the <i>Configuration</i> description for more information.
  //
  //
  static Configuration *Initialize (const ConfigDefaults * config_defaults =
                                    0);
#endif                          /* SWIG */
  //-
  // Destroy environment. Must be called after all other <i>mifluz</i>
  // objects are destroyed.
  // 
  static void Finish ();
};

#endif // _WordContext_h_
