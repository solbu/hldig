//
// WordListOne.cc
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordListOne.cc,v 1.1.2.2 2000/09/21 04:25:36 ghutchis Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "WordListOne.h"
#include "WordReference.h"
#include "WordRecord.h"
#include "WordType.h"
#include "WordContext.h"
#include "Configuration.h"
#include "htString.h"
#include "HtTime.h"
#include "WordDBCompress.h"
#include "WordDBCache.h"
#include "WordDead.h"
#include "WordMeta.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

// *****************************************************************************
//
WordListOne::WordListOne(WordContext* ncontext)
{
  context = ncontext;
  db = new WordDB(ncontext->GetDBInfo());
  dict = new WordDict();
  dict->Initialize(this);
  meta = new WordMeta();
  meta->Initialize(this);
  dead = new WordDead();
  dead->Initialize(this);

  // The database itself hasn't been opened yet
  isopen = 0;
  Configuration& config = context->GetConfiguration();
  extended = config.Boolean("wordlist_extend");
  verbose =  config.Value("wordlist_verbose");
  compressor = 0;
  caches = 0;
  flags = 0;
}

// *****************************************************************************
//
WordListOne::~WordListOne()
{
  BatchEnd();
  Close();
  delete dead;
  delete meta;
  delete dict;
  delete db;
}

static int word_db_qcmp(WordContext* context, const WordDBCacheEntry *a, const WordDBCacheEntry *b)
{
  return WordKey::Compare(context, (const unsigned char*)a->key, a->key_size, (const unsigned char*)b->key, b->key_size);
}

// *****************************************************************************
//
int WordListOne::Open(const String& nfilename, int mode)
{
  filename = nfilename;

  int usecompress = 0;
  Configuration& config = context->GetConfiguration();

  if(config.Boolean("wordlist_compress") == 1) {
    usecompress = DB_COMPRESS;
    WordDBCompress* compressor = new WordDBCompress(context);
    //      compressor->debug = config.Value("wordlist_compress_debug");
    SetCompressor(compressor);

    context->GetDBInfo().dbenv->mp_cmpr_info = compressor->CmprInfo();
    context->GetDBInfo().dbenv->flags |= DB_ENV_CMPR;
  }

  flags = (mode & O_RDWR) ? DB_CREATE : DB_RDONLY;
  flags |= usecompress;
  if(mode & O_TRUNC) {
    if(mode & O_RDWR) {
      unlink((char*)filename);
    } else
      fprintf(stderr, "WordListOne::Open: O_TRUNC | O_RDONLY is meaningless\n");
  }

  WordLock* lock;
  Meta()->Lock("open", lock);

  db->set_bt_compare(word_db_cmp, (void*)context);

  if(config.Boolean("wordlist_cache_inserts", 0)) {
    int size = config.Value("wordlist_cache_size", 0);
    if(size / 2 < WORD_DB_CACHE_MINIMUM)
      size = 0;
    else
      size /= 2;

    db->CacheOn(context, size);
    db->CacheCompare(word_db_qcmp);
  }

  db->set_pagesize(Pagesize());

  int ret = db->Open(filename, "index", DB_BTREE, flags, 0666, WORD_DB_INDEX) == 0 ? OK : NOTOK;
  if(ret == NOTOK) return ret;
  if(dict->Open() != OK) return NOTOK;
  if(meta->Open() != OK) return NOTOK;
  if(dead->Open() != OK) return NOTOK;

  isopen = 1;

  Meta()->Unlock("open", lock);

  return ret;
}

// *****************************************************************************
//
int WordListOne::Close()
{
  if(isopen) {
    if(db->Close() != 0) return NOTOK;
    if(dict->Close() != 0) return NOTOK;
    if(meta->Close() != 0) return NOTOK;
    if(dead->Close() != 0) return NOTOK;
    isopen = 0;
  }

  {
    WordDBCompress* compressor = GetCompressor();
    if(compressor) {
      delete compressor;
      SetCompressor(0);
    }
    delete context->GetDBInfo().dbenv->mp_cmpr_info;
    context->GetDBInfo().dbenv->mp_cmpr_info = 0;
    context->GetDBInfo().dbenv->flags &= ~DB_ENV_CMPR;
  }

  return OK;
}

// ****************************************************************************
//
unsigned int WordListOne::Size() const 
{
  return db->Size();
}

