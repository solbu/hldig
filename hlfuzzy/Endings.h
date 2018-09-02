//
// Endings.h
//
// Endings: A fuzzy matching algorithm to match the grammatical endings rules
//          used by the ispell dictionary files.
//           
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: Endings.h,v 1.7 2004/05/28 13:15:20 lha Exp $
//

#ifndef _Endings_h_
#define _Endings_h_

#include "Fuzzy.h"

class Dictionary;
class String;
class List;


class Endings:public Fuzzy
{
public:
  //
  // Construction/Destruction
  //
  Endings (const HtConfiguration & config_arg);
    virtual ~ Endings ();

  virtual void getWords (char *word, List & words);
  virtual void generateKey (char *word, String & key);
  virtual void addWord (char *word);
  virtual int openIndex ();
  virtual int writeDB ();

  //
  // Special member which will create the two databases needed for this
  // algorithm.
  //
  int createDB (const HtConfiguration & config);

  static void mungeWord (char *, String &);

private:
    Database * root2word;
  Database *word2root;

  int createRoot (Dictionary &, char *, char *, const String &);
  int readRules (Dictionary &, const String &);
  void expandWord (String &, List &, Dictionary &, char *, char *);
};

#endif
