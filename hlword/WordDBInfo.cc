// WordDBInfo.cc
//
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
//
#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>

#include "db.h"
#include "WordDBInfo.h"

//
// WordDBInfo implementation
//

WordDBInfo *
  WordDBInfo::instance = 0;

//
// Like standard function but allows easy breakpoint setting.
//
static void
message (const char *errpfx, char *msg)
{
  fprintf (stderr, "%s: %s\n", errpfx, msg);
}

WordDBInfo::WordDBInfo (const Configuration & config)
{
  dbenv = 0;

  if (config.Boolean ("wordlist_env_skip"))
    return;

  int error;
  if ((error = CDB_db_env_create (&dbenv, 0)) != 0)
  {
    fprintf (stderr, "WordDBInfo: CDB_db_env_create %s\n",
             CDB_db_strerror (error));
    return;
  }
  dbenv->set_errpfx (dbenv, "WordDB");
  dbenv->set_errcall (dbenv, message);
  if (dbenv->set_verbose (dbenv, DB_VERB_CHKPOINT, 1) != 0)
    return;
  if (dbenv->set_verbose (dbenv, DB_VERB_DEADLOCK, 1) != 0)
    return;
  if (dbenv->set_verbose (dbenv, DB_VERB_RECOVERY, 1) != 0)
    return;
  if (dbenv->set_verbose (dbenv, DB_VERB_WAITSFOR, 1) != 0)
    return;
  int cache_size = config.Value ("wordlist_cache_size", 10 * 1024 * 1024);
  if (cache_size > 0)
  {
    if (dbenv->set_cachesize (dbenv, 0, cache_size, 1) != 0)
      return;
  }

  char *dir = 0;
  int flags = DB_CREATE;
  if (config.Boolean ("wordlist_env_share"))
  {
    const String & env_dir = config["wordlist_env_dir"];
    if (env_dir.empty ())
    {
      fprintf (stderr, "WordDB: wordlist_env_dir not specified\n");
      return;
    }
    dir = strdup ((const char *) env_dir);

    if (config.Boolean ("wordlist_env_cdb"))
      flags |= DB_INIT_CDB;
    else
      flags |= DB_INIT_LOCK | DB_INIT_MPOOL;

  }
  else
  {
    flags |= DB_PRIVATE | DB_INIT_LOCK | DB_INIT_MPOOL;
  }

  if ((error =
       dbenv->open (dbenv, (const char *) dir, NULL, flags, 0666)) != 0)
    dbenv->err (dbenv, error, "open %s", (dir ? dir : ""));
  if (dir)
    free (dir);
}

WordDBInfo::~WordDBInfo ()
{
  if (dbenv)
    dbenv->close (dbenv, 0);
}

void
WordDBInfo::Initialize (const Configuration & config_arg)
{
  if (instance != 0)
    delete instance;
  instance = new WordDBInfo (config_arg);
}
