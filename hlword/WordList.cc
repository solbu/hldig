//
// WordList.cc
//
// WordList: Interface to the word database. Previously, this wrote to
//           a temporary text file. Now it writes directly to the
//           word database.
//           NOTE: Some code previously attempted to directly read from
//           the word db. This will no longer work, so it's preferred to
//           use the access methods here.
//       Configuration parameter used:
//           wordlist_extend
//           wordlist_verbose 1 walk logic
//           wordlist_verbose 2 walk logic details
//           wordlist_verbose 3 walk logic lots of details
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: WordList.cc,v 1.13 2004/05/28 13:15:27 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "hlconfig.h"
#endif /* HAVE_CONFIG_H */

#include "WordList.h"
#include "WordReference.h"
#include "WordRecord.h"
#include "WordType.h"
#include "WordStat.h"
#include "Configuration.h"
#include "htString.h"
#include "HtPack.h"
#include "HtTime.h"
#include "WordDBCompress.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

// *****************************************************************************
//
WordList::WordList (const Configuration & config_arg):
wtype (config_arg),
config (config_arg)
{
  // The database itself hasn't been opened yet
  isopen = 0;
  isread = 0;
  extended = config.Boolean ("wordlist_extend");
  verbose = config.Value ("wordlist_verbose");
  compressor = 0;
}

// *****************************************************************************
//
WordList::~WordList ()
{
  Close ();
}

// *****************************************************************************
//
int
WordList::Open (const String & filename, int mode, int word_only)
{
  int usecompress = 0;

  // If  word_only,  entries compare equal if the "word" part matches.
  // This should only be used for querying the database, not writing it.
  // It is needed by  speling  to test for the existence of words.
  db.set_bt_compare (word_only ? word_only_db_cmp : word_db_cmp);

  if (config.Value ("wordlist_page_size", 0))
    db.set_pagesize (config.Value ("wordlist_page_size"));

  if (config.Boolean ("wordlist_compress") == 1)
  {
    usecompress = DB_COMPRESS;
    WordDBCompress *compressor =
      new WordDBCompress (config.Boolean ("wordlist_compress_zlib", 0),
                          config.Value ("compression_level", 0));

    //      compressor->debug = config.Value("wordlist_compress_debug");
    SetCompressor (compressor);
    db.CmprInfo (compressor->CmprInfo ());
  }

  int flags = (mode & O_RDWR) ? DB_CREATE : DB_RDONLY;
  if (mode & O_TRUNC)
  {
    if (flags == DB_CREATE)
      flags |= DB_TRUNCATE;
    else
      fprintf (stderr, "WordList::Open: O_TRUNC | O_RDONLY is meaningless\n");
  }
  flags |= usecompress;

  int ret = db.Open (filename, DB_BTREE, flags, 0666) == 0 ? OK : NOTOK;

  isread = mode & O_RDONLY;
  isopen = 1;

  return ret;
}

// *****************************************************************************
//
int
WordList::Close ()
{
  if (isopen)
  {
    if (db.Close () != 0)
      return NOTOK;
    isopen = 0;
    isread = 0;
  }

  {
    WordDBCompress *compressor = GetCompressor ();
    if (compressor)
    {
      delete compressor;
      SetCompressor (0);
    }
  }

  return OK;
}

