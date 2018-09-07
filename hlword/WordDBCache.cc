//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
#ifdef HAVE_CONFIG_H
#include "hlconfig.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>

#ifndef _MSC_VER                /* _WIN32 */
#include <unistd.h>
#endif

#include <stdlib.h>
#include <sys/stat.h>

#include "WordKey.h"
#include "WordDB.h"
#include "WordDBCache.h"
#include "WordMeta.h"
#include "ber.h"

int
WordDBCaches::Add (char *key, int key_size, char *data, int data_size)
{
  int ret;
  if ((ret = cache.Allocate (key_size + data_size)) == ENOMEM)
  {
    if ((ret = CacheFlush ()) != 0)
      return ret;
    if ((ret = cache.Allocate (key_size + data_size)))
      return ret;
  }

  return cache.Add (key, key_size, data, data_size);
}

int
WordDBCaches::AddFile (String & filename)
{
  char tmp[32];
  unsigned int serial;
  words->Meta ()->Serial (WORD_META_SERIAL_FILE, serial);
  if (serial == WORD_META_SERIAL_INVALID)
    return NOTOK;
  filename = words->Filename ();
  sprintf (tmp, "C%08d", serial - 1);
  filename << tmp;

  String dummy;
  if (files->Put (0, filename, dummy, 0) != 0)
    return NOTOK;

  size = (cache.GetMax () / 1024) * serial;

  return OK;
}

int
WordDBCaches::CacheFlush ()
{
  if (cache.Empty ())
    return OK;

  if (cache.Sort () != OK)
    return NOTOK;
  String filename;
  int locking = 0;
  if (!lock)
  {
    words->Meta ()->Lock ("cache", lock);
    locking = 1;
  }
  if (AddFile (filename) != OK)
    return NOTOK;
  if (CacheWrite (filename) != OK)
    return NOTOK;

  unsigned int serial;
  words->Meta ()->GetSerial (WORD_META_SERIAL_FILE, serial);
  if (serial >= (unsigned int) file_max || Full ())
    if (Merge () != OK)
      return NOTOK;
  if (locking)
    words->Meta ()->Unlock ("cache", lock);

  return OK;
}

static int
merge_cmp_size (WordDBCaches *, WordDBCacheFile * a, WordDBCacheFile * b)
{
  return b->size - a->size;
}

int
WordDBCaches::Merge ()
{
  if (CacheFlush () != OK)
    return NOTOK;

  int locking = 0;
  if (!lock)
  {
    words->Meta ()->Lock ("cache", lock);
    locking = 1;
  }
  unsigned int serial;
  words->Meta ()->GetSerial (WORD_META_SERIAL_FILE, serial);
  if (serial <= 1)
    return OK;

  //
  // heap lists all the files in decreasing size order (biggest first)
  //
  WordDBCacheFile *heap = new WordDBCacheFile[serial];
  {
    String filename;
    String dummy;
    WordDBCursor *cursor = files->Cursor ();
    struct stat stat_buf;
    int i;
    int ret;
    for (i = 0; (ret = cursor->Get (filename, dummy, DB_NEXT)) == 0; i++)
    {
      WordDBCacheFile & file = heap[i];
      file.filename = filename;
      if (stat ((char *) file.filename, &stat_buf) == 0)
      {
        file.size = stat_buf.st_size;
      }
      else
      {
        const String message =
          String ("WordDBCaches::Merge: cannot stat ") + file.filename;
        perror ((const char *) message);
        return NOTOK;
      }
      cursor->Del ();
    }
    delete cursor;
    myqsort ((void *) heap, serial, sizeof (WordDBCacheFile),
             (myqsort_cmp) merge_cmp_size, (void *) this);
  }

  String tmpname = words->Filename () + String ("C.tmp");

  while (serial > 1)
  {
    WordDBCacheFile *a = &heap[serial - 1];
    WordDBCacheFile *b = &heap[serial - 2];

    if (Merge (a->filename, b->filename, tmpname) != OK)
      return NOTOK;

    //
    // Remove file a
    //
    if (unlink ((char *) a->filename) != 0)
    {
      const String message =
        String ("WordDBCaches::Merge: unlink ") + a->filename;
      perror ((const char *) message);
      return NOTOK;
    }

    //
    // Remove file b
    //
    if (unlink ((char *) b->filename) != 0)
    {
      const String message =
        String ("WordDBCaches::Merge: unlink ") + b->filename;
      perror ((const char *) message);
      return NOTOK;
    }

    //
    // Rename tmp file into file b
    //
    if (rename ((char *) tmpname, (char *) b->filename) != 0)
    {
      const String message =
        String ("WordDBCaches::Merge: rename ") + tmpname + String (" ") +
        b->filename;
      perror ((const char *) message);
      return NOTOK;
    }

    //
    // Update b file size. The size need not be accurate number as long
    // as it reflects the relative size of each file.
    //
    b->size += a->size;

    serial--;
    //
    // update heap
    //
    myqsort ((void *) heap, serial, sizeof (WordDBCacheFile),
             (myqsort_cmp) merge_cmp_size, (void *) this);
  }

  {
    String newname (words->Filename ());
    newname << "C00000000";

    if (rename ((char *) heap[0].filename, (char *) newname) != 0)
    {
      const String message =
        String ("WordDBCaches::Merge: rename ") + heap[0].filename +
        String (" ") + newname;
      perror ((const char *) message);
      return NOTOK;
    }

    String dummy;
    if (files->Put (0, newname, dummy, 0) != 0)
      return NOTOK;
    words->Meta ()->SetSerial (WORD_META_SERIAL_FILE, serial);
  }
  if (locking)
    words->Meta ()->Unlock ("cache", lock);

  return OK;
}

