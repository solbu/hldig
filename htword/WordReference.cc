//
// WordReference.cc
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordReference.cc,v 1.4.2.2 1999/12/14 13:36:06 loic Exp $
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
// Set a key from an ascii representation
//
int
WordReference::Set(const String& buffer)
{
  StringList fields(buffer, "\t ");
  return Set(fields);
}

//
// Set a key from list of fields
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

void WordReference::Print() const
{
  cout << *this;
}
