//
// Soundex.cc
//
// Implementation of Soundex
//
//
//
#if RELEASE
static char RCSid[] = "$Id: Soundex.cc,v 1.4 1999/02/06 01:19:12 ghutchis Exp $";
#endif

#include "Soundex.h"
#include "Dictionary.h"
#include <ctype.h>

//*****************************************************************************
// Soundex::Soundex()
//
Soundex::Soundex()
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
