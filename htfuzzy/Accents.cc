//
// Accents.cc
//
// Accents: A fuzzy matching algorithm by Robert Marchand, to treat all
//          ISO-8859-1 accented letters as equivalent to their unaccented
//          counterparts.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 2000-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: Accents.cc,v 1.5 2004/05/28 13:15:19 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "Configuration.h"
#include "Accents.h"
#include "Dictionary.h"
#include <ctype.h>

#ifdef HAVE_STD
#include <fstream>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif
#else
#include <fstream.h>
#endif /* HAVE_STD */

extern int debug;

/*-------------------------------------------------------------------.
| Ajoute par Robert Marchand pour permettre le traitement adequat de |
| l'ISO-LATIN         (provient du code de Pierre Rosa)              |
`-------------------------------------------------------------------*/

/*--------------------------------------------------.
| table iso-latin1 "minusculisee" et "de-accentuee" |
`--------------------------------------------------*/
  
static char MinusculeISOLAT1[256] = {
     0,   1,   2,   3,   4,   5,   6,   7,
     8,   9,  10,  11,  12,  13,  14,  15,
    16,  17,  18,  19,  20,  21,  22,  23,
    24,  25,  26,  27,  28,  29,  30,  31,
    32,  33,  34,  35,  36,  37,  38,  39,
    40,  41,  42,  43,  44,  45,  46,  47,
    48,  49,  50,  51,  52,  53,  54,  55,
    56,  57,  58,  59,  60,  61,  62,  63,
    64, 'a', 'b', 'c', 'd', 'e', 'f', 'g',
   'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
   'p', 'q', 'r', 's', 't', 'u', 'v', 'w',
   'x', 'y', 'z',  91,  92,  93,  94,  95,
    96, 'a', 'b', 'c', 'd', 'e', 'f', 'g',
   'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
   'p', 'q', 'r', 's', 't', 'u', 'v', 'w',
   'x', 'y', 'z', 123, 124, 125, 126, 127,
   128, 129, 130, 131, 132, 133, 134, 135,
   136, 137, 138, 139, 140, 141, 142, 143,
   144, 145, 146, 147, 148, 149, 150, 151,
   152, 153, 154, 155, 156, 157, 158, 159,
   160, 161, 162, 163, 164, 165, 166, 167,
   168, 168, 170, 171, 172, 173, 174, 175,
   176, 177, 178, 179, 180, 181, 182, 183,
   184, 185, 186, 187, 188, 189, 190, 191,
   'a', 'a', 'a', 'a', 'a', 'a', 'a', 'c',
   'e', 'e', 'e', 'e', 'i', 'i', 'i', 'i',
   208, 'n', 'o', 'o', 'o', 'o', 'o', 'o',
   'o', 'u', 'u', 'u', 'u', 'y', 222, 223,
   'a', 'a', 'a', 'a', 'a', 'a', 'a', 'c',
   'e', 'e', 'e', 'e', 'i', 'i', 'i', 'i',
   240, 'n', 'o', 'o', 'o', 'o', 'o', 'o',
   'o', 'u', 'u', 'u', 'u', 'y', 254, 255};
  

//*****************************************************************************
// Accents::Accents(const HtConfiguration& config_arg)
//
Accents::Accents(const HtConfiguration& config_arg) :
  Fuzzy(config_arg)
{
    name = "accents";
}


//*****************************************************************************
// Accents::~Accents()
//
Accents::~Accents()
{
}

//*****************************************************************************
// void Accents::generateKey(char *word, String &key)
//
void
Accents::generateKey(char *word, String &key)
{
    static int  maximum_word_length = config.Value("maximum_word_length", 12);

    if (!word || !*word)
      return;

    String      temp(word);
    if (temp.length() > maximum_word_length)
      temp.chop(temp.length()-maximum_word_length);
    word = temp.get();
    key = '0';
    while (*word) {
      key << MinusculeISOLAT1[ (unsigned char) *word++ ];
    }
}


//*****************************************************************************
// void Accents::addWord(char *word)
//
void
Accents::addWord(char *word)
{
    if (!dict)
    {
        dict = new Dictionary;
    }

    String      key;
    generateKey(word, key);

    // Do not add fuzzy key as a word, will be added at search time.
    if (mystrcasecmp(word, key.get()) == 0) 
	return;

    String      *s = (String *) dict->Find(key);
    if (s)
    {
      //        if (mystrcasestr(s->get(), word) != 0)
      (*s) << ' ' << word;
    }
    else
    {
        dict->Add(key, new String(word));
    }
}


//*****************************************************************************
// void Accents::getWords(char *word, List &words)
//
void
Accents::getWords(char *word, List &words)
{

    if (!word || !*word)
	return;

    Fuzzy::getWords(word, words);

    // fuzzy key itself is always searched.
    String	fuzzyKey;
    generateKey(word, fuzzyKey);
    if (mystrcasecmp(fuzzyKey.get(), word) != 0)
	words.Add(new String(fuzzyKey));
}
