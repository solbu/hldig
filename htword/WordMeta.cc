//
// WordMeta.cc
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordMeta.cc,v 1.2 2002/02/02 18:18:13 ghutchis Exp $
//
#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <fcntl.h>

extern "C" {
#include "db_int.h"
#include "db_page.h"
#include "db_shash.h"
#include "lock.h"
#include "mp.h"
}

#include "WordMeta.h"
#include "WordListOne.h"

#define WORD_META_SERIAL_SIZE	(WORD_META_SERIAL_FILE + 1)

class WordLock {
public:
  WordLock() { lock.off = LOCK_INVALID; }

  DB_LOCK lock;
};

//
// Total size of structure must *NOT* be over 256 bytes.
//
typedef struct _WordMetaInfo {
  DBMETA meta;
  unsigned int serials[WORD_META_SERIAL_SIZE];
} WordMetaInfo;

class WordMetaImp
{
public:
  WordMetaImp() {
    mpf = 0;
    pgno = PGNO_INVALID;
    info = 0;
  }

  DB_MPOOLFILE *mpf;
  db_pgno_t pgno;
  WordMetaInfo *info;
};

WordMeta::~WordMeta()
{
  delete imp;
  delete db;
}

int WordMeta::Initialize(WordList* nwords)
{
  words = nwords;
  db = new WordDB(nwords->GetContext()->GetDBInfo());
  imp = new WordMetaImp();
  return OK;
}

int WordMeta::Open()
{
  const String& filename = words->Filename();
  int flags = words->Flags();

  db->set_pagesize(words->Pagesize());

  if(db->Open(filename, "meta", DB_BTREE, flags, 0666, WORD_DB_DICT) != 0)
   return NOTOK;

  imp->mpf = db->db->mpf;

  int ret;
  String kpgno("pgno");

  if((ret = db->Get(0, kpgno, imp->pgno, 0)) != 0 && ret != DB_NOTFOUND)
    return NOTOK;
  
  /*
   * First time thru, create the meta page and initialize it.
   */
  if(ret == DB_NOTFOUND) {
    if(CDB_memp_fget(imp->mpf, &imp->pgno, DB_MPOOL_NEW, (void**)&imp->info) != 0)
      return NOTOK;
    memset((char*)imp->info, '\0', sizeof(WordMetaInfo));
    imp->info->meta.type = P_INVALID;
    imp->info->meta.pgno = imp->pgno;
    if(CDB_memp_fput(imp->mpf, (void*)imp->info, DB_MPOOL_DIRTY) != 0)
      return NOTOK;

    if(db->Put(0, kpgno, imp->pgno, 0) != 0)
      return NOTOK;
  }

  return OK;
}

int WordMeta::Close()
{
  return db->Close() == 0 ? OK : NOTOK;
}

int WordMeta::Serial(int what, unsigned int& serial)
{
  serial = WORD_META_SERIAL_INVALID;
  if(CDB_memp_fget(imp->mpf, &imp->pgno, 0, (void**)&imp->info) != 0)
    return NOTOK;
  serial = ++imp->info->serials[what];
  if(CDB_memp_fput(imp->mpf, (void*)imp->info, DB_MPOOL_DIRTY) != 0)
    return NOTOK;

  return OK;
}

int WordMeta::GetSerial(int what, unsigned int& serial)
{
  serial = WORD_META_SERIAL_INVALID;
  if(CDB_memp_fget(imp->mpf, &imp->pgno, 0, (void**)&imp->info) != 0)
    return NOTOK;
  serial = imp->info->serials[what];
  if(CDB_memp_fput(imp->mpf, (void*)imp->info, 0) != 0)
    return NOTOK;

  return OK;
}

int WordMeta::SetSerial(int what, unsigned int serial)
{
  if(CDB_memp_fget(imp->mpf, &imp->pgno, 0, (void**)&imp->info) != 0)
    return NOTOK;
  imp->info->serials[what] = serial;
  if(CDB_memp_fput(imp->mpf, (void*)imp->info, DB_MPOOL_DIRTY) != 0)
    return NOTOK;

  return OK;
}

int WordMeta::Lock(const String& resource, WordLock*& lock)
{
  lock = new WordLock;
  DB_ENV* dbenv = words->GetContext()->GetDBInfo().dbenv;
  u_int32_t id;
  if(CDB_lock_id(dbenv, &id) != 0) {
    delete lock;
    lock = 0;
    return NOTOK;
  }
  DBT obj;
  obj.size = resource.length();
  obj.data = (void*)resource.get();
  if(CDB_lock_get(dbenv, id, 0, &obj, DB_LOCK_WRITE, &lock->lock) != 0) {
    delete lock;
    lock = 0;
    return NOTOK;
  }
  return OK;
}

int WordMeta::Unlock(const String& resource, WordLock*& lock)
{
  DB_ENV* dbenv = words->GetContext()->GetDBInfo().dbenv;

  int ret = CDB_lock_put(dbenv, &lock->lock);

  delete lock;
  lock = 0;

  return ret == 0 ? OK : NOTOK;
}
