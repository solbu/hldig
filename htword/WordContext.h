//
// WordContext.h
//
// NAME
// 
// read configuration and setup mifluz context.
//
// SYNOPSIS
// 
// #include <mifluz.h>
// 
// WordContext context;
// 
// DESCRIPTION
// 
// The WordContext object must be the first object created.
// All other objects (WordList, WordReference, WordKey and WordRecord)
// are allocated via the corresponding methods of WordContext (List,
// Word, Key and Record respectively). 
//
// The WordContext object contains a <b>Configuration</b> object 
// that holds the configuration parameters used by the instance. 
// If a configuration parameter is changed, the <i>ReInitialize</b> 
// method should be called to take them in account.
//
// CONFIGURATION
// 
// wordlist_monitor {true|false} (default false)
//   If true create a <i>WordMonitor</i> instance to gather statistics and 
//   build reports.
//
// wordlist_multi {true|false} (default false)
//   If true the <b>List</b> method creates a <b>WordListMulti</b> instance,
//   if false it creates a <b>WordListOne</b> instance.
//
// ENVIRONMENT
//
// <b>MIFLUZ_CONFIG</b> file name of configuration file read by
// WordContext(3). Defaults to <b>~/.mifluz.</b>
//
// END
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordContext.h,v 1.1.2.8 2000/09/14 03:13:26 ghutchis Exp $
//
#ifndef _WordContext_h_
#define _WordContext_h_

#ifndef SWIG
#include "Configuration.h"
#include "WordType.h"
#include "WordKeyInfo.h"
#include "WordRecordInfo.h"
#include "WordDBInfo.h"
#include "WordMonitor.h"
#endif /* SWIG */

class WordRecord;
class WordKey;
class WordReference;
class WordList;

//
// Short hand for calling Initialize for all classes
// Word* that have a single instance (WordType, WordKeyInfo, WordRecordInfo).
//
class WordContext
{
 public:
#ifndef SWIG
  //-
  // Constructor. Read the configuration parameters from the
  // environment.  If the environment variable <b>MIFLUZ_CONFIG</b> is
  // set to a pathname, read it as a configuration file. If
  // <b>MIFLUZ_CONFIG</b> is not set, try to read the <i>~/.mifluz</b>
  // configuration file. See the mifluz manual page for a complete
  // list of the configuration attributes.
  //
  WordContext() {
    Clear();
    Initialize();
  }
  //-
  // Constructor. The <b>config</b> argument must contain all the configuration
  // parameters, no configuration file is loaded from the environment.
  //
  WordContext(const Configuration &config) {
    Clear();
    Initialize(config);
  }
#endif /* SWIG */
  //-
  // Constructor. The <b>array</b> argument holds configuration parameters
  // that will override their equivalent in the configuration file read 
  // from the environment.
  //
  WordContext(const ConfigDefaults *array) {
    Clear();
    Initialize(array);
  }
#ifndef SWIG
  ~WordContext() {
    Finish();
  }

  //-
  // Initialize the WordContext object. This method is called by 
  // every constructor.
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
  void               Initialize(const Configuration &config);
  //-
  // Initialize the WordContext object.
  // Build a <i>Configuration</i> object from the file pointed to by the 
  // MIFLUZ_CONFIG environment variable or ~/.mifluz.
  // The <b>config_defaults</b> argument, if provided, is passed to
  // the <i>Configuration</i> object using the <b>Defaults</b> method.
  // The <b>Initialize(const Configuration &)</b> method is then called
  // with the <i>Configuration</i> object.
  // Return OK if success, NOTOK otherwise.
  // Refer to the <i>Configuration</i> description for more information.
  //
  //
  int Initialize(const ConfigDefaults* config_defaults = 0);
#endif /* SWIG */
  //-
  // Destroy internal state except the <i>Configuration</i> object and
  // rebuild it. May be used when the configuration is changed to
  // take these changes in account.
  // Return OK if success, NOTOK otherwise.
  //
  int ReInitialize();

