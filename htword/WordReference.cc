//
// WordReference.cc
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordReference.cc,v 1.1 1999/09/30 15:56:46 loic Exp $
//

#include <iostream.h>

#include "WordReference.h"
#include "HtPack.h"

//
// Explain the format of data output of the Dump function
//
int WordReference::DumpHeader(FILE *fl)
{
  fprintf(fl, "#word\tdocument id\tflags\tlocation\tanchor\n");
  return OK;
}

//
// Ascii representation of a word occurence.
//
int WordReference::Dump(FILE *fl) const
{
  fprintf(fl, "%s\t%d\t%d\t%d\t%d\n",
	  (const char*)Word(),
	  DocID(),
	  Flags(),
	  Location(),
	  Anchor());
  return OK;
}

//
// Ascii representation of a word occurence.
//
ostream &operator << (ostream &o, const WordReference &wordRef)
{
  o << wordRef.Word() << "\t";
  o << wordRef.DocID() << "\t";
  o << wordRef.Flags() << "\t";
  o << wordRef.Location() << "\t";
  o << wordRef.Anchor();
  return o;
}

int WordReference::Pack(String& ckey, String& crecord) const
{
  // We need to compress the WordRecord and convert it into a binary form
  crecord = htPack(WORD_RECORD_COMPRESSED_FORMAT, (char *)&record );
  return key.Pack(ckey);
}

int WordReference::Unpack(const String& ckey, const String& crecord)
{
  String decompressed;
  decompressed = htUnpack(WORD_RECORD_COMPRESSED_FORMAT,
			  crecord);

  if (decompressed.length() != sizeof (WordRecord))
    {
      cerr << "WordReference::Unpack: Decoding mismatch" << endl;
      return NOTOK;
    }

  memcpy((char *)&record, decompressed.get(), sizeof(WordRecord));

  return key.Unpack(ckey);
}

int WordReference::Merge(const WordReference& other)
{
  int ret = key.Merge(other.Key());

  if(record.anchor == 0) record.anchor = other.Anchor();

  return ret;
}
