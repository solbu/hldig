//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
//
// WordRecordInfo.cc
//
// WordRecord: data portion of the inverted index database
//
#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "Configuration.h"
#include "WordRecordInfo.h"

WordRecordInfo* WordRecordInfo::instance = 0;

//
// WordRecordInfo implementation
//
void 
WordRecordInfo::Initialize(const Configuration &config)
{
  if(instance != 0)
    delete instance;
  instance = new WordRecordInfo(config);
}

WordRecordInfo::WordRecordInfo(const Configuration& config)
{
  default_type = WORD_RECORD_INVALID;
  const String &recorddesc = config["wordlist_wordrecord_description"];
  if(!recorddesc.nocase_compare("data")) 
  {
      default_type = WORD_RECORD_DATA;
  } 
  else 
  if(!recorddesc.nocase_compare("none") || recorddesc.empty()) 
  {
      default_type = WORD_RECORD_NONE;	
  } 
  else 
  {
    fprintf(stderr, "WordRecordInfo::WordRecordInfo: invalid wordlist_wordrecord_description: %s\n", (const char*)recorddesc);
  }
}