int
WordDBCaches::Merge (const String & filea, const String & fileb,
                     const String & tmpname)
{
  FILE *ftmp = fopen ((const char *) tmpname, "w");
  FILE *fa = fopen ((const char *) filea, "r");
  FILE *fb = fopen ((const char *) fileb, "r");

  unsigned int buffertmp_size = 128;
  unsigned char *buffertmp = (unsigned char *) malloc (buffertmp_size);
  unsigned int buffera_size = 128;
  unsigned char *buffera = (unsigned char *) malloc (buffera_size);
  unsigned int bufferb_size = 128;
  unsigned char *bufferb = (unsigned char *) malloc (bufferb_size);

  unsigned int entriesa_length;
  if (ber_file2value (fa, entriesa_length) < 1)
    return NOTOK;
  unsigned int entriesb_length;
  if (ber_file2value (fb, entriesb_length) < 1)
    return NOTOK;

  if (ber_value2file (ftmp, entriesa_length + entriesb_length) < 1)
    return NOTOK;

  WordDBCacheEntry entrya;
  WordDBCacheEntry entryb;

  if (entriesa_length > 0 && entriesb_length > 0)
  {

    if (ReadEntry (fa, entrya, buffera, buffera_size) != OK)
      return NOTOK;
    if (ReadEntry (fb, entryb, bufferb, bufferb_size) != OK)
      return NOTOK;

    while (entriesa_length > 0 && entriesb_length > 0)
    {
      if (WordKey::
          Compare (words->GetContext (), (const unsigned char *) entrya.key,
                   entrya.key_size, (const unsigned char *) entryb.key,
                   entryb.key_size) < 0)
      {
        if (WriteEntry (ftmp, entrya, buffertmp, buffertmp_size) != OK)
          return NOTOK;
        if (--entriesa_length > 0)
          if (ReadEntry (fa, entrya, buffera, buffera_size) != OK)
            return NOTOK;
      }
      else
      {
        if (WriteEntry (ftmp, entryb, buffertmp, buffertmp_size) != OK)
          return NOTOK;
        if (--entriesb_length > 0)
          if (ReadEntry (fb, entryb, bufferb, bufferb_size) != OK)
            return NOTOK;
      }
    }
  }

  if (entriesa_length > 0 || entriesb_length > 0)
  {
    FILE *fp = entriesa_length > 0 ? fa : fb;
    unsigned int &entries_length =
      entriesa_length > 0 ? entriesa_length : entriesb_length;
    WordDBCacheEntry & entry = entriesa_length > 0 ? entrya : entryb;
    while (entries_length > 0)
    {
      if (WriteEntry (ftmp, entry, buffertmp, buffertmp_size) != OK)
        return NOTOK;
      if (--entries_length > 0)
        if (ReadEntry (fp, entry, buffera, buffera_size) != OK)
          return NOTOK;
    }
  }

  free (buffera);
  free (bufferb);
  free (buffertmp);

  fclose (fa);
  fclose (fb);
  fclose (ftmp);

  return OK;
}

