//
// WordContext.cc 
//
// WordContext: call Initialize for all classes that need to.
//              This will enable the Instance() static member
//              of each to return a properly allocated and configured 
//              object.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordContext.cc,v 1.1.2.7 2000/09/21 04:25:35 ghutchis Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>

#include "WordContext.h"
#include "WordListOne.h"
#include "WordMonitor.h"

void WordContext::Initialize(const Configuration &config)
{
  Finish();
  configuration = new Configuration(config);
  type = new WordType(*configuration);
  key_info = new WordKeyInfo(*configuration);
  record_info = new WordRecordInfo(*configuration);
  db_info = new WordDBInfo(*configuration);
  if(config.Boolean("wordlist_monitor"))
    monitor = new WordMonitor(*configuration);
}

int WordContext::Initialize(const ConfigDefaults* config_defaults /* = 0 */)
{
  Configuration *config = new Configuration();

  if(config_defaults)
    config->Defaults(config_defaults);

  String filename;
  //
  // Check file pointed by MIFLUZ_CONFIG environment variable
  //
  if(getenv("MIFLUZ_CONFIG")) {
    filename << getenv("MIFLUZ_CONFIG");
    struct stat statbuf;
    if(stat((char*)filename, &statbuf) < 0) {
      if(errno != ENOENT) {
	fprintf(stderr, "WordContext::Initialize: MIFLUZ_CONFIG could not stat %s\n", (char*)filename);
	perror("");
	return NOTOK;
      }
      filename.trunc();
    }
  }
  //
  // Check for ~/.mifluz
  //
  if(filename.empty()) {
    const char* home = getenv("HOME");
    if(home) {
      filename << home << "/.mifluz";
      struct stat statbuf;
      if(stat((char*)filename, &statbuf) < 0) {
	if(errno != ENOENT) {
	  fprintf(stderr, "WordContext::Initialize: could not stat %s\n", (char*)filename);
	  perror("");
	  return NOTOK;
	}
	filename.trunc();
      }
    }
  }

  if(!filename.empty())
    config->Read(filename);

  Initialize(*config);

  delete config;

  return OK;
}

int WordContext::ReInitialize()
{
  if(type) delete type;
  type = 0;
  if(key_info) delete key_info;
  key_info = 0;
  if(record_info) delete record_info;
  record_info = 0;
  if(db_info) delete db_info;
  db_info = 0;
  if(monitor) delete monitor;
  monitor = 0;

  Configuration& config = *configuration;

  type = new WordType(config);
  key_info = new WordKeyInfo(config);
  record_info = new WordRecordInfo(config);
  db_info = new WordDBInfo(config);
  if(config.Boolean("wordlist_monitor")) {
    monitor = new WordMonitor(config);
    GetDBInfo().dbenv->mp_monitor = monitor;
  }

  return OK;
}

void WordContext::Finish()
{
  if(type) delete type;
  type = 0;
  if(key_info) delete key_info;
  key_info = 0;
  if(record_info) delete record_info;
  record_info = 0;
  if(db_info) delete db_info;
  db_info = 0;
  if(monitor) delete monitor;
  monitor = 0;
  if(configuration) delete configuration;
  configuration = 0;
}

WordList* WordContext::List()
{
#if 0
  if(configuration->Boolean("wordlist_multi"))
    return new WordListMulti(this);
  else
#endif
    return new WordListOne(this);
}

WordReference* WordContext::Word()
{
  return new WordReference(this);
}
WordReference* WordContext::Word(const String& key0, const String& record0)
{
  return new WordReference(this, key0, record0);
}
WordReference* WordContext::Word(const String& word)
{
  return new WordReference(this, word);
}

WordRecord* WordContext::Record()
{
  return new WordRecord(this);
}

WordKey* WordContext::Key()
{
  return new WordKey(this);
}
WordKey* WordContext::Key(const String& word)
{
  return new WordKey(this, word);
}

WordKey* WordContext::Key(const WordKey& other)
{
  return new WordKey(other);
}