  //
  // Accessors
  //
#ifndef SWIG
  //-
  // Return the <b>WordType</b> data member of the current object as a const. 
  //
  const WordType& GetType() const { return *type; }
#endif /* SWIG */
  //-
  // Return the <b>WordType</b> data member of the current object. 
  //
  WordType& GetType() { return *type; }

#ifndef SWIG
  //-
  // Return the <b>WordKeyInfo</b> data member of the current object
  // as a const.
  //
  const WordKeyInfo& GetKeyInfo() const { return *key_info; }
#endif /* SWIG */
  //-
  // Return the <b>WordKeyInfo</b> data member of the current object. 
  //
  WordKeyInfo& GetKeyInfo() { return *key_info; }

#ifndef SWIG
  //-
  // Return the <b>WordRecordInfo</b> data member of the current
  // object as a const.
  //
  const WordRecordInfo& GetRecordInfo() const { return *record_info; }
#endif /* SWIG */
  //-
  // Return the <b>WordRecordInfo</b> data member of the current object. 
  //
  WordRecordInfo& GetRecordInfo() { return *record_info; }

#ifndef SWIG
  //-
  // Return the <b>WordDBInfo</b> data member of the current object as
  // a const.
  //
  const WordDBInfo& GetDBInfo() const { return *db_info; }
#endif /* SWIG */
  //-
  // Return the <b>WordDBInfo</b> data member of the current object. 
  //
  WordDBInfo& GetDBInfo() { return *db_info; }

#ifndef SWIG
  //-
  // Return the <b>WordMonitor</b> data member of the current object
  // as a const.  The pointer may be NULL if the word_monitor
  // attribute is false.
  //
  const WordMonitor* GetMonitor() const { return monitor; }
#endif /* SWIG */
  //-
  // Return the <b>WordMonitor</b> data member of the current object.
  // The pointer may be NULL if the word_monitor attribute is false.
  //
  WordMonitor* GetMonitor() { return monitor; }

#ifndef SWIG
  //-
  // Return the <b>Configuration</b> data member of the current object
  // as a const.
  //
  const Configuration& GetConfiguration() const { return *configuration; }
#endif /* SWIG */
  //-
  // Return the <b>Configuration</b> data member of the current object. 
  //
  Configuration& GetConfiguration() { return *configuration; }

#ifndef SWIG
  //
  // Builders
  //
  //-
  // Return a new <b>WordList</b> object, using the 
  // WordList(WordContext*) constructor. It is the responsibility of the
  // caller to delete this object before the WordContext object is
  // deleted. Refer to the <b>wordlist_multi</b> configuration parameter
  // to know the exact type of the object created.
  //
  WordList* List();

  //-
  // Return a new <b>WordReference</b> object, using the
  // WordReference(WordContext*) constructor. It is the responsibility of the
  // caller to delete this object before the WordContext object is
  // deleted.
  //
  WordReference* Word();
  //-
  // Return a new <b>WordReference</b> object, using the
  // WordReference(WordContext*, const String&, const& String)
  // constructor. It is the responsibility of the
  // caller to delete this object before the WordContext object is
  // deleted.
  //
  WordReference* Word(const String& key0, const String& record0);
  //-
  // Return a new <b>WordReference</b> object, using the
  // WordReference(WordContext*, const String&)
  // constructor. It is the responsibility of the
  // caller to delete this object before the WordContext object is
  // deleted.
  //
  WordReference* Word(const String& word);

  //-
  // Return a new <b>WordRecord</b> object, using the
  // WordRecord(WordContext*) constructor. It is the responsibility of the
  // caller to delete this object before the WordContext object is
  // deleted.
  //
  WordRecord* Record();

  //-
  // Return a new <b>WordKey</b> object, using the
  // WordKey(WordContext*) constructor. It is the responsibility of the
  // caller to delete this object before the WordContext object is
  // deleted.
  //
  WordKey* Key();
  //-
  // Return a new <b>WordKey</b> object, using the
  // WordKey(WordContext*, const String&) constructor. It is the
  // responsibility of the caller to delete this object before the
  // WordContext object is deleted.
  //
  WordKey* Key(const String& word);
  //-
  // Return a new <b>WordKey</b> object, using the
  // WordKey(WordContext*, const WordKey&) constructor. It is the
  // responsibility of the caller to delete this object before the
  // WordContext object is deleted.
  //
  WordKey* Key(const WordKey& other);
  
 private:
  void Clear() {
    type = 0;
    key_info = 0;
    record_info = 0;
    db_info = 0;
    monitor = 0;
    configuration = 0;
  }
  void               Finish();

  WordType* type;
  WordKeyInfo* key_info;
  WordRecordInfo* record_info;
  WordDBInfo* db_info;
  WordMonitor* monitor;
  Configuration* configuration;
#endif /* SWIG */
};

#endif // _WordContext_h_