int
WordDBCaches::Merge (WordDB & db)
{
  int locking = 0;
  if (!lock)
  {
    words->Meta ()->Lock ("cache", lock);
    locking = 1;
  }
  if (Merge () != OK)
    return NOTOK;

  String filename;
  String dummy;
  WordDBCursor *cursor = files->Cursor ();
  if (cursor->Get (filename, dummy, DB_FIRST) != 0)
  {
    delete cursor;
    return NOTOK;
  }
  cursor->Del ();
  delete cursor;

  FILE *fp = fopen ((char *) filename, "r");

  unsigned int buffer_size = 128;
  unsigned char *buffer = (unsigned char *) malloc (buffer_size);

  unsigned int entries_length;
  if (ber_file2value (fp, entries_length) < 1)
    return NOTOK;

  WordDBCacheEntry entry;

  unsigned int i;
  for (i = 0; i < entries_length; i++)
  {
    if (ReadEntry (fp, entry, buffer, buffer_size) != OK)
      return NOTOK;
    void *user_data = words->GetContext ();
    WORD_DBT_INIT (rkey, (void *) entry.key, entry.key_size);
    WORD_DBT_INIT (rdata, (void *) entry.data, entry.data_size);
    db.db->put (db.db, 0, &rkey, &rdata, 0);
  }

  if (unlink ((char *) filename) != 0)
  {
    const String message = String ("WordDBCaches::Merge: unlink ") + filename;
    perror ((const char *) message);
    return NOTOK;
  }

  words->Meta ()->SetSerial (WORD_META_SERIAL_FILE, 0);
  if (locking)
    words->Meta ()->Unlock ("cache", lock);
  size = 0;
  free (buffer);
  fclose (fp);

  return OK;
}

int
WordDBCaches::CacheWrite (const String & filename)
{
  FILE *fp = fopen (filename, "w");
  if (!fp)
  {
    String message;
    message << "WordDBCaches::CacheWrite()" << filename << "): ";
    perror ((char *) message);
    return NOTOK;
  }

  int entries_length;
  WordDBCacheEntry *entries;
  int ret;
  if ((ret = cache.Entries (entries, entries_length)) != 0)
    return ret;

  if (ber_value2file (fp, entries_length) < 1)
    return NOTOK;

  unsigned int buffer_size = 1024;
  unsigned char *buffer = (unsigned char *) malloc (buffer_size);
  int i;
  for (i = 0; i < entries_length; i++)
  {
    if (WriteEntry (fp, entries[i], buffer, buffer_size) != OK)
      return NOTOK;
  }
  free (buffer);
  fclose (fp);

  cache.Flush ();

  return OK;
}

int
WordDBCaches::WriteEntry (FILE * fp, WordDBCacheEntry & entry,
                          unsigned char *&buffer, unsigned int &buffer_size)
{
  if (entry.key_size + entry.data_size + 64 > buffer_size)
  {
    buffer_size = entry.key_size + entry.data_size + 64;
    buffer = (unsigned char *) realloc (buffer, buffer_size);
  }

  int p_size = buffer_size;
  unsigned char *p = buffer;

  int ber_len;
  if ((ber_len = ber_value2buf (p, p_size, entry.key_size)) < 1)
  {
    fprintf (stderr, "WordDBCaches::WriteEntry: BER failed for key %d\n",
             entry.key_size);
    return NOTOK;
  }
  p += ber_len;
  memcpy (p, entry.key, entry.key_size);
  p += entry.key_size;

  p_size -= ber_len + entry.key_size;

  if ((ber_len = ber_value2buf (p, p_size, entry.data_size)) < 1)
  {
    fprintf (stderr, "WordDBCaches::WriteEntry: BER failed for data %d\n",
             entry.data_size);
    return NOTOK;
  }
  p += ber_len;
  memcpy (p, entry.data, entry.data_size);
  p += entry.data_size;

  if (fwrite ((void *) buffer, p - buffer, 1, fp) != 1)
  {
    perror ("WordDBCaches::WriteEntry: cannot write entry ");
    return NOTOK;
  }

  return OK;
}

int
WordDBCaches::ReadEntry (FILE * fp, WordDBCacheEntry & entry,
                         unsigned char *&buffer, unsigned int &buffer_size)
{
  if (ber_file2value (fp, entry.key_size) < 1)
    return NOTOK;

  if (entry.key_size > buffer_size)
  {
    buffer_size += entry.key_size;
    if (!(buffer = (unsigned char *) realloc (buffer, buffer_size)))
      return NOTOK;
  }

  if (fread ((void *) buffer, entry.key_size, 1, fp) != 1)
  {
    perror ("WordDBCaches::ReadEntry(): cannot read key entry ");
    return NOTOK;
  }

  if (ber_file2value (fp, entry.data_size) < 1)
    return NOTOK;

  if (entry.data_size > 0)
  {
    if (entry.data_size + entry.key_size > buffer_size)
    {
      buffer_size += entry.data_size;
      if (!(buffer = (unsigned char *) realloc (buffer, buffer_size)))
        return NOTOK;
    }

    if (fread ((void *) (buffer + entry.key_size), entry.data_size, 1, fp) !=
        1)
    {
      perror ("WordDBCaches::ReadEntry(): cannot read data entry ");
      return NOTOK;
    }
  }

  entry.key = (char *) buffer;
  entry.data = (char *) (buffer + entry.key_size);

  return OK;
}
