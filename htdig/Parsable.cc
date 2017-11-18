//
// Parsable.cc
//
// Parsable: Base class for file parsers (HTML, PDF, ExternalParser ...)
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: Parsable.cc,v 1.9 2004/05/28 13:15:15 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "Parsable.h"
#include "htdig.h"
#include "defaults.h"


//*****************************************************************************
// Parsable::Parsable()
//
Parsable::Parsable()
{
  HtConfiguration* config= HtConfiguration::config();
    contents = 0;
    max_head_length = config->Value("max_head_length", 0);
    max_description_length = config->Value("max_description_length", 50);
    max_meta_description_length = config->Value("max_meta_description_length", 0);

    max_keywords = config->Value("max_keywords", -1);
    if (max_keywords < 0)
  max_keywords = (int) ((unsigned int) ~1 >> 1);
    minimum_word_length = config->Value("minimum_word_length", 3);
}


//*****************************************************************************
// Parsable::~Parsable()
//
Parsable::~Parsable()
{
    delete contents;
}


//*****************************************************************************
// void Parsable::setContents(char *data, int length)
//   This will set the contents of the parsable object.
//
void
Parsable::setContents(char *data, int length)
{
    delete contents;
    contents = new String(data, length);
}

//*****************************************************************************
// void Parsable::addString(char *s, int& wordindex, int slot)
//   Add all words in string s in "heading level" slot, incrementing  wordindex
//   along the way.  String  s  is corrupted.
//
void
Parsable::addString(Retriever& retriever, char *s, int& wordindex, int slot)
{
    char *w = HtWordToken(s);
    while (w)
    {
  if (strlen(w) >= minimum_word_length)
      retriever.got_word(w, wordindex++, slot); // slot for img_alt
  w = HtWordToken(0);
    }
    w = '\0';
}

//*****************************************************************************
// void Parsable::addKeywordString(char *s, int& wordindex)
//   Add all words in string  s  as keywords, incrementing  wordindex
//   along the way.  String  s  is corrupted.
//
void
Parsable::addKeywordString(Retriever& retriever, char *s, int& wordindex)
{
    char  *w = HtWordToken(s);
    while (w)
    {
  if (strlen(w) >= minimum_word_length && ++keywordsCount <= max_keywords)
      retriever.got_word(w, wordindex++, 9);
  w = HtWordToken(0);
    }
    w = '\0';
}
