//
// Soundex.h
//
// Soundex: A fuzzy matching algorithm on the principal of the 
//          Soundex method for last names used by the U.S. INS
//          and described by Knuth and others.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: Soundex.h,v 1.7 2004/05/28 13:15:20 lha Exp $
//

#ifndef _Soundex_h_
#define _Soundex_h_

#include "Fuzzy.h"

class Soundex : public Fuzzy
{
public:
  //
  // Construction/Destruction
  //
        Soundex(const HtConfiguration& config_arg);
  virtual      ~Soundex();

  virtual void  generateKey(char *word, String &key);

  virtual void  addWord(char *word);
  
private:
};

#endif


