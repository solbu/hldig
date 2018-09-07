//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
//
// WordRecord.cc
//
// WordRecord: data portion of the inverted index database
//
#ifdef HAVE_CONFIG_H
#include "hlconfig.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>

#include "WordRecord.h"

//
// WordRecord implementation
//

//
// Convert the whole structure to an ascii string description
//
int
WordRecord::Get (String & buffer) const
{
  buffer.trunc ();

  switch (type)
  {

  case WORD_RECORD_DATA:
    buffer << info.data;
    break;

  case WORD_RECORD_STATS:
    buffer << info.stats.noccurrence << "\t";
    buffer << info.stats.ndoc;
    break;

  case WORD_RECORD_NONE:
    break;

  default:
    fprintf (stderr, "WordRecord::Get: unknown type %d\n", type);
    return NOTOK;
    break;
  }

  return OK;
}

String WordRecord::Get () const
{
  String
    tmp;
  Get (tmp);
  return tmp;
}

//
// Set a record from an ascii representation
//
int
WordRecord::Set (const String & buffer)
{
  StringList fields (buffer, "\t ");
  return SetList (fields);
}

int
WordRecord::SetList (StringList & fields)
{
  int i = 0;

  switch (type)
  {

  case WORD_RECORD_DATA:
    {
      String *field = (String *) fields.Get_First ();

      if (field == 0)
      {
        fprintf (stderr, "WordRecord::Set: failed to retrieve field %d\n", i);
        return NOTOK;
      }
      info.data = (unsigned int) atoi (field->get ());
      fields.Remove (field);
      i++;
    }
    break;

  case WORD_RECORD_STATS:
    {
      String *field = (String *) fields.Get_First ();

      if (field == 0)
      {
        fprintf (stderr, "WordRecord::Set: failed to retrieve field %d\n", i);
        return NOTOK;
      }
      info.stats.noccurrence = (unsigned int) atoi (field->get ());
      fields.Remove (field);
      i++;

      field = (String *) fields.Get_First ();

      if (field == 0)
      {
        fprintf (stderr, "WordRecord::Set: failed to retrieve field %d\n", i);
        return NOTOK;
      }
      info.stats.ndoc = (unsigned int) atoi (field->get ());
      fields.Remove (field);
      i++;
    }
    break;

  case WORD_RECORD_NONE:
    break;

  default:
    fprintf (stderr, "WordRecord::Set: unknown type %d\n", type);
    break;
  }

  return OK;
}

int
WordRecord::Write (FILE * f) const
{
  String tmp;
  Get (tmp);
  fprintf (f, "%s", (char *) tmp);
  return 0;
}

void
WordRecord::Print () const
{
  Write (stderr);
}