// ****************************************************************************
//
int WordListOne::Override(const WordReference& arg)
{
  if (arg.GetWord().length() == 0) {
    fprintf(stderr, "WordListOne::Override(%s) word is zero length\n", (char*)arg.Get());
    return NOTOK;
  }
  if (!arg.Key().Filled()) {
    fprintf(stderr, "WordListOne::Override(%s) key is not fully defined\n", (char*)arg.Get());
    return NOTOK;
  }

  WordType& wtype = context->GetType();
  WordReference	wordRef(arg);
  String 	word = wordRef.GetWord();
  if(wtype.Normalize(word) & WORD_NORMALIZE_NOTOK)
    return NOTOK;
  wordRef.SetWord(word);
  unsigned int wordid = 0;
  if(dict->SerialRef(word, wordid) != OK) return NOTOK;
  wordRef.Key().Set(WORD_KEY_WORD, wordid);

  int ret = NOTOK;

  if(caches) {
    String key;
    String record;
    if(wordRef.Pack(key, record) != OK)
      return NOTOK;
    ret = caches->Add(key.get(), key.length(), record.get(), record.length()) == 0 ? OK : NOTOK;
    if(caches->Full()) caches->Merge(*db);
  } else {
    ret = db->Put(wordRef, 0) == 0 ? OK : NOTOK;
  }

  return ret;
}


// *****************************************************************************
//
List *WordListOne::operator [] (const WordReference& wordRef)
{
  return Collect(wordRef);
}

// *****************************************************************************
//
List *WordListOne::Prefix (const WordReference& prefix)
{
  List* result = new List();
  WordDictCursor* cursor = Dict()->CursorPrefix(prefix.GetWord());
  String word;
  WordDictRecord record;
  WordReference prefix2(prefix);
  while(Dict()->NextPrefix(cursor, word, record) == 0) {
    prefix2.Key().Set(WORD_KEY_WORD, record.Id());
    List* tmp_result = Collect(prefix2);
    while(tmp_result->Count() > 0) {
      WordReference* entry = (WordReference*)tmp_result->Shift(LIST_REMOVE_RELEASE);
      entry->SetWord(word);
      result->Push(entry);
    }
    delete tmp_result;
  }
  return result;
}

// *****************************************************************************
//
List *WordListOne::WordRefs()
{
  return Collect(WordReference(context));
}

// *****************************************************************************
//
List *WordListOne::Collect(const WordReference& wordRef)
{
  WordCursor *search = Cursor(wordRef.Key(), HTDIG_WORDLIST_COLLECTOR);
  if(search->Walk() != OK) return 0;
  List* result = search->GetResults();
  delete search;
  return result;
}

// *****************************************************************************
//
int 
WordListOne::Read(FILE* f)
{
  WordReference wordRef(context);
#define WORD_BUFFER_SIZE	1024
  char buffer[WORD_BUFFER_SIZE + 1];
  String line;
  int line_number = 0;
  int inserted = 0;

  BatchStart();

  String key;
  String record;

  while(fgets(buffer, WORD_BUFFER_SIZE, f)) {
    line_number++;
    int buffer_length = strlen(buffer);
    int eol = buffer[buffer_length - 1] == '\n';

    if(eol) buffer[--buffer_length] = '\0';
    
    line.append(buffer, buffer_length);
    //
    // Join big lines
    //
    if(!eol) continue;
    //
    // If line ends with a \ continue
    //
    if(line.last() == '\\') {
      line.chop(1);
      continue;
    }
      
    if(!line.empty()) {
      StringList fields(line, "\t ");

      //
      // Convert the word to a wordid
      //
      String* word = (String*)fields.Get_First();
      unsigned int wordid;
      if(dict->SerialRef(*word, wordid) != OK) return NOTOK;
      word->trunc();
      (*word) << wordid;

      if(wordRef.SetList(fields) != OK) {
	fprintf(stderr, "WordList::Read: line %d : %s\n", line_number, (char*)line);
	fprintf(stderr, " cannot build WordReference (ignored)\n");
      } else {
	if(wordRef.Pack(key, record) != OK) {
	  fprintf(stderr, "WordList::Read: line %d : %s\n", line_number, (char*)line);
	  fprintf(stderr, " pack failed (ignored)\n");
	} else {
	  caches->Add(key.get(), key.length(), record.get(), record.length());
	  inserted++;
	}
	if(verbose && (inserted % 10000 == 0)) fprintf(stderr, "WordList::Read: inserted %d entries\n", inserted);
	if(verbose > 1) fprintf(stderr, "WordList::Read: inserting %s\n", (char*)wordRef.Get());
      }

      line.trunc();
    }
  }

  BatchEnd();

  return inserted;
}

