//
// Soundex.cc
//
// Soundex: A fuzzy matching algorithm on the principal of the 
//          Soundex method for last names used by the U.S. INS
//          and described by Knuth and others.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Soundex.cc,v 1.7 2000/02/19 05:29:02 ghutchis Exp $
//

#include "Soundex.h"
#include "Dictionary.h"

#include <ctype.h>

//*****************************************************************************
// Soundex::Soundex(const HtConfiguration& config_arg)
//
Soundex::Soundex(const HtConfiguration& config_arg) :
  Fuzzy(config_arg)
{
    name = "soundex";
}


//*****************************************************************************
// Soundex::~Soundex()
//
Soundex::~Soundex()
{
}


//*****************************************************************************
// void Soundex::generateKey(char *word, String &key)
//
void
Soundex::generateKey(char *word, String &key)
{
    int code = 0;
    int lastcode = 0;

    key = 0;
    if (!word)
      {
	key = '0';
	return;
      }

    while (!isalpha(*word))
      word++;

    if (word)
    {
	key << *word++;
    }
    else
    {
      key = '0';
      return;
    }


    while (key.length() < 6)
    {
	switch (*word)
	{
	    case 'b':
	    case 'p':
	    case 'f':
	    case 'v':
		code = 1;
		break;

	    case 'c':
	    case 's':
	    case 'k':
	    case 'g':
	    case 'j':
	    case 'q':
	    case 'x':
	    case 'z':
		code = 2;
		break;

	    case 'd':
	    case 't':
		code = 3;
		break;

	    case 'l':
		code = 4;
		break;

	    case 'm':
	    case 'n':
		code = 5;
		break;

	    case 'r':
		code = 6;
		break;

	    case 'a':
	    case 'e':
	    case 'i':
	    case 'o':
	    case 'u':
	    case 'y':
	    case 'w':
	    case 'h':
	        code = 0;
		break;

	    default:
	        break;
	}
	if (code && code != lastcode)
	  {
	    key << code;
	    lastcode = code;
	  }
	if (*word)
	    word++;
	else
	    break;
    }
}


//*****************************************************************************
// void Soundex::addWord(char *word)
//
void
Soundex::addWord(char *word)
{
    if (!dict)
    {
	dict = new Dictionary;
    }

    String	key;
    generateKey(word, key);

    String	*s = (String *) dict->Find(key);
    if (s)
    {
      //	if (mystrcasestr(s->get(), word) != 0)
      (*s) << ' ' << word;
    }
    else
    {
	dict->Add(key, new String(word));
    }
}
