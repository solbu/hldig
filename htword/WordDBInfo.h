//
// WordDBInfo.h
//
// NAME
// inverted index usage environment.
//
// SYNOPSIS
//
// Only called thru WordContext::Initialize()
//
// DESCRIPTION
// 
// The inverted indexes may be shared among processes/threads and provide the
// appropriate locking to prevent mistakes. In addition the memory cache
// used by <i>WordList</i> objects may be shared by processes/threads, 
// greatly reducing the memory needs in multi-process applications.
// For more information about the shared environment, check the Berkeley
// DB documentation.
//
// CONFIGURATION
//
// wordlist_env_skip {true,false} (default false)
//   If true no environment is created at all. This must never 
//   be used if a <i>WordList</i> object is created. It may be
//   useful if only <i>WordKey</i> objects are used, for instance.
//
// wordlist_env_share {true,false} (default false)
//   If true a sharable environment is open or created if none exist.
//
// wordlist_env_dir <directory> (default .)
//   Only valid if <i>wordlist_env_share</i> set to <i>true.</i>
//   Specify the directory in which the sharable environment will 
//   be created. All
//   inverted indexes specified with a non-absolute pathname will be
//   created relative to this directory.
//
// 
// END
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
//

#ifndef _WordDBInfo_h_
#define _WordDBInfo_h_

#include "Configuration.h"

struct __db_env;

class WordDBInfo 
{
 public:
    WordDBInfo(const Configuration& config);
    ~WordDBInfo();
    //
    // Unique instance handlers 
    //
    static void Initialize(const Configuration& config);

    static WordDBInfo* Instance() {
      if(instance) return instance;
      fprintf(stderr, "WordDBInfo::Instance: no instance\n");
      return 0;
    }

    //
    // Berkeley DB environment
    //
    struct __db_env *dbenv;

    //
    // Unique instance pointer
    //
    static WordDBInfo* instance;
};

#endif
