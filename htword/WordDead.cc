//
// WordDead.cc
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordDead.cc,v 1.2 2002/02/02 18:18:13 ghutchis Exp $
//
#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <fcntl.h>

#include "WordDead.h"
#include "WordListOne.h"

class WordDeadCursor {
public:
  WordDBCursor* cursor;
};

WordDead::~WordDead()
{
  delete db;
  delete mask;
}

int WordDead::Initialize(WordList* nwords)
{
  words = nwords;
  db = new WordDB(nwords->GetContext()->GetDBInfo());
  mask = new WordKey(words->GetContext());
  return OK;
}

int WordDead::Open()
{
  const String& filename = words->Filename();
  int flags = words->Flags();

  db->set_pagesize(words->Pagesize());

  return db->Open(filename, "dead", DB_BTREE, flags, 0666, WORD_DB_DEAD) == 0 ? OK : NOTOK;
}

int WordDead::Remove()
{
  return db->Remove(words->Filename(), "dead") == 0 ? OK : NOTOK;
}

int WordDead::Close()
{
  return db->Close() == 0 ? OK : NOTOK;
}

int WordDead::Normalize(WordKey& key) const
{
  int nfields = words->GetContext()->GetKeyInfo().nfields;
  int i;
  //
  // Undefine in 'key' all fields not defined in 'mask'
  //
  for(i = 0; i < nfields; i++) {
    if(!mask->IsDefined(i))
      key.Set(i, WORD_KEY_VALUE_INVALID);
  }

  return OK;
}

int WordDead::Exists(const WordKey& key) const
{
  WordKey tmp_key = key;

  Normalize(tmp_key);

  String coded;
  String dummy;

  tmp_key.Pack(coded);

  return db->Get(0, coded, dummy, 0) == 0;
}

int WordDead::Put(const WordKey& key) const
{
  WordKey tmp_key = key;

  Normalize(tmp_key);

  String coded;
  String dummy;

  tmp_key.Pack(coded);

  return db->Put(0, coded, dummy, 0) == 0 ? OK : NOTOK;
}

WordDeadCursor* WordDead::Cursor() const
{
  WordDeadCursor* cursor = new WordDeadCursor;
  cursor->cursor = db->Cursor();

  return cursor;
}

int WordDead::Next(WordDeadCursor* cursor, WordKey& key)
{
  String coded;
  String dummy;
  int ret = cursor->cursor->Get(coded, dummy, DB_NEXT);
  if(ret != 0) {
    delete cursor->cursor;
    delete cursor;
  } else {
    key.Unpack(coded);
  }
  return ret;
}