// ****************************************************************************
//
int
WordList::Put (const WordReference & arg, int flags)
{
  if (arg.Key ().GetWord ().length () == 0)
  {
    fprintf (stderr, "WordList::Put(%s) word is zero length\n",
             (char *) arg.Get ());
    return NOTOK;
  }
  if (!arg.Key ().Filled ())
  {
    fprintf (stderr, "WordList::Put(%s) key is not fully defined\n",
             (char *) arg.Get ());
    return NOTOK;
  }

  WordReference wordRef (arg);
  String word = wordRef.Key ().GetWord ();
  if (wtype.Normalize (word) & WORD_NORMALIZE_NOTOK)
    return NOTOK;
  wordRef.Key ().SetWord (word);

  //
  // The two case could be grouped in a more compact way.
  // However, the resources consumption difference between
  // a Put(DB_NOOVERWRITE) and Put(0) is huge (the first is 75%
  // slower than the second). Check the db_put sources for the
  // explanation.
  //
  int ret = NOTOK;
  if (flags)
  {
    //
    // First attempt tells us if the key exists. If it
    // does not we just increment the reference count.
    // Otherwise, and only if flags does not contain DB_NOOVERWRITE,
    // we override the key/record pair.
    //
    int error;
    if ((error = db.Put (wordRef, DB_NOOVERWRITE)) != 0)
    {
      if (error == DB_KEYEXIST && flags == 0)
        ret = db.Put (wordRef, 0) == 0 ? OK : NOTOK;
    }
    else
    {
      ret = Ref (wordRef);
    }
  }
  else
  {
    if ((ret = db.Put (wordRef, 0)) == 0)
      ret = Ref (wordRef);
  }

  return ret;
}


// *****************************************************************************
//
List *
WordList::operator [] (const WordReference & wordRef)
{
  return Collect (wordRef);
}

// *****************************************************************************
//
List *
WordList::Prefix (const WordReference & prefix)
{
  WordReference prefix2 (prefix);
  prefix2.Key ().UndefinedWordSuffix ();
  return Collect (prefix2);
}

// *****************************************************************************
//
List *
WordList::WordRefs ()
{
  return Collect (WordReference ());
}

// *****************************************************************************
//
List *
WordList::Collect (const WordReference & wordRef)
{
  WordCursor *search = Cursor (wordRef.Key (), HTDIG_WORDLIST_COLLECTOR);
  if (search->Walk () != OK)
    return 0;
  List *result = search->GetResults ();
  delete search;
  return result;
}

// *****************************************************************************
//
// Callback data dedicated to Dump and dump_word communication
//
class DeleteWordData:public Object
{
public:
  DeleteWordData ()
  {
    count = 0;
  }

  int count;
};

// *****************************************************************************
//
//
static int
delete_word (WordList * words, WordDBCursor & cursor,
             const WordReference * word, Object & data)
{
  if (words->Delete (cursor) == 0)
  {
    words->Unref (*word);
    ((DeleteWordData &) data).count++;
    return OK;
  }
  else
  {
    fprintf (stderr, "WordList delete_word: deleting %s failed\n",
             (char *) word->Get ());
    return NOTOK;
  }
}

// *****************************************************************************
//
// Delete all records matching wordRef, return the number of
// deleted records.
//
int
WordList::WalkDelete (const WordReference & wordRef)
{
  DeleteWordData data;
  WordCursor *description = Cursor (wordRef.Key (), delete_word, &data);
  description->Walk ();
  delete description;
  return data.count;
}

// *****************************************************************************
//
//
List *
WordList::Words ()
{
  List *list = 0;
  String key;
  String record;
  WordReference lastWord;
  WordDBCursor cursor;

  if (cursor.Open (db.db) != 0)
    return 0;

  //
  // Move past the first word count record
  //
  const WordReference & last = WordStat::Last ();
  last.Pack (key, record);
  if (cursor.Get (key, record, DB_SET_RANGE) != 0)
    return 0;
  list = new List;
  do
  {
    WordReference wordRef (key, record);
    if (lastWord.Key ().GetWord ().empty () ||
        wordRef.Key ().GetWord () != lastWord.Key ().GetWord ())
    {
      list->Add (new String (wordRef.Key ().GetWord ()));
      lastWord = wordRef;
    }
  }
  while (cursor.Get (key, record, DB_NEXT) == 0);

  return list;
}

// *****************************************************************************
//
// Returns the reference count for word in <count> arg
//
int
WordList::Noccurrence (const WordKey & key, unsigned int &noccurrence) const
{
  noccurrence = 0;
  WordStat stat (key.GetWord ());
  int ret;
  if ((ret = db.Get (stat)) != 0)
  {
    if (ret != DB_NOTFOUND)
      return NOTOK;
  }
  else
  {
    noccurrence = stat.Noccurrence ();
  }

  return OK;
}