// *****************************************************************************
//
// streaming operators for ascii dumping and reading a list
class FileOutData : public Object
{
public:
  FILE* f;
  String word;
  FileOutData(FILE* f_arg) : f(f_arg) { }
};

// *****************************************************************************
//
static int
wordlist_walk_callback_file_out(WordList *, WordDBCursor& , const WordReference *wordRef, Object &ndata)
{
  FileOutData& data = (FileOutData&)ndata;
  ((WordReference*)wordRef)->SetWord(data.word);
  fprintf(data.f, "%s\n", (char*)wordRef->Get());
  return OK;
}

int WordListOne::Write(FILE* f)
{
  FileOutData data(f);
  WordDictCursor* cursor = dict->Cursor();
  int ret;
  String word;
  WordDictRecord wordinfo;
  while((ret = dict->Next(cursor, word, wordinfo)) == 0) {
    WordKey key(context);
    key.Set(WORD_KEY_WORD, wordinfo.Id());
    data.word = word;
    WordCursor *search = Cursor(key, wordlist_walk_callback_file_out, (Object *)&data);
    search->Walk();
    delete search;
  }
  return ret == DB_NOTFOUND ? OK : NOTOK;
}


// *****************************************************************************
//
// Callback data dedicated to Dump and dump_word communication
//
class DeleteWordData : public Object
{
public:
  DeleteWordData() { count = 0; }

  int count;
};

// *****************************************************************************
//
//
static int delete_word(WordList *words, WordDBCursor &cursor, const WordReference *word, Object &data)
{
  WordListOne *words_one = (WordListOne*)words;
  if(words_one->DeleteCursor(cursor) == 0) {
    ((DeleteWordData&)data).count++;
    return OK;
  } else {
    fprintf(stderr, "WordList delete_word: deleting %s failed\n", (char*)word->Get());
    return NOTOK;
  }
}

// *****************************************************************************
// 
// Delete all records matching wordRef, return the number of 
// deleted records.
//
int WordListOne::WalkDelete(const WordReference& wordRef)
{
  DeleteWordData data;
  WordKey key = wordRef.Key();

  if(key.IsDefined(WORD_KEY_WORD)) {
    WordCursor *description = Cursor(key, delete_word, &data);
    description->Walk();
    delete description;
    dict->Decr(wordRef.GetWord(), data.count);
  } else {
    WordDictCursor* cursor = dict->Cursor();
    int ret;
    String word;
    WordDictRecord wordinfo;
    int total = 0;
    while((ret = dict->Next(cursor, word, wordinfo)) == 0) {
      key.Set(WORD_KEY_WORD, wordinfo.Id());
      WordCursor *search = Cursor(key, delete_word, &data);
      search->Walk();
      delete search;
      dict->Decr(word, data.count);
      total += data.count;
      data.count = 0;
    }
    data.count = total;
  }
  return data.count;
}

// *****************************************************************************
//
// Returns the reference count for word in <count> arg
//
int WordListOne::Noccurrence(const String& word, unsigned int& noccurrence) const
{
  return dict->Noccurrence(word, noccurrence);
}

WordKey WordListOne::Key(const String& bufferin)
{
  WordKey key(context);
  StringList fields(bufferin, "\t ");
  String* field = (String*)fields.Get_First();
  unsigned int wordid;
  Dict()->Serial(*field, wordid);
  field->trunc();
  (*field) << wordid;
  key.SetList(fields);
  return key;
}

WordReference WordListOne::Word(const String& bufferin, int exists /* = 1 */)
{
  WordReference wordRef(context);
  StringList fields(bufferin, "\t ");
  String* field = (String*)fields.Get_First();
  if(context->GetType().Normalize(*field) & WORD_NORMALIZE_NOTOK) {
    fprintf(stderr, "WordListOne::Word: cannot normalize word\n");
  }
  String word = *field;
  unsigned int wordid;
  if(exists)
    Dict()->SerialExists(word, wordid);
  else
    Dict()->Serial(word, wordid);
  field->trunc();
  (*field) << wordid;
  wordRef.SetList(fields);
  wordRef.SetWord(word);
  return wordRef;
}

void
WordListOne::BatchEnd()
{
  if(caches) {
    caches->Merge(*db);
    WordList::BatchEnd();
  }
}
