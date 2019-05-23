//
// StringList.h
//
// StringList: Specialized List containing String objects. 
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: StringList.h,v 1.12 2004/05/28 13:15:21 lha Exp $
//

#ifndef _StringList_h_
#define _StringList_h_

#include "Object.h"
#include "List.h"
#include "htString.h"


class StringList:public List
{
public:
  //
  // Construction/Destruction
  //
  StringList ();

  //
  // Creation of a String from a string or String
  //
  StringList (const char *str, char sep = '\t')
  {
    Create (str, sep);
  }
  StringList (const String & str, char sep = '\t')
  {
    Create (str, sep);
  }
  StringList (const char *str, const char *sep)
  {
    Create (str, sep);
  }
  StringList (const String & str, const char *sep)
  {
    Create (str, sep);
  }

  int Create (const char *str, char sep = '\t');
  int Create (const String & str, char sep = '\t')
  {
    return Create (str.get (), sep);
  }
  int Create (const char *str, const char *sep);
  int Create (const String & str, const char *sep)
  {
    return Create (str.get (), sep);
  }

  //
  // Standard List operations...
  //
  using List::Add;
  virtual void Add (const char *);
  virtual void Add (String * obj)
  {
    List::Add (obj);
  }
  using List::Insert;
  virtual void Insert (const char *, int pos);
  virtual void Insert (String * obj, int pos)
  {
    List::Insert (obj, pos);
  }
  using List::Assign;
  virtual void Assign (const char *, int pos);
  virtual void Assign (String * obj, int pos)
  {
    List::Assign (obj, pos);
  }

  //
  // Since we know we only store strings, we can reliably sort them.
  // If direction is 1, the sort will be in descending order
  //
  void Sort (int direction = 0);

  //
  // Join the Elements of the StringList together
  //
  String Join (char) const;

  //
  // Getting at the parts of the StringList
  //
  char *operator [] (int n);

private:
};

#endif
