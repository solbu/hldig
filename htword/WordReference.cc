//
// WordReference.cc
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordReference.cc,v 1.5 2000/02/19 05:29:08 ghutchis Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <iostream.h>

#include "WordReference.h"

int WordReference::Merge(const WordReference& other)
{
  int ret = key.Merge(other.Key());
  record = other.record;

  return ret;
}

//
// Set the structure from an ascii representation
//
int
WordReference::Set(const String& buffer)
{
  StringList fields(buffer, "\t ");
  return Set(fields);
}

//
// Set the structure from list of fields
//
int
WordReference::Set(StringList& fields)
{
  Clear();
  if(key.Set(fields) != OK ||
     record.Set(fields) != OK)
    return NOTOK;
  else
    return OK;
}      

//
// Convert the whole structure to an ascii string description
//
int
WordReference::Get(String& buffer) const
{
  String tmp;
  buffer.trunc();

  if(key.Get(tmp) != OK) return NOTOK;
  buffer.append(tmp);

  if(record.Get(tmp) != OK) return NOTOK;
  buffer.append(tmp);

  return OK;
}

ostream &operator << (ostream &o, const WordReference &wordRef)
{
  String tmp;
  wordRef.Get(tmp);
  o << tmp;
  return o;
}

void WordReference::Print() const
{
  cout << *this;
}
