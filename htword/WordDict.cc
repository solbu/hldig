//
// WordDict.cc
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: WordDict.cc,v 1.4 2004/05/28 13:15:26 lha Exp $
//
#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <fcntl.h>

#include "WordDict.h"
#include "WordListOne.h"

#define WORD_DICT_CURSOR_FIRST  1
#define WORD_DICT_CURSOR_NEXT  2

class WordDictCursor {
public:
  int info;
  String prefix;
  WordDBCursor* cursor;
};

WordDict::~WordDict()
{
  delete db;
}

int WordDict::Initialize(WordList* nwords)
{
  words = nwords;
  db = new WordDB(nwords->GetContext()->GetDBInfo());
  return OK;
}

int WordDict::Open()
{
  const String& filename = words->Filename();
  int flags = words->Flags();

  db->set_pagesize(words->Pagesize());

  return db->Open(filename, "dict", DB_BTREE, flags, 0666, WORD_DB_DICT) == 0 ? OK : NOTOK;
}

int WordDict::Remove()
{
  return db->Remove(words->Filename(), "dict") == 0 ? OK : NOTOK;
}

int WordDict::Close()
{
  return db->Close() == 0 ? OK : NOTOK;
}

int WordDict::Serial(const String& word, unsigned int& serial)
{
  int ret;
  WordDictRecord entry;
  if((ret = entry.Get(db, word)) != 0 && ret != DB_NOTFOUND)
    return NOTOK;
  if(ret == DB_NOTFOUND) {
    words->Meta()->Serial(WORD_META_SERIAL_WORD, entry.id);
    if(entry.Put(db, word) != 0) return NOTOK;
  }
  serial = entry.id;

  return OK;
}

int WordDict::SerialExists(const String& word, unsigned int& serial)
{
  int ret;
  WordDictRecord entry;
  if((ret = entry.Get(db, word)) != 0 && ret != DB_NOTFOUND)
    return NOTOK;

  serial = ret == DB_NOTFOUND ? WORD_DICT_SERIAL_INVALID : entry.id;

  return OK;
}

int WordDict::SerialRef(const String& word, unsigned int& serial)
{
  int ret;
  WordDictRecord entry;
  if((ret = entry.Get(db, word)) != 0 && ret != DB_NOTFOUND)
    return NOTOK;
  if(ret == DB_NOTFOUND)
    words->Meta()->Serial(WORD_META_SERIAL_WORD, entry.id);
  entry.count++;
  if(entry.Put(db, word) != 0) return NOTOK;
  serial = entry.id;

  return OK;
}

int WordDict::Noccurrence(const String& word, unsigned int& noccurrence) const
{
  if(word.empty()) {
    fprintf(stderr, "WordDict::Noccurrence: null word\n");
    return NOTOK;
  }
  WordDictRecord entry;
  noccurrence = 0;
  int ret;
  if((ret = entry.Get(db, word)) != 0) {
    if(ret != DB_NOTFOUND)
      return NOTOK;
  }
  noccurrence = entry.count;

  return OK;
}

int WordDict::Normalize(String& word) const
{
  const WordType& wtype = words->GetContext()->GetType();

  return wtype.Normalize(word);
}

int WordDict::Incr(const String& word, unsigned int incr)
{
  int ret;
  WordDictRecord entry;
  if((ret = entry.Get(db, word)) != 0 && ret != DB_NOTFOUND)
    return NOTOK;
  if(ret == DB_NOTFOUND)
    words->Meta()->Serial(WORD_META_SERIAL_WORD, entry.id);
  entry.count += incr;
  if(entry.Put(db, word) != 0) return NOTOK;
  return OK;
}

int WordDict::Decr(const String& word, unsigned int decr)
{
  WordDictRecord entry;
  int ret;
  if((ret = entry.Get(db, word)) != 0) {
    if(ret == DB_NOTFOUND)
      fprintf(stderr, "WordDict::Unref(%s) Unref on non existing word occurrence\n", (const char*)word);
    return NOTOK;
  }
  entry.count -= decr;
  if(entry.count > 0)
    ret = entry.Put(db, word) == 0 ? OK : NOTOK;
  else
    ret = entry.Del(db, word) == 0 ? OK : NOTOK;

  return ret;
}

int WordDict::Put(const String& word, unsigned int noccurrence)
{
  int ret;
  WordDictRecord entry;
  if((ret = entry.Get(db, word)) != 0 && ret != DB_NOTFOUND)
    return NOTOK;
  if(ret == DB_NOTFOUND)
    words->Meta()->Serial(WORD_META_SERIAL_WORD, entry.id);
  entry.count = noccurrence;
  if(entry.Put(db, word) != 0) return NOTOK;
  return OK;
}

List *WordDict::Words() const
{
  String key;
  String coded;
  WordDBCursor* cursor = db->Cursor();
  List* list = new List;

  while(cursor->Get(key, coded, DB_NEXT) == 0)
    list->Add(new String(key));

  delete cursor;

  return list;
}

int WordDict::Exists(const String& word) const
{
  String tmp_word = word;
  String coded;
  
  return db->Get(0, tmp_word, coded, 0) == 0;
}

WordDictCursor* WordDict::Cursor() const
{
  WordDictCursor* cursor = new WordDictCursor;
  cursor->cursor = db->Cursor();

  return cursor;
}

int WordDict::Next(WordDictCursor* cursor, String& word, WordDictRecord& record)
{
  String coded;
  int ret = cursor->cursor->Get(word, coded, DB_NEXT);
  if(ret != 0) {
    delete cursor->cursor;
    delete cursor;
  } else {
    record.Unpack(coded);
  }
  return ret;
}

WordDictCursor* WordDict::CursorPrefix(const String& prefix) const
{
  WordDictCursor* cursor = new WordDictCursor;
  cursor->cursor = db->Cursor();
  cursor->prefix = prefix;
  cursor->info = WORD_DICT_CURSOR_FIRST;

  return cursor;
}

int WordDict::NextPrefix(WordDictCursor* cursor, String& word, WordDictRecord& record)
{
  String coded;
  int ret;
  if(cursor->info == WORD_DICT_CURSOR_FIRST) {
    word = cursor->prefix;
    ret = cursor->cursor->Get(word, coded, DB_SET_RANGE);
    cursor->info = WORD_DICT_CURSOR_NEXT;
  } else {
    ret = cursor->cursor->Get(word, coded, DB_NEXT);
  }
  //
  // Stop walking when 1) DB_NOTFOUND, 2) the word found is shorter than
  // the required prefix, 3) the word found does not start with the 
  // required prefix.
  //
  if(ret != 0 ||
     cursor->prefix.length() > word.length() ||
     strncmp(cursor->prefix.get(), word.get(), cursor->prefix.length())) {
    delete cursor->cursor;
    delete cursor;
    if(ret == 0) ret = DB_NOTFOUND;
  } else {
    record.Unpack(coded);
  }
  return ret;
}

int WordDict::Write(FILE* f)
{
  WordDBCursor* cursor = db->Cursor();
  String key;
  String coded;
  unsigned int occurrence;
  unsigned int id;

  while(cursor->Get(key, coded, DB_NEXT) == 0) {
    int offset = 0;
    coded.ber_shift(offset, occurrence);
    coded.ber_shift(offset, id);
    fprintf(f, "%s %d %d\n", (char*)key, id, occurrence);
  }

  delete cursor;

  return OK;
}
