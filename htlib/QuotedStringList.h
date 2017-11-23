//
// QuotedStringList.h
//
// QuotedStringList: Fed with a string it will extract separator delimited
//                   words and store them in a list. The words may be 
//                   delimited by " or ', hence the name.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: QuotedStringList.h,v 1.7 2004/05/28 13:15:21 lha Exp $
//

#ifndef _QuotedStringList_h_
#define _QuotedStringList_h_

#include "StringList.h"

class QuotedStringList:public StringList
{
public:
  //
  // Construction/Destruction
  //
  QuotedStringList ();

  //
  // Creation of a String from a string or String
  //
  QuotedStringList (const char *str, char sep = '\t', int single = 0)
  {
    Create (str, sep, single);
  }
  QuotedStringList (const String & str, char sep = '\t', int single = 0)
  {
    Create (str, sep, single);
  }
  QuotedStringList (const char *str, const char *sep, int single = 0)
  {
    Create (str, sep, single);
  }
  QuotedStringList (const String & str, const char *sep, int single = 0)
  {
    Create (str, sep, single);
  }

  int Create (const char *str, char sep = '\t', int single = 0);
  int Create (const String & str, char sep = '\t', int single = 0)
  {
    return Create (str.get (), sep, single);
  }
  int Create (const char *str, const char *sep, int single = 0);
  int Create (const String & str, const char *sep, int single = 0)
  {
    return Create (str.get (), sep, single);
  }
private:
};

#endif