// *****************************************************************************
//
// Increment reference count for wordRef
//
int
WordList::Ref (const WordReference & wordRef)
{
  if (!extended)
    return OK;

  WordStat stat (wordRef.Key ().GetWord ());
  int ret;
  if ((ret = db.Get (stat)) != 0 && ret != DB_NOTFOUND)
    return NOTOK;

  stat.Noccurrence ()++;

  return db.Put (stat, 0) == 0 ? OK : NOTOK;
}

// *****************************************************************************
//
// Decrement reference count for wordRef
//
int
WordList::Unref (const WordReference & wordRef)
{
  if (!extended)
    return OK;

  WordStat stat (wordRef.Key ().GetWord ());
  int ret;
  if ((ret = db.Get (stat)) != 0)
  {
    if (ret == DB_NOTFOUND)
      fprintf (stderr,
               "WordList::Unref(%s) Unref on non existing word occurrence\n",
               (char *) wordRef.Get ());
    return NOTOK;
  }

  if (stat.Noccurrence () == 0)
  {
    fprintf (stderr, "WordList::Unref(%s) Unref on 0 occurrences word\n",
             (char *) wordRef.Get ());
    return NOTOK;
  }
  stat.Noccurrence ()--;

  if (stat.Noccurrence () > 0)
  {
    ret = db.Put (stat, 0) == 0 ? OK : NOTOK;
  }
  else
    ret = db.Del (stat) == 0 ? OK : NOTOK;
  return ret;
}


// *****************************************************************************
//
// streaming operators for ascii dumping and reading a list
class FileOutData:public Object
{
public:
  FILE * f;
  FileOutData (FILE * f_arg):f (f_arg)
  {
  }
};

// *****************************************************************************
//
static int
wordlist_walk_callback_file_out (WordList *, WordDBCursor &,
                                 const WordReference * word, Object & data)
{
  fprintf (((FileOutData &) data).f, "%s\n", (char *) word->Get ());
  return OK;
}

// *****************************************************************************
//
int
WordList::Write (FILE * f)
{
  WordKey empty;
  FileOutData data (f);
  WordCursor *description =
    Cursor (empty, wordlist_walk_callback_file_out, (Object *) & data);
  description->Walk ();
  delete description;
  return 0;
}

// *****************************************************************************
//
int
WordList::Read (FILE * f)
{
  WordReference word;
#define WORD_BUFFER_SIZE  1024
  char buffer[WORD_BUFFER_SIZE + 1];
  String line;
  int line_number = 0;
  int inserted = 0;

  while (fgets (buffer, WORD_BUFFER_SIZE, f))
  {
    line_number++;
    int buffer_length = strlen (buffer);
    int eol = buffer[buffer_length - 1] == '\n';

    if (eol)
      buffer[--buffer_length] = '\0';

    line.append (buffer, buffer_length);
    //
    // Join big lines
    //
    if (!eol)
      continue;
    //
    // If line ends with a \ continue
    //
    if (line.last () == '\\')
    {
      line.chop (1);
      continue;
    }

    if (!line.empty ())
    {
      if (word.Set (line) != OK)
      {
        fprintf (stderr, "WordList::Read: line %d : %s\n", line_number,
                 (char *) line);
        fprintf (stderr, " cannot build WordReference (ignored)\n");
      }
      else
      {
        if (Insert (word) != OK)
        {
          fprintf (stderr, "WordList::Read: line %d : %s\n", line_number,
                   (char *) line);
          fprintf (stderr, " insert failed (ignored)\n");
        }
        else
        {
          inserted++;
        }
        if (verbose)
          fprintf (stderr, "WordList::Read: inserting %s\n",
                   (char *) word.Get ());
      }

      line.trunc ();
    }
  }
  return inserted;
}
